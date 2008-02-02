/* -*- c++ -*- */
/* Automatically generated by augidl */
/* at Sat Feb 02 08:11:12 GMT Standard Time 2008 */

#ifndef AUG_ITEROB_H
#define AUG_ITEROB_H

#include "augabi.h"

#if defined(__cplusplus)

#include "augabipp.hpp"

namespace aug {
    template <typename T>
    struct object_traits;
}

#endif /* __cplusplus */
struct aug_var;

/**
 * @defgroup aug_iterob aug_iterob
 *
 * @ingroup Object
 *
 * @{
 */

AUG_INTERFACE(aug_iterob);

struct aug_iterobvtbl {
    AUG_VTBL(aug_iterob);
    int (*end_)(aug_iterob*);
    int (*next_)(aug_iterob*);
    struct aug_var* (*key_)(aug_iterob*, struct aug_var*);
    struct aug_var* (*value_)(aug_iterob*, struct aug_var*);
};

#define aug_iterobend(this_) \
    (this_)->vtbl_->end_(this_)

#define aug_nextiterob(this_) \
    (this_)->vtbl_->next_(this_)

#define aug_iterobkey(this_, var) \
    (this_)->vtbl_->key_(this_, var)

#define aug_iterobvalue(this_, var) \
    (this_)->vtbl_->value_(this_, var)

/** @} */

#if defined(__cplusplus)
namespace aug {
    template <>
    struct object_traits<aug_iterob> {
        typedef aug_iterobvtbl vtbl;
        static const char*
        id() AUG_NOTHROW
        {
            return aug_iterobid;
        }
    };
}

namespace aug {

    typedef aug::obref<aug_iterob> iterobref;

    inline int
    iterobend(aug::obref<aug_iterob> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->end_(this_.get());
    }

    inline int
    nextiterob(aug::obref<aug_iterob> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->next_(this_.get());
    }

    inline struct aug_var*
    iterobkey(aug::obref<aug_iterob> this_, struct aug_var* var) AUG_NOTHROW
    {
        return this_.get()->vtbl_->key_(this_.get(), var);
    }

    inline struct aug_var*
    iterobvalue(aug::obref<aug_iterob> this_, struct aug_var* var) AUG_NOTHROW
    {
        return this_.get()->vtbl_->value_(this_.get(), var);
    }

    template <typename T>
    class iterob {

        aug_iterob iterob_;

        iterob(const iterob&);

        iterob&
        operator =(const iterob&);

        static void*
        cast_(aug_iterob* this_, const char* id) AUG_NOTHROW
        {
            return aug::incget(static_cast<T*>(this_->impl_)->cast_(id));
        }
        static void
        retain_(aug_iterob* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->retain_();
        }
        static void
        release_(aug_iterob* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->release_();
        }
        static int
        end_(aug_iterob* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->iterobend_();
        }
        static int
        next_(aug_iterob* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->nextiterob_();
        }
        static struct aug_var*
        key_(aug_iterob* this_, struct aug_var* var) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->iterobkey_(var);
        }
        static struct aug_var*
        value_(aug_iterob* this_, struct aug_var* var) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->iterobvalue_(var);
        }
        static const aug_iterobvtbl*
        vtbl()
        {
            static const aug_iterobvtbl local = {
                cast_,
                retain_,
                release_,
                end_,
                next_,
                key_,
                value_
            };
            return &local;
        }
    public:
        explicit
        iterob(T* impl = 0)
        {
            this->iterob_.vtbl_ = this->vtbl();
            this->iterob_.impl_ = impl;
        }
        void
        reset(T* impl)
        {
            this->iterob_.impl_ = impl;
        }
        aug_iterob*
        get() AUG_NOTHROW
        {
            return &this->iterob_;
        }
        operator aug::obref<aug_iterob>() AUG_NOTHROW
        {
            return this->get();
        }
    };

    template <typename T>
    class basic_iterob {
        iterob<basic_iterob<T> > iterob_;
        T impl_;
        int refs_;
        explicit
        basic_iterob(const T& impl)
            : impl_(impl),
              refs_(1)
        {
            this->iterob_.reset(this);
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_iterob>(id))
                return aug::object_retain<aug_object>(this->iterob_);
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
        iterobend_() AUG_NOTHROW
        {
            return this->impl_.iterobend_();
        }
        int
        nextiterob_() AUG_NOTHROW
        {
            return this->impl_.nextiterob_();
        }
        struct aug_var*
        iterobkey_(struct aug_var* var) AUG_NOTHROW
        {
            return this->impl_.iterobkey_(var);
        }
        struct aug_var*
        iterobvalue_(struct aug_var* var) AUG_NOTHROW
        {
            return this->impl_.iterobvalue_(var);
        }
        static aug::smartob<aug_iterob>
        create(const T& impl = T())
        {
            basic_iterob* ptr(new basic_iterob(impl));
            return aug::object_attach<aug_iterob>(ptr->iterob_);
        }
    };

    template <typename T>
    class scoped_iterob {
        iterob<scoped_iterob<T> > iterob_;
        T impl_;
    public:
        explicit
        scoped_iterob(const T& impl = T())
            : impl_(impl)
        {
            this->iterob_.reset(this);
        }
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_iterob>(id))
                return aug::object_retain<aug_object>(this->iterob_);
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
        iterobend_() AUG_NOTHROW
        {
            return this->impl_.iterobend_();
        }
        int
        nextiterob_() AUG_NOTHROW
        {
            return this->impl_.nextiterob_();
        }
        struct aug_var*
        iterobkey_(struct aug_var* var) AUG_NOTHROW
        {
            return this->impl_.iterobkey_(var);
        }
        struct aug_var*
        iterobvalue_(struct aug_var* var) AUG_NOTHROW
        {
            return this->impl_.iterobvalue_(var);
        }
        aug_object*
        base() AUG_NOTHROW
        {
            return aug::obref<aug_object>(this->iterob_).get();
        }
        aug_iterob*
        get() AUG_NOTHROW
        {
            return this->iterob_.get();
        }
        operator aug::obref<aug_iterob>() AUG_NOTHROW
        {
            return this->iterob_;
        }
    };
}
#endif /* __cplusplus */

#endif /* AUG_ITEROB_H */
