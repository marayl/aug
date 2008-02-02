/* -*- c++ -*- */
/* Automatically generated by augidl */
/* at Sat Feb 02 08:11:11 GMT Standard Time 2008 */

#ifndef AUG_ERRINFOB_H
#define AUG_ERRINFOB_H

#include "augabi.h"

#if defined(__cplusplus)

#include "augabipp.hpp"

namespace aug {
    template <typename T>
    struct object_traits;
}

#endif /* __cplusplus */

/**
 * @defgroup aug_errinfob aug_errinfob
 *
 * @ingroup Object
 *
 * @{
 */

AUG_INTERFACE(aug_errinfob);

/**
 * Detailed error information.
 */

struct aug_errinfobvtbl {
    AUG_VTBL(aug_errinfob);
    int (*num_)(aug_errinfob*);
    char* (*desc_)(aug_errinfob*, char*, size_t);
};

/**
 * Last error number.
 * @param this_ The object.
 * @return Error number.
 */

#define aug_errinfobnum(this_) \
    (this_)->vtbl_->num_(this_)

/**
 * Last error description.
 * @param this_ The object.
 * @param dst Output buffer.
 * @param size Size of @a dst.
 * @return @a dst output buffer.
 */

#define aug_errinfobdesc(this_, dst, size) \
    (this_)->vtbl_->desc_(this_, dst, size)

/** @} */

#if defined(__cplusplus)
namespace aug {
    template <>
    struct object_traits<aug_errinfob> {
        typedef aug_errinfobvtbl vtbl;
        static const char*
        id() AUG_NOTHROW
        {
            return aug_errinfobid;
        }
    };
}

namespace aug {

    typedef aug::obref<aug_errinfob> errinfobref;

    inline int
    errinfobnum(aug::obref<aug_errinfob> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->num_(this_.get());
    }

    inline char*
    errinfobdesc(aug::obref<aug_errinfob> this_, char* dst, size_t size) AUG_NOTHROW
    {
        return this_.get()->vtbl_->desc_(this_.get(), dst, size);
    }

    template <typename T>
    class errinfob {

        aug_errinfob errinfob_;

        errinfob(const errinfob&);

        errinfob&
        operator =(const errinfob&);

        static void*
        cast_(aug_errinfob* this_, const char* id) AUG_NOTHROW
        {
            return aug::incget(static_cast<T*>(this_->impl_)->cast_(id));
        }
        static void
        retain_(aug_errinfob* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->retain_();
        }
        static void
        release_(aug_errinfob* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->release_();
        }
        static int
        num_(aug_errinfob* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->errinfobnum_();
        }
        static char*
        desc_(aug_errinfob* this_, char* dst, size_t size) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->errinfobdesc_(dst, size);
        }
        static const aug_errinfobvtbl*
        vtbl()
        {
            static const aug_errinfobvtbl local = {
                cast_,
                retain_,
                release_,
                num_,
                desc_
            };
            return &local;
        }
    public:
        explicit
        errinfob(T* impl = 0)
        {
            this->errinfob_.vtbl_ = this->vtbl();
            this->errinfob_.impl_ = impl;
        }
        void
        reset(T* impl)
        {
            this->errinfob_.impl_ = impl;
        }
        aug_errinfob*
        get() AUG_NOTHROW
        {
            return &this->errinfob_;
        }
        operator aug::obref<aug_errinfob>() AUG_NOTHROW
        {
            return this->get();
        }
    };

    template <typename T>
    class basic_errinfob {
        errinfob<basic_errinfob<T> > errinfob_;
        T impl_;
        int refs_;
        explicit
        basic_errinfob(const T& impl)
            : impl_(impl),
              refs_(1)
        {
            this->errinfob_.reset(this);
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_errinfob>(id))
                return aug::object_retain<aug_object>(this->errinfob_);
            return null;
        }
        void
        retain_() AUG_NOTHROW
        {
            assert(0 < this->refs_);
            ++this->refs_;
        }
        void
        release_() AUG_NOTHROW
        {
            assert(0 < this->refs_);
            if (0 == --this->refs_)
                delete this;
        }
        int
        errinfobnum_() AUG_NOTHROW
        {
            return this->impl_.errinfobnum_();
        }
        char*
        errinfobdesc_(char* dst, size_t size) AUG_NOTHROW
        {
            return this->impl_.errinfobdesc_(dst, size);
        }
        static aug::smartob<aug_errinfob>
        create(const T& impl = T())
        {
            basic_errinfob* ptr(new basic_errinfob(impl));
            return aug::object_attach<aug_errinfob>(ptr->errinfob_);
        }
    };

    template <typename T>
    class scoped_errinfob {
        errinfob<scoped_errinfob<T> > errinfob_;
        T impl_;
    public:
        explicit
        scoped_errinfob(const T& impl = T())
            : impl_(impl)
        {
            this->errinfob_.reset(this);
        }
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_errinfob>(id))
                return aug::object_retain<aug_object>(this->errinfob_);
            return null;
        }
        void
        retain_() AUG_NOTHROW
        {
        }
        void
        release_() AUG_NOTHROW
        {
        }
        int
        errinfobnum_() AUG_NOTHROW
        {
            return this->impl_.errinfobnum_();
        }
        char*
        errinfobdesc_(char* dst, size_t size) AUG_NOTHROW
        {
            return this->impl_.errinfobdesc_(dst, size);
        }
        aug_object*
        base() AUG_NOTHROW
        {
            return aug::obref<aug_object>(this->errinfob_).get();
        }
        aug_errinfob*
        get() AUG_NOTHROW
        {
            return this->errinfob_.get();
        }
        operator aug::obref<aug_errinfob>() AUG_NOTHROW
        {
            return this->errinfob_;
        }
    };
}
#endif /* __cplusplus */

#endif /* AUG_ERRINFOB_H */
