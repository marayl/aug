/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/inet.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augsys/socket.h"
#include "augsys/unistd.h" /* aug_close() */

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/errno.h"
#include "augctx/string.h" /* aug_strlcpy() */

#if !defined(_WIN32)
# if HAVE_ALLOCA_H
#  include <alloca.h>
# endif /* HAVE_ALLOCA_H */
# include <netinet/tcp.h>
#else /* _WIN32 */
# include <malloc.h>
#endif /* _WIN32 */

#include <string.h>        /* strchr() */

AUGNET_API aug_sd
aug_tcpconnect(const char* host, const char* serv, struct aug_endpoint* ep)
{
    aug_sd sd;
    struct addrinfo hints, * res, * save;

    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (-1 == aug_getaddrinfo(host, serv, &hints, &res))
        return AUG_BADSD;

    save = res;

    do {
        sd = aug_socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (AUG_BADSD == sd)
            continue; /* Ignore this one. */

        aug_getendpoint(res, ep);

        if (0 == aug_connect(sd, ep))
            break; /* Success. */

        if (-1 == aug_sclose(sd)) /* Ignore this one. */
            goto fail;

    } while ((res = res->ai_next));

    if (!res) /* errno set from final aug_connect(). */
        goto fail;

    aug_destroyaddrinfo(save);
    return sd;

 fail:
    aug_destroyaddrinfo(save);
    return AUG_BADSD;
}

AUGNET_API aug_sd
aug_tcplisten(const char* host, const char* serv, struct aug_endpoint* ep)
{
    aug_sd sd;
    struct addrinfo hints, * res, * save;

    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (-1 == aug_getaddrinfo(host, serv, &hints, &res))
        return AUG_BADSD;

    save = res;

    do {
        sd = aug_socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (AUG_BADSD == sd)
            continue; /* Error, try next one. */

        if (-1 == aug_setreuseaddr(sd, 1))
            goto fail2;

        aug_getendpoint(res, ep);

        if (0 == aug_bind(sd, ep))
            break; /* Success. */

        if (-1 == aug_sclose(sd)) /* Bind error, close and try next one. */
            goto fail1;

    } while ((res = res->ai_next));

    if (!res) /* errno from final aug_socket() or aug_bind(). */
        goto fail1;

    if (-1 == aug_listen(sd, SOMAXCONN))
        goto fail2;

    aug_destroyaddrinfo(save);
    return sd;

 fail2:
    aug_sclose(sd);

 fail1:
    aug_destroyaddrinfo(save);
    return AUG_BADSD;
}

AUGNET_API aug_sd
aug_udpclient(const char* host, const char* serv, struct aug_endpoint* ep)
{
    aug_sd sd;
    struct addrinfo hints, * res, * save;

    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if (-1 == aug_getaddrinfo(host, serv, &hints, &res))
        return AUG_BADSD;

    save = res;

    do {
        sd = aug_socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (AUG_BADSD != sd)
            break; /* Success. */

    } while ((res = res->ai_next));

    if (!res) /* errno set from final aug_socket(). */
        goto fail;

    aug_getendpoint(res, ep);

    aug_destroyaddrinfo(save);
    return sd;

 fail:
    aug_destroyaddrinfo(save);
    return AUG_BADSD;
}

AUGNET_API aug_sd
aug_udpconnect(const char* host, const char* serv, struct aug_endpoint* ep)
{
    aug_sd sd;
    struct addrinfo hints, * res, * save;

    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if (-1 == aug_getaddrinfo(host, serv, &hints, &res))
        return AUG_BADSD;

    save = res;

    do {
        sd = aug_socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (AUG_BADSD == sd)
            continue; /* Ignore this one. */

        aug_getendpoint(res, ep);

        if (0 == aug_connect(sd, ep))
            break; /* Success. */

        if (-1 == aug_sclose(sd)) /* Ignore this one. */
            goto fail;

    } while ((res = res->ai_next));

    if (!res) /* errno set from final aug_connect() */
        return AUG_BADSD;

    aug_destroyaddrinfo(save);
    return sd;

 fail:
    aug_destroyaddrinfo(save);
    return AUG_BADSD;
}

AUGNET_API aug_sd
aug_udpserver(const char* host, const char* serv, struct aug_endpoint* ep)
{
    aug_sd sd;
    struct addrinfo hints, * res, * save;

    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if (-1 == aug_getaddrinfo(host, serv, &hints, &res))
        return AUG_BADSD;

    save = res;

    do {
        sd = aug_socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (AUG_BADSD == sd)
            continue; /* Error, try next one. */

        aug_getendpoint(res, ep);

        if (0 == aug_bind(sd, ep))
            break; /* Success. */

        if (-1 == aug_sclose(sd)) /* bind error, close and try next one */
            goto fail;

    } while ((res = res->ai_next));

    if (!res) /* errno from final aug_socket() or aug_bind(). */
        return AUG_BADSD;

    aug_destroyaddrinfo(save);
    return sd;

 fail:
    aug_destroyaddrinfo(save);
    return AUG_BADSD;
}

AUGNET_API struct aug_hostserv*
aug_parsehostserv(const char* src, struct aug_hostserv* dst)
{
    size_t len;
    char* serv;

    aug_strlcpy(dst->data_, src, sizeof(dst->data_));

    /* Locate host and serv separator. */

    if (!(serv = strrchr(dst->data_, ':'))) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EPARSE,
                       AUG_MSG("missing separator '%s'"), src);
        return NULL;
    }

    /* Calculate length of host part. */

    len = serv - dst->data_;

    /* Ensure host and serv parts exists. */

    if (!len) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EPARSE,
                       AUG_MSG("missing host part '%s'"), src);
        return NULL;
    }

    if ('\0' == *++serv) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EPARSE,
                       AUG_MSG("missing service part '%s'"), src);
        return NULL;
    }

    /* The host part of an ipv6 address may be contained within square
       brackets. */

    if ('[' == dst->data_[0]) {

        if (']' != dst->data_[len - 1]) {
            aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EPARSE,
                           AUG_MSG("unmatched brackets '%s'"), src);
            return NULL;
        }

        len -= 2;
        dst->host_ = dst->data_ + 1;
    } else
        dst->host_ = dst->data_;

    dst->host_[len] = '\0';
    dst->serv_ = serv;
    return dst;
}

AUGNET_API int
aug_setnodelay(aug_sd sd, int on)
{
    return aug_setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
}

AUGNET_API int
aug_established(aug_sd sd)
{
    int ret;
    struct aug_endpoint ep;
    if (!aug_getpeername(sd, &ep))
        ret = ENOTCONN == aug_errno() ? AUG_FAILNONE : -1;
    else
        ret = 0;

    return ret;
}
