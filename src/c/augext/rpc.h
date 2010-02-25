/* -*- c++ -*- */
/* Automatically generated by augidl */
/* at Thu Feb 25 09:28:51 +0000 2010 */

#ifndef AUGEXT_RPC_H
#define AUGEXT_RPC_H

#include "augabi.h"

#if defined(__cplusplus)

#include "augabipp.hpp"

namespace aug {
    template <typename T>
    struct object_traits;
}

#endif /* __cplusplus */

/* Includes. */

#include "augext/array.h"

/* Interface declarations. */

AUG_INTERFACE(aug_rpc);

/* Interface definitions. */

/**
 * @defgroup aug_rpc aug_rpc
 *
 * @ingroup Object
 *
 * @{
 */

struct aug_rpcvtbl {
    AUG_VTBL(aug_rpc);
    const char* (*getmethod_)(aug_rpc*);
    aug_array* (*getargs_)(aug_rpc*);
};

#define aug_getrpcmethod(this_) \
    (this_)->vtbl_->getmethod_(this_)

#define aug_getrpcargs(this_) \
    (this_)->vtbl_->getargs_(this_)

/** @} */

#if defined(__cplusplus)
namespace aug {
    template <>
    struct object_traits<aug_rpc> {
        typedef aug_rpcvtbl vtbl;
        static const char*
        id() AUG_NOTHROW
        {
            return aug_rpcid;
        }
    };
}

namespace aug {

    template <typename T>
    class rpc {

        aug_rpc rpc_;

        rpc(const rpc&);

        rpc&
        operator =(const rpc&);

        static void*
        cast_(aug_rpc* this_, const char* id) AUG_NOTHROW
        {
            return aug::retget(static_cast<T*>(this_->impl_)->cast_(id));
        }
        static void
        retain_(aug_rpc* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->retain_();
        }
        static void
        release_(aug_rpc* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->release_();
        }
        static const char*
        getmethod_(aug_rpc* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->getrpcmethod_();
        }
        static aug_array*
        getargs_(aug_rpc* this_) AUG_NOTHROW
        {
            return aug::retget(static_cast<T*>(this_->impl_)->getrpcargs_());
        }
        static const aug_rpcvtbl*
        vtbl()
        {
            static const aug_rpcvtbl local = {
                cast_,
                retain_,
                release_,
                getmethod_,
                getargs_
            };
            return &local;
        }
    public:
        explicit
        rpc(T* impl = 0)
        {
            this->rpc_.vtbl_ = this->vtbl();
            this->rpc_.impl_ = impl;
        }
        void
        reset(T* impl)
        {
            this->rpc_.impl_ = impl;
        }
        aug_rpc*
        get() AUG_NOTHROW
        {
            return &this->rpc_;
        }
        operator aug::obref<aug_rpc>() AUG_NOTHROW
        {
            return this->get();
        }
    };

    typedef aug::obref<aug_rpc> rpcref;
    typedef aug::smartob<aug_rpc> rpcptr;
}
#endif /* __cplusplus */

/* C++ definitions. */

#if defined(__cplusplus)
namespace aug {

    inline const char*
    getrpcmethod(aug::obref<aug_rpc> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->getmethod_(this_.get());
    }

    inline aug::smartob<aug_array>
    getrpcargs(aug::obref<aug_rpc> this_) AUG_NOTHROW
    {
        return aug::object_attach<aug_array>(this_.get()->vtbl_->getargs_(this_.get()));
    }

    template <typename T>
    class rpc_base {
        rpc<T> rpc_;
        int refs_;
    protected:
        ~rpc_base()
        {
            /* Always deleted via derived, so no need to be virtual. */
        }
        rpc_base()
            : refs_(1)
        {
            this->rpc_.reset(static_cast<T*>(this));
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_rpc>(id))
                return aug::object_retain<aug_object>(this->rpc_);
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
                delete static_cast<T*>(this);
        }
        static aug::smartob<aug_rpc>
        attach(T* ptr)
        {
            return aug::object_attach<aug_rpc>(ptr->rpc_);
        }
    };

    template <typename T>
    class scoped_rpc_base {
        rpc<T> rpc_;
    protected:
        ~scoped_rpc_base()
        {
        }
        scoped_rpc_base()
        {
            this->rpc_.reset(static_cast<T*>(this));
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_rpc>(id))
                return aug::object_retain<aug_object>(this->rpc_);
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
        aug_object*
        base() AUG_NOTHROW
        {
            return aug::obref<aug_object>(this->rpc_).get();
        }
        aug_rpc*
        get() AUG_NOTHROW
        {
            return this->rpc_.get();
        }
        operator aug::obref<aug_rpc>() AUG_NOTHROW
        {
            return this->rpc_;
        }
    };

    template <typename T>
    class rpc_wrapper {
        rpc<rpc_wrapper<T> > rpc_;
        T impl_;
        int refs_;
        explicit
        rpc_wrapper(const T& impl)
            : impl_(impl),
              refs_(1)
        {
            this->rpc_.reset(this);
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_rpc>(id))
                return aug::object_retain<aug_object>(this->rpc_);
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
        const char*
        getrpcmethod_() AUG_NOTHROW
        {
            return this->impl_.getrpcmethod_();
        }
        aug::smartob<aug_array>
        getrpcargs_() AUG_NOTHROW
        {
            return this->impl_.getrpcargs_();
        }
        static aug::smartob<aug_rpc>
        create(const T& impl = T())
        {
            rpc_wrapper* ptr(new rpc_wrapper(impl));
            return aug::object_attach<aug_rpc>(ptr->rpc_);
        }
    };

    template <typename T>
    class scoped_rpc_wrapper {
        rpc<scoped_rpc_wrapper<T> > rpc_;
        T impl_;
    public:
        explicit
        scoped_rpc_wrapper(const T& impl = T())
            : impl_(impl)
        {
            this->rpc_.reset(this);
        }
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_rpc>(id))
                return aug::object_retain<aug_object>(this->rpc_);
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
        const char*
        getrpcmethod_() AUG_NOTHROW
        {
            return this->impl_.getrpcmethod_();
        }
        aug::smartob<aug_array>
        getrpcargs_() AUG_NOTHROW
        {
            return this->impl_.getrpcargs_();
        }
        aug_object*
        base() AUG_NOTHROW
        {
            return aug::obref<aug_object>(this->rpc_).get();
        }
        aug_rpc*
        get() AUG_NOTHROW
        {
            return this->rpc_.get();
        }
        operator aug::obref<aug_rpc>() AUG_NOTHROW
        {
            return this->rpc_;
        }
    };

    typedef aug::smartob<aug_rpc> rpcptr;
}
#endif /* __cplusplus */

#endif /* AUGEXT_RPC_H */
