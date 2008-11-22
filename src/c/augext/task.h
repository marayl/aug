/* -*- c++ -*- */
/* Automatically generated by augidl */
/* at Fri Nov 21 18:56:07 GMT Standard Time 2008 */

#ifndef AUGEXT_TASK_H
#define AUGEXT_TASK_H

#include "augabi.h"

#if defined(__cplusplus)

#include "augabipp.hpp"

namespace aug {
    template <typename T>
    struct object_traits;
}

#endif /* __cplusplus */

/* Includes. */

#include "augtypes.h"

/* Interface declarations. */

AUG_INTERFACE(aug_task);

/* Interface definitions. */

/**
 * @defgroup aug_task aug_task
 *
 * @ingroup Object
 *
 * @{
 */

struct aug_taskvtbl {
    AUG_VTBL(aug_task);
    aug_result (*run_)(aug_task*);
};

#define aug_runtask(this_) \
    (this_)->vtbl_->run_(this_)

/** @} */

#if defined(__cplusplus)
namespace aug {
    template <>
    struct object_traits<aug_task> {
        typedef aug_taskvtbl vtbl;
        static const char*
        id() AUG_NOTHROW
        {
            return aug_taskid;
        }
    };
}

namespace aug {

    template <typename T>
    class task {

        aug_task task_;

        task(const task&);

        task&
        operator =(const task&);

        static void*
        cast_(aug_task* this_, const char* id) AUG_NOTHROW
        {
            return aug::retget(static_cast<T*>(this_->impl_)->cast_(id));
        }
        static void
        retain_(aug_task* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->retain_();
        }
        static void
        release_(aug_task* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->release_();
        }
        static aug_result
        run_(aug_task* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->runtask_();
        }
        static const aug_taskvtbl*
        vtbl()
        {
            static const aug_taskvtbl local = {
                cast_,
                retain_,
                release_,
                run_
            };
            return &local;
        }
    public:
        explicit
        task(T* impl = 0)
        {
            this->task_.vtbl_ = this->vtbl();
            this->task_.impl_ = impl;
        }
        void
        reset(T* impl)
        {
            this->task_.impl_ = impl;
        }
        aug_task*
        get() AUG_NOTHROW
        {
            return &this->task_;
        }
        operator aug::obref<aug_task>() AUG_NOTHROW
        {
            return this->get();
        }
    };

    typedef aug::obref<aug_task> taskref;
    typedef aug::smartob<aug_task> taskptr;
}
#endif /* __cplusplus */

/* C++ definitions. */

#if defined(__cplusplus)
namespace aug {

    inline void
    runtask(aug::obref<aug_task> this_)
    {
        verify(this_.get()->vtbl_->run_(this_.get()), this_);
    }

    template <typename T>
    class task_base {
        task<T> task_;
        int refs_;
    protected:
        ~task_base()
        {
            /* Always deleted via derived, so no need to be virtual. */
        }
        task_base()
            : refs_(1)
        {
            this->task_.reset(static_cast<T*>(this));
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_task>(id))
                return aug::object_retain<aug_object>(this->task_);
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
        static aug::smartob<aug_task>
        attach(T* ptr)
        {
            return aug::object_attach<aug_task>(ptr->task_);
        }
    };

    template <typename T>
    class scoped_task_base {
        task<T> task_;
    protected:
        ~scoped_task_base()
        {
        }
        scoped_task_base()
        {
            this->task_.reset(static_cast<T*>(this));
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_task>(id))
                return aug::object_retain<aug_object>(this->task_);
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
            return aug::obref<aug_object>(this->task_).get();
        }
        aug_task*
        get() AUG_NOTHROW
        {
            return this->task_.get();
        }
        operator aug::obref<aug_task>() AUG_NOTHROW
        {
            return this->task_;
        }
    };

    template <typename T>
    class task_wrapper {
        task<task_wrapper<T> > task_;
        T impl_;
        int refs_;
        explicit
        task_wrapper(const T& impl)
            : impl_(impl),
              refs_(1)
        {
            this->task_.reset(this);
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_task>(id))
                return aug::object_retain<aug_object>(this->task_);
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
        aug_result
        runtask_() AUG_NOTHROW
        {
            return this->impl_.runtask_();
        }
        static aug::smartob<aug_task>
        create(const T& impl = T())
        {
            task_wrapper* ptr(new task_wrapper(impl));
            return aug::object_attach<aug_task>(ptr->task_);
        }
    };

    template <typename T>
    class scoped_task_wrapper {
        task<scoped_task_wrapper<T> > task_;
        T impl_;
    public:
        explicit
        scoped_task_wrapper(const T& impl = T())
            : impl_(impl)
        {
            this->task_.reset(this);
        }
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_task>(id))
                return aug::object_retain<aug_object>(this->task_);
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
        aug_result
        runtask_() AUG_NOTHROW
        {
            return this->impl_.runtask_();
        }
        aug_object*
        base() AUG_NOTHROW
        {
            return aug::obref<aug_object>(this->task_).get();
        }
        aug_task*
        get() AUG_NOTHROW
        {
            return this->task_.get();
        }
        operator aug::obref<aug_task>() AUG_NOTHROW
        {
            return this->task_;
        }
    };

    typedef aug::smartob<aug_task> taskptr;
}
#endif /* __cplusplus */

#endif /* AUGEXT_TASK_H */
