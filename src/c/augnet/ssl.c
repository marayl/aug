/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/ssl.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#if ENABLE_SSL

# include "augsys/chan.h"   /* aug_clearready() */

# include "augsys/socket.h" /* aug_shutdown() */
# include "augsys/uio.h"

# include "augctx/base.h"
# include "augctx/errinfo.h"
# include "augctx/errno.h"

# include "augext/log.h"
# include "augext/stream.h"

# include <openssl/err.h>
# include <openssl/ssl.h>

# include <assert.h>
# include <string.h>        /* memcpy() */

# define SSLREAD_  0x01
# define SSLWRITE_ 0x02

struct buf_ {
    char buf_[4096];
    char* rd_, * wr_;
};

enum sslstate_ {
    NORMAL,

    /* Bytes stored in SSL buffer awaiting SSL_read(). */

    RDPEND,
    RDWANTRD,
    RDWANTWR,
    WRWANTRD,
    WRWANTWR,

    /* End of data. */

    RDZERO,
    SSLERR
};

/* FIXME: add sticky handling. */

struct impl_ {
    aug_chan chan_;
    aug_stream stream_;
    int refs_;
    aug_mpool* mpool_;
    aug_muxer_t muxer_;
    aug_sd sd_;

    /* The event mask from the user's perspective. */

    unsigned short mask_;
    struct aug_ssldata data_;
    SSL* ssl_;
    struct buf_ inbuf_, outbuf_;
    enum sslstate_ state_;
    struct aug_errinfo errinfo_;
    int shutdown_;
};

static aug_bool
error_(aug_chandler* handler, aug_chan* chan, aug_sd sd,
       unsigned short events)
{
    /* Exceptions may include non-error exceptions, such as high priority
       data. */

    if ((AUG_MDEVENTEX & events)) {

        /* Set socket-level error if one exists. */

        struct aug_errinfo errinfo;
        aug_setsockerrinfo(&errinfo, __FILE__, __LINE__, sd);

        if (errinfo.num_) {

            /* Error occurred. */

            aug_clearerror(handler, chan, &errinfo);
            return AUG_TRUE;
        }
    }

    return AUG_FALSE;
}

static int
bufempty_(struct buf_* x)
{
    /* Return true if the buffer is empty. */

    return x->rd_ == x->wr_;
}

static aug_bool
buffull_(struct buf_* x)
{
    /* Return true if the buffer is full. */

    const char* end = x->buf_ + sizeof(x->buf_);
    return x->wr_ == end;
}

static void
clearbuf_(struct buf_* x)
{
    /* Reset pointers to start of buffer. */

    x->rd_ = x->wr_ = x->buf_;
}

static size_t
readbuf_(struct buf_* x, void* buf, size_t size)
{
    size_t ret = AUG_MIN((size_t)(x->wr_ - x->rd_), size);
    if (ret) {

        /* Copy bytes from buffer at read pointer. */

        memcpy(buf, x->rd_, ret);
        x->rd_ += ret;

        /* Clear if read pointer caught write. */

        if (bufempty_(x))
            clearbuf_(x);
    }
    return ret;
}

static size_t
writebuf_(struct buf_* x, const void* buf, size_t size)
{
    const char* end = x->buf_ + sizeof(x->buf_);
    size_t ret = AUG_MIN((size_t)(end - x->wr_), size);
    if (ret) {

        /* Copy bytes to buffer at write pointer. */

        memcpy(x->wr_, buf, ret);
        x->wr_ += ret;
    }
    return ret;
}

static size_t
readbufv_(struct buf_* x, const struct iovec* iov, int size)
{
    size_t n, ret = 0;

    /* For each entry in the vector. */

    for (; size; --size, ++iov) {

        n = readbuf_(x, iov->iov_base, iov->iov_len);
        ret += n;

        /* Not enough to satisfy entry. */

        if (n < (size_t)iov->iov_len)
            break;
    }
    return ret;
}

static size_t
writebufv_(struct buf_* x, const struct iovec* iov, int size)
{
    size_t n, ret = 0;

    /* For each entry in the vector. */

    for (; size; --size, ++iov) {

        n = writebuf_(x, iov->iov_base, iov->iov_len);
        ret += n;

        /* Not enough to satisfy entry. */

        if (n < (size_t)iov->iov_len)
            break;
    }
    return ret;
}

static aug_result
shutwr_(struct impl_* impl, struct aug_errinfo* errinfo)
{
    int ret;

    AUG_CTXDEBUG3(aug_tlx, "SSL: shutdown: id=[%u]", impl->data_.id_);
    ret = SSL_shutdown(impl->ssl_);
    aug_sshutdown(impl->sd_, SHUT_WR);

    if (ret < 0) {
        aug_setsslerrinfo(errinfo, __FILE__, __LINE__, ERR_get_error());
        return AUG_FAILERROR;
    }
    return AUG_SUCCESS;
}

static ssize_t
sslread_(SSL* ssl, struct buf_* x)
{
    /* Read bytes into buffer at write pointer. */

    const char* end = x->buf_ + sizeof(x->buf_);
    ssize_t n = (ssize_t)(end - x->wr_);
    if (n) {
        AUG_CTXDEBUG3(aug_tlx, "SSL: ssl read to input buffer");
        n = SSL_read(ssl, x->wr_, n);
        if (0 < n)
            x->wr_ += n;
    } else {

        /* Input buffer is full, try handshake. */

        AUG_CTXDEBUG3(aug_tlx, "SSL: handshake without read");
        n = SSL_do_handshake(ssl);
    }
    return n;
}

static ssize_t
sslwrite_(SSL* ssl, struct buf_* x)
{
    /* Write bytes from buffer at read pointer. */

    ssize_t n = (ssize_t)(x->wr_ - x->rd_);
    if (n) {
        AUG_CTXDEBUG3(aug_tlx, "SSL: ssl write from output buffer");
        n = SSL_write(ssl, x->rd_, n);
        if (0 < n) {
            x->rd_ += n;

            /* Clear if read pointer caught write. */

            if (bufempty_(x))
                clearbuf_(x);
        }
    } else {

        /* Output buffer is empty, try handshake. */

        AUG_CTXDEBUG3(aug_tlx, "SSL: handshake without write");
        n = SSL_do_handshake(ssl);
    }
    return n;
}

static int
realmask_(struct impl_* impl)
{
    /* Calculate the real mask to be used by the muxer. */

    unsigned short mask = 0;

    switch (impl->state_) {
    case NORMAL:
        if (!bufempty_(&impl->outbuf_))
            mask = AUG_MDEVENTRDWR;
        else
            mask = AUG_MDEVENTRD;
        break;
    case RDPEND:
        break;
    case RDWANTRD:
    case WRWANTRD:
        mask = AUG_MDEVENTRD;
        break;
    case RDWANTWR:
    case WRWANTWR:
        mask = AUG_MDEVENTWR;
        break;
    case RDZERO:
    case SSLERR:
        break;
    }

    return mask;
}

static unsigned short
userevents_(struct impl_* impl)
{
    unsigned short events = 0;

    /* End of data and errors are communicated via aug_read(). */

    if ((impl->mask_ & AUG_MDEVENTRD)
        && (!bufempty_(&impl->inbuf_) || RDZERO == impl->state_
            || SSLERR == impl->state_))
        events |= AUG_MDEVENTRD;

    if ((impl->mask_ & AUG_MDEVENTWR) && !buffull_(&impl->outbuf_))
        events |= AUG_MDEVENTWR;

    return events;
}

static void
updateevents_(struct impl_* impl)
{
    unsigned short real, user;

    real = realmask_(impl);
    user = userevents_(impl);

    aug_setmdeventmask(impl->muxer_, impl->sd_, real);

    AUG_CTXDEBUG3(aug_tlx, "SSL: events: id=[%u], realmask=[%u],"
                  " usermask=[%u], userevents=[%u]", impl->data_.id_,
                  (unsigned)real, (unsigned)impl->mask_, (unsigned)user);
}

static void
readwrite_(struct impl_* impl, int rw)
{
    ssize_t ret;

    if (rw & SSLREAD_) {

        /* Perform read operation.  If input buffer is full, sslread_() will
           perform SSL_do_handshake(). */

        ret = sslread_(impl->ssl_, &impl->inbuf_);
        switch (SSL_get_error(impl->ssl_, ret)) {
        case SSL_ERROR_NONE:
            AUG_CTXDEBUG3(aug_tlx, "SSL: %ld bytes read to input buffer",
                          (long)ret);
            if ((ret = SSL_pending(impl->ssl_))) {
                AUG_CTXDEBUG3(aug_tlx, "SSL: %d bytes pending for immediate"
                              " read", ret);
                impl->state_ = RDPEND;
                goto done;
            }
            impl->state_ = NORMAL;
            break;
        case SSL_ERROR_ZERO_RETURN:
            AUG_CTXDEBUG3(aug_tlx, "SSL: end of data");
            if (!impl->shutdown_) {
                AUG_CTXDEBUG3(aug_tlx, "SSL: shutting-down", ret);
                SSL_shutdown(impl->ssl_);
            }
            impl->state_ = RDZERO;
            goto done;
        case SSL_ERROR_WANT_READ:
            AUG_CTXDEBUG3(aug_tlx, "SSL: read wants read");
            impl->state_ = RDWANTRD;
            goto done;
        case SSL_ERROR_WANT_WRITE:
            AUG_CTXDEBUG3(aug_tlx, "SSL: read wants write");
            impl->state_ = RDWANTWR;
            goto done;
        default:

            /* Save error locally. */

            aug_setsslerrinfo(&impl->errinfo_, __FILE__, __LINE__,
                              ERR_get_error());
            impl->state_ = SSLERR;
            goto done;
        }
    }

    if (rw & SSLWRITE_) {

        /* Perform write operation.  If output buffer is empty, sslwrite_()
           will perform SSL_do_handshake(). */

        ret = sslwrite_(impl->ssl_, &impl->outbuf_);
        switch (SSL_get_error(impl->ssl_, ret)) {
        case SSL_ERROR_NONE:
            AUG_CTXDEBUG3(aug_tlx, "SSL: %d bytes written from output buffer",
                          ret);
            impl->state_ = NORMAL;
            break;
        case SSL_ERROR_WANT_WRITE:
            AUG_CTXDEBUG3(aug_tlx, "SSL: write wants write");
            impl->state_ = WRWANTWR;
            break;
        case SSL_ERROR_WANT_READ:
            AUG_CTXDEBUG3(aug_tlx, "SSL: write wants read");
            impl->state_ = WRWANTRD;
            break;
        default:

            /* Save error locally. */

            aug_setsslerrinfo(&impl->errinfo_, __FILE__, __LINE__,
                              ERR_get_error());
            impl->state_ = SSLERR;
            break;
        }

        /* If shutdown is pending and output buffer is now empty, do
           shutdown. */

        if (impl->shutdown_ && bufempty_(&impl->outbuf_)
            && AUG_ISFAIL(shutwr_(impl, &impl->errinfo_))) {

            /* Save error locally. */

            impl->state_ = SSLERR;
        }
    }
 done:
    return;
}

static aug_result
close_(struct impl_* impl)
{
    AUG_CTXDEBUG3(aug_tlx, "SSL: clearing io-event mask: id=[%u]",
                  impl->data_.id_);
    aug_setmdeventmask(impl->muxer_, impl->sd_, 0);
    return aug_sclose(impl->sd_);
}

static void*
cast_(struct impl_* impl, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_chanid)) {
        aug_retain(&impl->chan_);
        return &impl->chan_;
    } else if (AUG_EQUALID(id, aug_streamid)) {
        aug_retain(&impl->stream_);
        return &impl->stream_;
    }
    return NULL;
}

static void
retain_(struct impl_* impl)
{
    assert(0 < impl->refs_);
    ++impl->refs_;
}

static void
release_(struct impl_* impl)
{
    assert(0 < impl->refs_);
    if (0 == --impl->refs_) {
        aug_mpool* mpool = impl->mpool_;
        if (AUG_BADSD != impl->sd_)
            close_(impl);
        SSL_free(impl->ssl_);
        aug_release(impl->data_.handler_);
        aug_freemem(mpool, impl);
        aug_release(mpool);
    }
}

static void*
ccast_(aug_chan* ob, const char* id)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    return cast_(impl, id);
}

static void
cretain_(aug_chan* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    retain_(impl);
}

static void
crelease_(aug_chan* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    release_(impl);
}

static aug_result
cclose_(aug_chan* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    aug_result result;
    AUG_CTXDEBUG3(aug_tlx, "SSL: closing file: id=[%u]", impl->data_.id_);
    result = close_(impl);
    impl->sd_ = AUG_BADSD;
    return result;
}

static aug_chan*
cprocess_(aug_chan* ob, aug_chandler* handler, aug_bool* fork)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    unsigned short events;
    int rw = 0;

    /* Channel closed. */

    if (AUG_BADSD == impl->sd_)
        return NULL;

    events = aug_getmdevents(impl->muxer_, impl->sd_);

    /* Close socket on error. */

    if (error_(handler, &impl->chan_, impl->sd_, events))
        return NULL;

    /* TODO: how should any remaining exceptional events be handled? */

    /* Determine which SSL operations are to be performed. */

    switch (impl->state_) {
    case RDPEND:
        if (!buffull_(&impl->inbuf_)) {

            /* Data is available for immediate read to input buffer. */
            break;
        }

        /* Fall through to normal case. */

    case NORMAL:
        if (events & AUG_MDEVENTRD)
            rw = SSLREAD_;
        if (events & AUG_MDEVENTWR)
            rw |= SSLWRITE_;
        break;
    case RDWANTRD:
        if (events & AUG_MDEVENTRD)
            rw = SSLREAD_;
        break;
    case RDWANTWR:
        if (events & AUG_MDEVENTWR)
            rw = SSLREAD_;
        break;
    case WRWANTRD:
        if (events & AUG_MDEVENTRD)
            rw = SSLWRITE_;
        break;
    case WRWANTWR:
        if (events & AUG_MDEVENTWR)
            rw = SSLWRITE_;
        break;
    case RDZERO:
    case SSLERR:
        break;
    }

    if (rw)
        readwrite_(impl, rw);
    else
        AUG_CTXDEBUG3(aug_tlx, "SSL: readwrite_() skipped");

    events = userevents_(impl);
    AUG_CTXDEBUG3(aug_tlx, "SSL: userevents_(): id=[%u], events=[%u]",
                  impl->data_.id_, (unsigned)events);

    if (events) {

        if (!aug_clearready(handler, &impl->chan_, events)) {

            /* No need to update events if file is being removed - indicated
               by false return. */

            return NULL;
        }
        updateevents_(impl);

    }

    retain_(impl);
    return ob;
}

static aug_result
csetmask_(aug_chan* ob, unsigned short mask)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);

    AUG_CTXDEBUG3(aug_tlx, "SSL: setting event mask: id=[%u], mask=[%u]",
                  impl->data_.id_, (unsigned)mask);

    impl->mask_ = mask;
    updateevents_(impl);
    return AUG_SUCCESS;
}

static unsigned short
cgetmask_(aug_chan* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    return impl->mask_;
}

static unsigned
cgetid_(aug_chan* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    return impl->data_.id_;
}

static char*
cgetname_(aug_chan* ob, char* dst, unsigned size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    struct aug_endpoint ep;

    if (!aug_getpeername(impl->sd_, &ep) || !aug_endpointntop(&ep, dst, size))
        return NULL;

    return dst;
}

static aug_bool
cisready_(aug_chan* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);

    /* True if closed. */

    if (AUG_BADSD == impl->sd_)
        return AUG_TRUE;

    return userevents_(impl)
        || (RDPEND == impl->state_ && !buffull_(&impl->inbuf_));
}

static const struct aug_chanvtbl cvtbl_ = {
    ccast_,
    cretain_,
    crelease_,
    cclose_,
    cprocess_,
    csetmask_,
    cgetmask_,
    cgetid_,
    cgetname_,
    cisready_
};

static void*
scast_(aug_stream* ob, const char* id)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    return cast_(impl, id);
}

static void
sretain_(aug_stream* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    retain_(impl);
}

static void
srelease_(aug_stream* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    release_(impl);
}

static aug_result
sshutdown_(aug_stream* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);

    impl->shutdown_ = 1;

    /* If the output buffer is not empty, the shutdown call will be delayed
       until the remaining data has been written. */

    return bufempty_(&impl->outbuf_) ? shutwr_(impl, aug_tlerr) : AUG_SUCCESS;
}

static aug_rsize
sread_(aug_stream* ob, void* buf, size_t size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    ssize_t ret;

    if (SSLERR == impl->state_) {

        /* Restore saved error. */

        aug_seterrinfo(aug_tlerr, impl->errinfo_.file_, impl->errinfo_.line_,
                       impl->errinfo_.src_, impl->errinfo_.num_,
                       impl->errinfo_.desc_);
        return AUG_FAILERROR;
    }

    /* Only return end once all data has been read from buffer. */

    if (RDZERO == impl->state_ && bufempty_(&impl->inbuf_))
        return AUG_SUCCESS;

    /* Fail with EWOULDBLOCK is non-blocking operation would have blocked. */

    if (bufempty_(&impl->inbuf_))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__,
                                   EWOULDBLOCK);

    AUG_CTXDEBUG3(aug_tlx, "SSL: user read from input buffer: id=[%u]",
                  impl->data_.id_);

    ret = (ssize_t)readbuf_(&impl->inbuf_, buf, size);
    updateevents_(impl);
    return AUG_MKRESULT(ret);
}

static aug_rsize
sreadv_(aug_stream* ob, const struct iovec* iov, int size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    ssize_t ret;

    if (SSLERR == impl->state_) {

        /* Restore saved error. */

        aug_seterrinfo(aug_tlerr, impl->errinfo_.file_, impl->errinfo_.line_,
                       impl->errinfo_.src_, impl->errinfo_.num_,
                       impl->errinfo_.desc_);
        return AUG_FAILERROR;
    }

    /* Only return end once all data has been read from buffer. */

    if (RDZERO == impl->state_ && bufempty_(&impl->inbuf_))
        return AUG_SUCCESS;

    /* Fail with EWOULDBLOCK is non-blocking operation would have blocked. */

    if (bufempty_(&impl->inbuf_))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__,
                                   EWOULDBLOCK);

    AUG_CTXDEBUG3(aug_tlx, "SSL: user readv from input buffer: id=[%u]",
                  impl->data_.id_);

    ret = (ssize_t)readbufv_(&impl->inbuf_, iov, size);
    updateevents_(impl);
    return AUG_MKRESULT(ret);
}

static aug_rsize
swrite_(aug_stream* ob, const void* buf, size_t size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    ssize_t ret;

    /* Fail with EWOULDBLOCK is non-blocking operation would have blocked. */

    if (buffull_(&impl->outbuf_))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__,
                                   EWOULDBLOCK);

    if (impl->shutdown_)
#if !defined(_WIN32)
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, ESHUTDOWN);
#else /* _WIN32 */
        return aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__,
                                   WSAESHUTDOWN);
#endif /* _WIN32 */

    AUG_CTXDEBUG3(aug_tlx, "SSL: user write to output buffer: id=[%u]",
                  impl->data_.id_);

    ret = (ssize_t)writebuf_(&impl->outbuf_, buf, size);
    updateevents_(impl);
    return AUG_MKRESULT(ret);
}

static aug_rsize
swritev_(aug_stream* ob, const struct iovec* iov, int size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    ssize_t ret;

    /* Fail with EWOULDBLOCK is non-blocking operation would have blocked. */

    if (buffull_(&impl->outbuf_))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__,
                                   EWOULDBLOCK);

    if (impl->shutdown_)
#if !defined(_WIN32)
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, ESHUTDOWN);
#else /* _WIN32 */
        return aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__,
                                   WSAESHUTDOWN);
#endif /* _WIN32 */

    AUG_CTXDEBUG3(aug_tlx, "SSL: user writev to output buffer: id=[%u]",
                  impl->data_.id_);

    ret = (ssize_t)writebufv_(&impl->outbuf_, iov, size);
    updateevents_(impl);
    return AUG_MKRESULT(ret);
}

static const struct aug_streamvtbl svtbl_ = {
    scast_,
    sretain_,
    srelease_,
    sshutdown_,
    sread_,
    sreadv_,
    swrite_,
    swritev_
};

static struct impl_*
createssl_(aug_mpool* mpool, unsigned id, aug_muxer_t muxer, aug_sd sd,
           unsigned short mask, aug_chandler* handler,
           struct ssl_ctx_st* sslctx)
{
    struct impl_* impl;
    SSL* ssl;
    BIO* bio;

    if (AUG_ISFAIL(aug_setmdeventmask(muxer, sd, AUG_MDEVENTRDWR)))
        return NULL;

    if (!(impl = aug_allocmem(mpool, sizeof(struct impl_)))) {
        aug_setmdeventmask(muxer, sd, 0);
        return NULL;
    }

    impl->chan_.vtbl_ = &cvtbl_;
    impl->chan_.impl_ = NULL;
    impl->stream_.vtbl_ = &svtbl_;
    impl->stream_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;
    impl->muxer_ = muxer;
    impl->sd_ = sd;
    impl->mask_ = mask;
    impl->data_.handler_ = handler;
    impl->data_.id_ = id;

    /* SSL state */

    ssl = SSL_new(sslctx);
    bio = BIO_new_socket((int)sd, BIO_NOCLOSE);
    SSL_set_bio(ssl, bio, bio);
    SSL_set_app_data(ssl, &impl->data_);

    impl->ssl_ = ssl;
    clearbuf_(&impl->inbuf_);
    clearbuf_(&impl->outbuf_);
    impl->state_ = NORMAL;
    impl->shutdown_ = 0;

    aug_retain(mpool);
    aug_retain(handler);
    return impl;
}

AUGNET_API void
aug_setsslerrinfo(struct aug_errinfo* errinfo, const char* file, int line,
                  unsigned long err)
{
    aug_seterrinfo(errinfo, file, line, "ssl", err,
                   "%s: %s: %s",
                   ERR_lib_error_string(err),
                   ERR_func_error_string(err),
                   ERR_reason_error_string(err));

    /* Log those that are not set in errinfo record. */

    while ((err = ERR_get_error()))
        aug_ctxerror(aug_tlx, "%s: %s: %s",
                     ERR_lib_error_string(err),
                     ERR_func_error_string(err),
                     ERR_reason_error_string(err));
}

AUGNET_API aug_chan*
aug_createsslclient(aug_mpool* mpool, unsigned id, aug_muxer_t muxer,
                    aug_sd sd, unsigned short mask, aug_chandler* handler,
                    struct ssl_ctx_st* sslctx)
{
    struct impl_* impl = createssl_(mpool, id, muxer, sd, mask, handler,
                                    sslctx);
    if (!impl)
        return NULL;

    /* Write client-initiated handshake. */

    impl->state_ = WRWANTWR;

    SSL_set_connect_state(impl->ssl_);
    return &impl->chan_;
}

AUGNET_API aug_chan*
aug_createsslserver(aug_mpool* mpool, unsigned id, aug_muxer_t muxer,
                    aug_sd sd, unsigned short mask, aug_chandler* handler,
                    struct ssl_ctx_st* sslctx)
{
    struct impl_* impl = createssl_(mpool, id, muxer, sd, mask, handler,
                                    sslctx);
    if (!impl)
        return NULL;

    /* Read client-initiated handshake. */

    impl->state_ = RDWANTRD;

    SSL_set_accept_state(impl->ssl_);
    return &impl->chan_;
}

#endif /* ENABLE_SSL */
