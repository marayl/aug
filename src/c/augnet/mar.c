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
#define AUGNET_BUILD
#include "augnet/mar.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augnet/http.h"

#include "augutil/object.h"
#include "augutil/xstr.h"

#include "augmar/mar.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/errno.h"

#include <assert.h>
#include <string.h>

struct aug_marparser_ {
    aug_mpool* mpool_;
    aug_marstore* marstore_;
    aug_httphandler httphandler_;
    aug_httpparser_t http_;
    aug_xstr_t request_;
    aug_mar* mar_;
};

static void*
cast_(aug_httphandler* ob, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_httphandlerid)) {
        aug_retain(ob);
        return ob;
    }
    return NULL;
}

static void
retain_(aug_httphandler* ob)
{
}

static void
release_(aug_httphandler* ob)
{
}

static aug_result
request_(aug_httphandler* ob, const char* request)
{
    struct aug_marparser_* parser
        = AUG_PODIMPL(struct aug_marparser_, httphandler_, ob);

    assert(!parser->request_);
    assert(!parser->mar_);

    parser->request_ = aug_createxstr(parser->mpool_, 0);

    if (!parser->request_)
        return -1; /* Allocation failed. */

    if (!(parser->mar_ = aug_getmar(parser->marstore_, request))) {
        aug_destroyxstr(parser->request_);
        parser->request_ = NULL;
        return -1;
    }
    aug_xstrcpys(parser->request_, request);
    return 0;
}

static aug_result
field_BIN_(aug_httphandler* ob, const char* name, const char* value)
{
    struct aug_marparser_* parser
        = AUG_PODIMPL(struct aug_marparser_, httphandler_, ob);

    struct aug_field field;

    assert(parser->request_);
    assert(parser->mar_);

    field.name_ = name;
    field.value_ = value;
    field.size_ = (unsigned)strlen(value);

    /* EXCEPT: field_BIN_ -> aug_putfield_BIN; */

    return aug_putfield_BIN(parser->mar_, &field);
}

static aug_result
csize_BIN_(aug_httphandler* ob, unsigned csize)
{
    struct aug_marparser_* parser
        = AUG_PODIMPL(struct aug_marparser_, httphandler_, ob);

    assert(parser->request_);
    assert(parser->mar_);

    /* EXCEPT: csize_BIN_ -> aug_truncatemar_BIN; */

    if (aug_truncatemar_BIN(parser->mar_, csize) < 0
        || aug_seekmar(parser->mar_, AUG_SET, 0) < 0)
        return -1;

    return 0;
}

static aug_result
cdata_BIN_(aug_httphandler* ob, const void* buf, unsigned len)
{
    struct aug_marparser_* parser
        = AUG_PODIMPL(struct aug_marparser_, httphandler_, ob);

    assert(parser->request_);
    assert(parser->mar_);

    /* Returns aug_rsize. */

    /* EXCEPT: cdata_BIN_ -> aug_writemar_BIN; */

    if (aug_writemar_BIN(parser->mar_, buf, len) < 0)
        return -1;

    return 0;
}

static aug_result
end_(aug_httphandler* ob, aug_bool commit)
{
    struct aug_marparser_* parser
        = AUG_PODIMPL(struct aug_marparser_, httphandler_, ob);

    aug_result result;

    if (commit) {

        if (!parser->request_)
            return 0; /* Blank line. */

        result = aug_putmar(parser->marstore_, aug_xstr(parser->request_),
                            parser->mar_);
    } else
        result = 0;

    if (parser->request_) {
        aug_destroyxstr(parser->request_);
        parser->request_ = NULL;
    }

    if (parser->mar_) {
        aug_release(parser->mar_);
        parser->mar_ = NULL;
    }

    return result;
}

static const struct aug_httphandlervtbl vtbl_ = {
    cast_,
    retain_,
    release_,
    request_,
    field_BIN_,
    csize_BIN_,
    cdata_BIN_,
    end_
};

AUGNET_API aug_marparser_t
aug_createmarparser(aug_mpool* mpool, aug_marstore* marstore, unsigned size)
{
    aug_marparser_t parser = aug_allocmem(mpool,
                                          sizeof(struct aug_marparser_));
    if (!parser)
        return NULL;

    parser->mpool_ = mpool;
    parser->marstore_ = marstore;
    parser->httphandler_.vtbl_ = &vtbl_;
    parser->httphandler_.impl_ = NULL;
    parser->http_ = NULL;
    parser->request_ = NULL;
    parser->mar_ = NULL;

    if (!(parser->http_ = aug_createhttpparser(mpool, &parser->httphandler_,
                                               size))) {
        aug_freemem(mpool, parser);
        return NULL;
    }

    aug_retain(mpool);
    aug_retain(marstore);
    return parser;
}

AUGNET_API void
aug_destroymarparser(aug_marparser_t parser)
{
    aug_mpool* mpool = parser->mpool_;

    if (parser->mar_)
        aug_release(parser->mar_);

    if (parser->request_)
        aug_destroyxstr(parser->request_);

    aug_destroyhttpparser(parser->http_);
    aug_release(parser->marstore_);

    aug_freemem(mpool, parser);
    aug_release(mpool);
}

AUGNET_API aug_result
aug_appendmar(aug_marparser_t parser, const char* buf, unsigned size)
{
    return aug_appendhttp(parser->http_, buf, size);
}

AUGNET_API aug_result
aug_finishmar(aug_marparser_t parser)
{
    return aug_finishhttp(parser->http_);
}
