/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#define AUGD_BUILD
#include "augd/ssl.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#if WITH_SSL

# include "augd/options.hpp"

# include "augaspp/conn.hpp"

# include "augctx/string.h"

# include "augext/log.h"

# include <sstream>

# include <openssl/err.h>
# include <openssl/ssl.h>

using namespace aug;
using namespace augd;
using namespace std;

namespace {

    int
    passwdcb_(char* buf, int size, int rwflag, void* arg)
    {
        char* frobpass(static_cast<char*>(arg));
        aug_memfrob(frobpass, AUG_MAXPASSWORD);
        aug_strlcpy(buf, frobpass, size);
        aug_memfrob(frobpass, AUG_MAXPASSWORD);
        return static_cast<int>(strlen(buf));
    }

    void
    infocb_(const SSL* ssl, int where, int ret)
    {
        if (aug_loglevel() < AUG_LOGDEBUG0 + 2)
            return;

        const char* str;
        int op(where & ~SSL_ST_MASK);

        if (op & SSL_ST_CONNECT)
            str = "connect";
        else if (op & SSL_ST_ACCEPT)
            str = "accept";
        else
            str = "undefined";

        if (where & SSL_CB_LOOP) {

            // Callback has been called to indicate state change inside a
            // loop.

            aug_ctxdebug2(aug_tlx, "SSL: %s loop: %s", str,
                          SSL_state_string_long(ssl));

        } else if (where & SSL_CB_ALERT) {

            // Callback has been called due to an alert being sent or
            // received.

            str = (where & SSL_CB_READ) ? "read" : "write";
            aug_ctxdebug2(aug_tlx, "SSL: %s %s: %s", str,
                          SSL_alert_type_string_long(ret),
                          SSL_alert_desc_string_long(ret));

        } else if (where & SSL_CB_EXIT) {

            // Callback has been called to indicate error exit of a handshake
            // function. (May be soft error with retry option for non-blocking
            // setups.)

            if (0 == ret) {
                aug_ctxdebug2(aug_tlx, "SSL: %s failed: %s", str,
                              SSL_state_string_long(ssl));
            } else if (ret < 0) {
                aug_ctxdebug2(aug_tlx, "SSL: %s error: %s", str,
                              SSL_state_string_long(ssl));
            }
        }
    }

    void
    lognid_(const char* s, X509_name_st* name, int nid)
    {
        char buf[256];
        buf[0] = '\0';

        // Log specified component of name.

        X509_NAME_get_text_by_NID(name, nid, buf, sizeof(buf));
        if (buf[0])
            aug_ctxinfo(aug_tlx, "%s=[%s]", s, buf);
    }

    int
    verifycb_(int preverify_ok, X509_STORE_CTX* x509_ctx)
    {
        X509* peer(X509_STORE_CTX_get_current_cert(x509_ctx));
        X509_name_st* name(X509_get_subject_name(peer));

        char subject[256];
        X509_NAME_oneline(name, subject, sizeof(subject));
        aug_ctxinfo(aug_tlx, "subject: %s", subject);

        char issuer[256];
        X509_NAME_oneline(X509_get_issuer_name(peer), issuer, sizeof(issuer));
        aug_ctxinfo(aug_tlx, "issuer: %s", issuer);

        lognid_("country", name, NID_countryName);
        lognid_("state or province", name, NID_stateOrProvinceName);
        lognid_("locality", name, NID_localityName);
        lognid_("organisation", name, NID_organizationName);
        lognid_("organisational unit", name, NID_organizationalUnitName);
        lognid_("common name", name, NID_commonName);
        lognid_("email address", name, NID_pkcs9_emailAddress);

        if (!preverify_ok) {
            aug_ctxwarn(aug_tlx, "verification failed: %s",
                        X509_verify_cert_error_string(x509_ctx->error));
            return 0;
        }

        // Allow the associated module an opportunity to reject the
        // certificate: although the certificate has been verified by the CA,
        // the module should determine whether it is fit for purpose.

        SSL* ssl(static_cast<SSL*>
                 (X509_STORE_CTX_get_ex_data
                  (x509_ctx, SSL_get_ex_data_X509_STORE_CTX_idx())));
        aug_ssldata* data(static_cast<aug_ssldata*>(SSL_get_app_data(ssl)));
        assert(data);

        if (!authchan(data->handler_, data->id_, subject, issuer)) {
            aug_ctxwarn(aug_tlx, "verification failed: rejected by session");
            return 0;
        }

        return 1;
    }

    int
    tomode_(int level)
    {
        int mode;
        switch (level) {
        case 0:
            mode = SSL_VERIFY_NONE;
            break;
        case 1:
            mode = SSL_VERIFY_PEER;
            break;
        default:
            mode = SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
            break;
        }
        return mode;
    }
}

sslctxptr
augd::createsslctx(const string& name, const options& options,
                   char* frobpass)
{
    string s("ssl.context.");
    s += name;

    // Read ssl options from configuration.

    const char* certfile(options.get(s + ".certfile", 0));
    const char* keyfile(options.get(s + ".keyfile", 0));
    const char* cadir(options.get(s + ".cadir", 0));
    const char* cafile(options.get(s + ".cafile", 0));
    const char* crlfile(options.get(s + ".crlfile", 0));
    const char* ciphers(options.get(s + ".ciphers", 0));
    int depth(atoi(options.get(s + ".depth", "1")));
    int verify(atoi(options.get(s + ".verify", "1")));

    sslctxptr ptr(new (tlx) sslctx());
    SSL_CTX* ctx(ptr->get());

    // Load keys and certificates.

    if (certfile) {

        if (!SSL_CTX_use_certificate_file(ctx, certfile, SSL_FILETYPE_ASN1)) {

            ERR_clear_error();

            if (!SSL_CTX_use_certificate_chain_file(ctx, certfile))
                throw ssl_error(__FILE__, __LINE__, ERR_get_error());
        }

        if (!keyfile)
            keyfile = certfile;
    }

    SSL_CTX_set_default_passwd_cb(ctx, passwdcb_);
    SSL_CTX_set_default_passwd_cb_userdata(ctx, frobpass);

    if (keyfile) {

        if (!SSL_CTX_use_PrivateKey_file(ctx, keyfile, SSL_FILETYPE_ASN1)) {

            ERR_clear_error();

            if (!SSL_CTX_use_PrivateKey_file(ctx, keyfile, SSL_FILETYPE_PEM))
                throw ssl_error(__FILE__, __LINE__, ERR_get_error());
        }
    }

    // Load trusted CAs.

    if ((cafile || cadir)
        && !SSL_CTX_load_verify_locations(ctx, cafile, cadir))
        throw ssl_error(__FILE__, __LINE__, ERR_get_error());

    if (ciphers && !SSL_CTX_set_cipher_list(ctx, ciphers))
        throw ssl_error(__FILE__, __LINE__, ERR_get_error());

    SSL_CTX_set_info_callback(ctx, infocb_);
    SSL_CTX_set_verify(ctx, tomode_(verify), verifycb_);

    SSL_CTX_set_mode(ctx, SSL_MODE_ENABLE_PARTIAL_WRITE);
    SSL_CTX_set_verify_depth(ctx, verify);

    if (crlfile) {

        X509_STORE* store(SSL_CTX_get_cert_store(ctx));
        X509_LOOKUP* lookup;

        if (!(lookup = X509_STORE_add_lookup(store, X509_LOOKUP_file())))
            throw ssl_error(__FILE__, __LINE__, ERR_get_error());

        if (!X509_load_crl_file(lookup, crlfile, X509_FILETYPE_PEM))
            throw ssl_error(__FILE__, __LINE__, ERR_get_error());

        X509_STORE_set_flags(store, X509_V_FLAG_CRL_CHECK
                             | X509_V_FLAG_CRL_CHECK_ALL);
    }

    return ptr;
}

void
augd::createsslctxs(sslctxs& sslctxs, const options& options, char* frobpass)
{
    const char* contexts = options.get("ssl.contexts", 0);

    if (contexts) {

        istringstream is(contexts);
        string name;
        while (is >> name)
            sslctxs.insert(make_pair(name, createsslctx(name, options,
                                                        frobpass)));
    }
}

#endif // WITH_SSL
