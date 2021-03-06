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
#define AUGMAR_BUILD
#include "augmar/mar.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augmar/body_.h"
#include "augmar/format_.h"
#include "augmar/header_.h"
#include "augmar/info_.h"
#include "augmar/mfile_.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include "augext/blob.h"
#include "augext/log.h"

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define READ_  0x0001
#define WRITE_ 0x0002

#define READABLE_(x) ((x) && (torw_((x)->flags_) & READ_))
#define WRITABLE_(x) ((x) && (torw_((x)->flags_) & WRITE_))

static unsigned
torw_(int from)
{
    switch (from & (AUG_RDONLY | AUG_WRONLY | AUG_RDWR)) {
    case AUG_RDONLY:
        return READ_;
    case AUG_WRONLY:
        return WRITE_;
    case AUG_RDWR:
        return READ_ | WRITE_;
    }
    return 0;
}

static aug_result
init_BIN_(aug_seq_t seq, struct aug_info_* info)
{
    static const aug_proto_t PROTO = 3U;
    unsigned size = aug_seqsize_(seq);

    /* An existing archive will be at least as big as the minimum size. */

    if (AUG_LEADERSIZE <= size) {

        if (aug_info_(seq, info) < 0)
            return -1;

        /* Verify version number embedded within header. */

        if (PROTO != info->proto_) {

            aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                            AUG_MSG("invalid protocol number [%d]"),
                            (int)info->proto_);
            return -1;
        }
    } else {

        char* ptr;

        /* EXCEPT: init_BIN_ -> aug_resizeseq_BIN_; */

        if (aug_setregion_(seq, 0, size) < 0
            || !(ptr = aug_resizeseq_BIN_(seq, AUG_LEADERSIZE)))
            return -1;

        memcpy(ptr + AUG_MAGICOFF, AUG_MAGIC, sizeof(aug_magic_t));
        aug_encodeproto((aug_proto_t)(info->proto_ = PROTO),
                        ptr + AUG_PROTOOFF);
        aug_encodefields((aug_fields_t)(info->fields_ = 0),
                         ptr + AUG_FIELDSOFF);
        aug_encodehsize((aug_hsize_t)(info->hsize_ = 0),
                        ptr + AUG_HSIZEOFF);
        aug_encodebsize((aug_bsize_t)(info->bsize_ = 0),
                        ptr + AUG_BSIZEOFF);
    }

    return 0;
}

struct impl_ {
    aug_mar mar_;
    aug_blob blob_;
    int refs_;
    aug_mpool* mpool_;
    aug_seq_t seq_;
    struct aug_info_ info_;
    int flags_;
    unsigned offset_;
};

static void
destroy_(struct impl_* impl)
{
    if (WRITABLE_(impl)) {

        /* TODO: warn about corruption if either of these two operations
           fail. */

        struct aug_info_ local;
        if (aug_info_(impl->seq_, &local) < 0)
            goto done;

        if (0 != memcmp(&local, &impl->info_, sizeof(local))
            && aug_setinfo_(impl->seq_, &impl->info_) < 0)
            goto done;
    }

 done:
    aug_destroyseq_(impl->seq_);
}

static void*
cast_(struct impl_* impl, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_marid)) {
        aug_retain(&impl->mar_);
        return &impl->mar_;
    } else if (AUG_EQUALID(id, aug_blobid)) {
        aug_retain(&impl->blob_);
        return &impl->blob_;
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
        destroy_(impl);
        aug_release(mpool);
    }
}

static void*
mar_cast_(aug_mar* obj, const char* id)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    return cast_(impl, id);
}

static void
mar_retain_(aug_mar* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    retain_(impl);
}

static void
mar_release_(aug_mar* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    release_(impl);
}

static aug_result
compact_(aug_mar* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    if (!WRITABLE_(impl)) {

        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("invalid archive handle"));
        return -1;
    }
    return 0; /* Not implemented. */
}

static aug_rint
clear_BIN_(aug_mar* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    if (!WRITABLE_(impl)) {

        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("invalid archive handle"));
        return -1;
    }

    /* EXCEPT: clear_BIN_ -> aug_clearfields_BIN_; */

    return aug_clearfields_BIN_(impl->seq_, &impl->info_);
}

static aug_result
deln_BIN_(aug_mar* obj, unsigned n)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    if (!WRITABLE_(impl)) {

        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("invalid archive handle"));
        return -1;
    }

    /* EXCEPT: deln_BIN_ -> aug_delfieldn_BIN_; */

    return aug_delfieldn_BIN_(impl->seq_, &impl->info_, n);
}

static aug_rint
delp_BIN_(aug_mar* obj, const char* name)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    if (!WRITABLE_(impl)) {

        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("invalid archive handle"));
        return -1;
    }

    /* EXCEPT: delp_BIN_ -> aug_delfieldp_BIN_; */

    return aug_delfieldp_BIN_(impl->seq_, &impl->info_, name);
}

static aug_rint
getn_(aug_mar* obj, unsigned n, const void** value)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    if (!READABLE_(impl)) {

        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("invalid archive handle"));
        return -1;
    }
    return aug_getfieldn_(impl->seq_, &impl->info_, n, value);
}

static aug_rint
getp_(aug_mar* obj, const char* name, const void** value)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    if (!READABLE_(impl)) {

        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("invalid archive handle"));
        return -1;
    }
    return aug_getfieldp_(impl->seq_, &impl->info_, name, value);
}

static aug_result
get_(aug_mar* obj, unsigned n, struct aug_field* field)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    if (!READABLE_(impl)) {

        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("invalid archive handle"));
        return -1;
    }
    return aug_getfield_(impl->seq_, &impl->info_, n, field);
}

static aug_result
putn_BIN_(aug_mar* obj, unsigned n, const void* value, unsigned size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    if (!WRITABLE_(impl)) {

        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("invalid archive handle"));
        return -1;
    }

    /* EXCEPT: putn_BIN_ -> aug_putfieldn_BIN_; */

    return aug_putfieldn_BIN_(impl->seq_, &impl->info_, n, value, size);
}

static aug_rint
putp_BIN_(aug_mar* obj, const char* name, const void* value, unsigned size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    if (!WRITABLE_(impl)) {

        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("invalid archive handle"));
        return -1;
    }

    /* EXCEPT: putp_BIN_ -> aug_putfieldp_BIN_; */

    return aug_putfieldp_BIN_(impl->seq_, &impl->info_, name, value, size);
}

static aug_rint
put_BIN_(aug_mar* obj, const struct aug_field* field)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    if (!WRITABLE_(impl)) {

        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("invalid archive handle"));
        return -1;
    }

    /* EXCEPT: put_BIN_ -> aug_putfieldp_BIN_; */

    return aug_putfieldp_BIN_(impl->seq_, &impl->info_, field->name_,
                              field->value_, field->size_);
}

static aug_result
ntop_(aug_mar* obj, unsigned n, const char** name)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    if (!READABLE_(impl)) {

        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("invalid archive handle"));
        return -1;
    }
    return aug_fieldntop_(impl->seq_, &impl->info_, n, name);
}

static aug_rint
pton_(aug_mar* obj, const char* name)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    if (!READABLE_(impl)) {

        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("invalid archive handle"));
        return -1;
    }
    return aug_fieldpton_(impl->seq_, &impl->info_, name);
}

static unsigned
getcount_(aug_mar* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    return impl->info_.fields_;
}

static aug_result
insert_BIN_(aug_mar* obj, const char* path)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    aug_mpool* mpool;
    aug_mfile_t mfile;
    const void* addr;
    unsigned size;

    mpool = aug_seqmpool_(impl->seq_);

    /* EXCEPT: insert_BIN_ -> aug_openmfile_IN_; */

    mfile = aug_openmfile_IN_(mpool, path, AUG_RDONLY, 0, 0);
    aug_release(mpool);

    if (!mfile)
        return -1;

    if (0 != (size = aug_mfileresvd_(mfile))) {

        /* EXCEPT: insert_BIN_ -> aug_mapmfile_BIN_; */
        /* EXCEPT: insert_BIN_ -> aug_setcontent_BIN; */

        if (!(addr = aug_mapmfile_BIN_(mfile, size))
            || aug_setcontent_BIN(obj, addr, size) < 0) {
            aug_closemfile_BI_(mfile);
            return -1;
        }
    }

    /* EXCEPT: insert_BIN_ -> aug_closemfile_BI_ */

    return aug_closemfile_BI_(mfile);
}

static aug_rsize
seek_(aug_mar* obj, off_t offset, int whence)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    off_t local;

    switch (whence) {
    case AUG_SET:
        local = offset;
        break;
    case AUG_CUR:
        local = (off_t)(impl->offset_ + offset);
        break;
    case AUG_END:
        local = (off_t)(impl->info_.bsize_ + offset);
        break;
    default:
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("invalid whence value [%d]"), (int)whence);
        return -1;
    }

    if (local < 0) {

        /* Assertion? */

        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_ERANGE,
                        AUG_MSG("negative file position [%d]"), (int)local);
        return -1;
    }

    impl->offset_ = local;
    return local;
}

static aug_result
setcontent_BIN_(aug_mar* obj, const void* cdata, unsigned size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    if (!WRITABLE_(impl)) {

        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("invalid archive handle"));
        return -1;
    }

    /* EXCEPT: setcontent_BIN_ -> aug_setcontent_BIN_; */

    return aug_setcontent_BIN_(impl->seq_, &impl->info_, cdata, size);
}

static aug_result
sync_(aug_mar* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    return aug_syncseq_(impl->seq_);
}

static aug_result
truncate_BIN_(aug_mar* obj, unsigned size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    if (!WRITABLE_(impl)) {

        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("invalid archive handle"));
        return -1;
    }

    /* In keeping with the semantics of ftruncate, this function does not
       modify the offset - the equivalent of the file offset. */

    /* EXCEPT: truncate_BIN_ -> aug_truncate_BIN_; */

    return aug_truncate_BIN_(impl->seq_, &impl->info_, size);
}

static aug_rsize
write_BIN_(aug_mar* obj, const void* buf, unsigned len)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    aug_rsize rsize;

    if (!WRITABLE_(impl)) {

        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("invalid archive handle"));
        return -1;
    }

    if (impl->flags_ & AUG_APPEND) {

        assert(AUG_APPEND == (impl->flags_ & AUG_APPEND));

        if ((rsize = aug_seekmar(obj, 0, AUG_END)) < 0)
            return -1;
    }

    /* EXCEPT: write_BIN_ -> aug_write_BIN_; */

    if (0 < (rsize = aug_write_BIN_(impl->seq_, &impl->info_,
                                    impl->offset_, buf, len)))
        impl->offset_ += rsize;

    return rsize;
}

static aug_result
extract_BIN_(aug_mar* obj, const char* path)

{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    aug_mpool* mpool;
    aug_mfile_t mfile;
    void* dst;
    const void* src;
    unsigned size;

    if (!READABLE_(impl)) {

        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("invalid archive handle"));
        return -1;
    }

    size = impl->info_.bsize_;
    if (!(src = aug_getcontent_(impl->seq_, &impl->info_)))
        return -1;

    mpool = aug_seqmpool_(impl->seq_);

    /* EXCEPT: extract_BIN_ -> aug_openmfile_IN_; */

    mfile = aug_openmfile_IN_(mpool, path, AUG_WRONLY | AUG_CREAT | AUG_TRUNC,
                              0666, 0);
    aug_release(mpool);

    if (!mfile)
        return -1;

    if (size) {

        /* EXCEPT: extract_BIN_ -> aug_mapmfile_BIN_; */

        if (!(dst = aug_mapmfile_BIN_(mfile, size)))
            goto fail;

        memcpy(dst, src, size);
    }

    /* EXCEPT: extract_BIN_ -> aug_closemfile_BI_; */

    return aug_closemfile_BI_(mfile);

 fail:
    aug_closemfile_BI_(mfile);
    return -1;
}

static aug_rsize
read_(aug_mar* obj, void* buf, unsigned len)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    aug_rsize rsize;

    if (!READABLE_(impl)) {

        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("invalid archive handle"));
        return -1;
    }

    if (0 < (rsize = aug_read_(impl->seq_, &impl->info_,
                               impl->offset_, buf, len)))
        impl->offset_ += rsize;

    return rsize;
}

static const struct aug_marvtbl marvtbl_ = {
    mar_cast_,
    mar_retain_,
    mar_release_,
    compact_,
    clear_BIN_,
    deln_BIN_,
    delp_BIN_,
    getn_,
    getp_,
    get_,
    putn_BIN_,
    putp_BIN_,
    put_BIN_,
    ntop_,
    pton_,
    getcount_,
    insert_BIN_,
    seek_,
    setcontent_BIN_,
    sync_,
    truncate_BIN_,
    write_BIN_,
    extract_BIN_,
    read_,
};

static void*
blob_cast_(aug_blob* obj, const char* id)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, blob_, obj);
    return cast_(impl, id);
}

static void
blob_retain_(aug_blob* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, blob_, obj);
    retain_(impl);
}

static void
blob_release_(aug_blob* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, blob_, obj);
    release_(impl);
}

static const char*
gettype_(aug_blob* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, blob_, obj);
    const void* value;
    if (aug_getfieldp_(impl->seq_, &impl->info_, "Content-Type", &value) < 0
        || !value)
        value = "application/octet-stream";
    return value;
}

static const void*
getdata_(aug_blob* obj, size_t* size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, blob_, obj);
    if (!READABLE_(impl)) {

        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("invalid archive handle"));
        return NULL;
    }

    *size = impl->info_.bsize_;
    return aug_getcontent_(impl->seq_, &impl->info_);
}

static size_t
getsize_(aug_blob* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, blob_, obj);
    return impl->info_.bsize_;
}

static const struct aug_blobvtbl blobvtbl_ = {
    blob_cast_,
    blob_retain_,
    blob_release_,
    gettype_,
    getdata_,
    getsize_
};

AUGMAR_API aug_mar*
aug_createmar_BIN(aug_mpool* mpool)
{
    aug_seq_t seq;
    struct aug_info_ info;
    struct impl_* impl;

    if (!(seq = aug_createseq_(mpool, sizeof(struct impl_))))
        return NULL;

    /* EXCEPT: aug_createmar_BIN -> init_BIN_; */

    if (init_BIN_(seq, &info) < 0) {
        aug_destroyseq_(seq);
        return NULL;
    }

    impl = (struct impl_*)aug_seqtail_(seq);

    impl->mar_.vtbl_ = &marvtbl_;
    impl->mar_.impl_ = NULL;

    impl->blob_.vtbl_ = &blobvtbl_;
    impl->blob_.impl_ = NULL;

    impl->refs_ = 1;
    impl->mpool_ = mpool;

    impl->seq_ = seq;
    memcpy(&impl->info_, &info, sizeof(info));
    impl->flags_ = AUG_RDWR;
    impl->offset_ = 0;

    aug_retain(mpool);
    return &impl->mar_;
}

AUGMAR_API aug_mar*
aug_openmar_BIN(aug_mpool* mpool, const char* path, int flags, ...)
{
    aug_seq_t seq;
    struct aug_info_ info;
    mode_t mode;
    struct impl_* impl;

    if (flags & AUG_CREAT) {

        va_list args;
        assert(AUG_CREAT == (flags & AUG_CREAT));
        va_start(args, flags);
        mode = va_arg(args, int);
        va_end(args);

    } else
        mode = 0;

    /* EXCEPT: aug_openmar_BIN -> aug_openseq_IN_; */

    if (!(seq = aug_openseq_IN_(mpool, path, flags & ~AUG_TRUNC, mode,
                                sizeof(struct impl_))))
        return NULL;

    /* EXCEPT: aug_openmar_BIN -> init_BIN_; */

    if (init_BIN_(seq, &info) < 0) {
        aug_destroyseq_(seq);
        return NULL;
    }

    impl = (struct impl_*)aug_seqtail_(seq);

    impl->mar_.vtbl_ = &marvtbl_;
    impl->mar_.impl_ = NULL;

    impl->blob_.vtbl_ = &blobvtbl_;
    impl->blob_.impl_ = NULL;

    impl->refs_ = 1;
    impl->mpool_ = mpool;

    impl->seq_ = seq;
    memcpy(&impl->info_, &info, sizeof(info));
    impl->flags_ = flags;
    impl->offset_ = 0;

    aug_retain(mpool);

    /* Fully constructed at this point. */

    if (flags & AUG_TRUNC) {

        assert(AUG_TRUNC == (flags & AUG_TRUNC));

        /* EXCEPT: aug_openmar_BIN -> aug_truncatemar_BIN; */

        if (aug_truncatemar_BIN(&impl->mar_, 0) < 0) {
            aug_release(&impl->mar_);
            return NULL;
        }
    }

    return &impl->mar_;
}
