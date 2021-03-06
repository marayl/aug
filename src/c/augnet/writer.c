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
#include "augnet/writer.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augsys/uio.h"
#include "augutil/list.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include "augabi.h"

#if !defined(_WIN32)
# if HAVE_ALLOCA_H
#  include <alloca.h>
# endif /* HAVE_ALLOCA_H */
#else /* _WIN32 */
# include <malloc.h> /* alloca() */
#endif /* _WIN32 */

#include <assert.h>

struct buf_ {
    AUG_ENTRY(buf_);
    aug_blob* blob_;
};

AUG_HEAD(list_, buf_);

struct aug_writer_ {
    aug_mpool* mpool_;
    struct list_ list_;
    size_t part_;
    unsigned size_;
};

static struct buf_*
createbuf_(aug_mpool* mpool, aug_blob* blob)
{
    struct buf_* buf;
    assert(blob);

    if (!(buf = aug_allocmem(mpool, sizeof(struct buf_))))
        return NULL;

    buf->blob_ = blob;
    aug_retain(blob);
    return buf;
}

static void
destroybuf_(aug_mpool* mpool, struct buf_* buf)
{
    aug_release(buf->blob_);
    aug_freemem(mpool, buf);
}

static void
popbufs_(aug_writer_t writer, const struct iovec* iov, size_t num)
{
    while (num && (size_t)iov->iov_len <= num) {

        struct buf_* it = AUG_FIRST(&writer->list_);
        assert(it);
        AUG_REMOVE_HEAD(&writer->list_);

        destroybuf_(writer->mpool_, it);

        --writer->size_;
        num -= (iov++)->iov_len;
    }

    /* Store offset into current for next write. */

    writer->part_ = num;
}

AUGNET_API aug_writer_t
aug_createwriter(aug_mpool* mpool)
{
    aug_writer_t writer = aug_allocmem(mpool, sizeof(struct aug_writer_));
    if (!writer)
        return NULL;

    writer->mpool_ = mpool;
    AUG_INIT(&writer->list_);
    writer->part_ = 0;
    writer->size_ = 0;

    aug_retain(mpool);
    return writer;
}

AUGNET_API void
aug_destroywriter(aug_writer_t writer)
{
    aug_mpool* mpool = writer->mpool_;
    struct buf_* it;

    /* Destroy in single batch to avoid multiple calls to aug_lock(). */

    while ((it = AUG_FIRST(&writer->list_))) {
        AUG_REMOVE_HEAD(&writer->list_);
        destroybuf_(mpool, it);
    }

    aug_freemem(mpool, writer);
    aug_release(mpool);
}

AUGNET_API aug_result
aug_appendwriter(aug_writer_t writer, aug_blob* blob)
{
    struct buf_* buf;
    assert(blob);
    if (!(buf = createbuf_(writer->mpool_, blob)))
        return -1;

    AUG_INSERT_TAIL(&writer->list_, buf);
    ++writer->size_;
    return 0;
}

AUGNET_API aug_bool
aug_writerempty(aug_writer_t writer)
{
    return AUG_EMPTY(&writer->list_);
}

AUGNET_API aug_rsize
aug_writersize(aug_writer_t writer)
{
    struct buf_* it;
    size_t size = 0;

    AUG_FOREACH(it, &writer->list_) {

        size_t len;
        if (!aug_getblobdata(it->blob_, &len)) {
            aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EDOMAIN,
                            AUG_MSG("failed conversion from var to buffer"));
            return -1;
        }

        size += len;
    }

    return (ssize_t)size;
}

AUGNET_API aug_rsize
aug_writesome_BI(aug_writer_t writer, aug_stream* stream)
{
    unsigned i, size;
    struct iovec* iov;
    struct buf_* it;
    size_t len;
    aug_rsize rsize;

    /* As a precaution, limit use of stack space. */

    if (0 == (size = AUG_MIN(writer->size_, 64)))
        return 0;

    iov = alloca(sizeof(struct iovec) * size);

    /* Map each buffer to an entry in the iov table. */

    i = 0;
    AUG_FOREACH(it, &writer->list_) {

        if (!(iov[i].iov_base = (void*)aug_getblobdata(it->blob_, &len))) {
            aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EDOMAIN,
                            AUG_MSG("failed conversion from var to buffer"));
            return -1;
        }

        iov[i].iov_len = (int)len;
        if (++i == size)
            break; /* Exhausted vector size. */
    }

    /* Add offset to first entry. */

    iov->iov_base = (char*)iov->iov_base + writer->part_;
    iov->iov_len -= (int)writer->part_;

    /* EXCEPT: aug_writesome_BI -> aug_writev_BI; */

    if (0 <= (rsize = aug_writev_BI(stream, iov, size))) {

        /* Pop any completed buffers from queue. */

        popbufs_(writer, iov, rsize);
    }

    return rsize;
}
