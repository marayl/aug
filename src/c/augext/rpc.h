/* -*- c++ -*- */
/* Automatically generated by augidl */
/* at Fri Sep 26 07:19:49 GMT Daylight Time 2008 */

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

#include "augext/seq.h"

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
    aug_seq* (*getargs_)(aug_rpc*);
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

    typedef aug::obref<aug_rpc> rpcref;

    inline const char*
    getrpcmethod(aug::obref<aug_rpc> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->getmethod_(this_.get());
    }

    inline aug::smartob<aug_seq>
    getrpcargs(aug::obref<aug_rpc> this_) AUG_NOTHROW
    {
        return aug::object_attach<aug_seq>(this_.get()->vtbl_->getargs_(this_.get()));
    }

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
        static aug_seq*
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

    template <typename T>
    class basic_rpc {
        rpc<basic_rpc<T> > rpc_;
        T impl_;
        int refs_;
        explicit
        basic_rpc(const T& impl)
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
        aug::smartob<aug_seq>
        getrpcargs_() AUG_NOTHROW
        {
            return this->impl_.getrpcargs_();
        }
        static aug::smartob<aug_rpc>
        create(const T& impl = T())
        {
            basic_rpc* ptr(new basic_rpc(impl));
            return aug::object_attach<aug_rpc>(ptr->rpc_);
        }
    };

    template <typename T>
    class scoped_rpc {
        rpc<scoped_rpc<T> > rpc_;
        T impl_;
    public:
        explicit
        scoped_rpc(const T& impl = T())
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
        aug::smartob<aug_seq>
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
