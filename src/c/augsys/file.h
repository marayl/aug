/* -*- c++ -*- */
/* Automatically generated by augidl */
/* at Wed May 07 08:18:18 GMT Daylight Time 2008 */

#ifndef AUGSYS_FILE_H
#define AUGSYS_FILE_H

#include "augabi.h"

#if defined(__cplusplus)

#include "augabipp.hpp"

namespace aug {
    template <typename T>
    struct object_traits;
}

#endif /* __cplusplus */
#include "augsys/muxer.h"
#include "augctx/mpool.h"
struct aug_file_;
typedef aug_bool (*aug_filecb_t)(struct aug_file_*);

/**
 * @defgroup aug_file aug_file
 *
 * @ingroup Object
 *
 * @{
 */

AUG_INTERFACE(aug_file);

struct aug_filevtbl {
    AUG_VTBL(aug_file);
    aug_result (*close_)(aug_file*);
    aug_bool (*dispatch_)(aug_file*, aug_filecb_t);
    aug_result (*setnonblock_)(aug_file*, aug_bool);
    aug_result (*seteventmask_)(aug_file*, unsigned short);
    int (*eventmask_)(aug_file*);
    int (*events_)(aug_file*);
};

#define aug_close(this_) \
    (this_)->vtbl_->close_(this_)

#define aug_dispatch(this_, cb) \
    (this_)->vtbl_->dispatch_(this_, cb)

#define aug_setnonblock(this_, on) \
    (this_)->vtbl_->setnonblock_(this_, on)

#define aug_seteventmask(this_, mask) \
    (this_)->vtbl_->seteventmask_(this_, mask)

#define aug_eventmask(this_) \
    (this_)->vtbl_->eventmask_(this_)

#define aug_events(this_) \
    (this_)->vtbl_->events_(this_)

/** @} */

#if defined(__cplusplus)
namespace aug {
    template <>
    struct object_traits<aug_file> {
        typedef aug_filevtbl vtbl;
        static const char*
        id() AUG_NOTHROW
        {
            return aug_fileid;
        }
    };
}

namespace aug {

    typedef aug::obref<aug_file> fileref;

    inline aug_result
    close(aug::obref<aug_file> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->close_(this_.get());
    }

    inline aug_bool
    dispatch(aug::obref<aug_file> this_, aug_filecb_t cb) AUG_NOTHROW
    {
        return this_.get()->vtbl_->dispatch_(this_.get(), cb);
    }

    inline aug_result
    setnonblock(aug::obref<aug_file> this_, aug_bool on) AUG_NOTHROW
    {
        return this_.get()->vtbl_->setnonblock_(this_.get(), on);
    }

    inline aug_result
    seteventmask(aug::obref<aug_file> this_, unsigned short mask) AUG_NOTHROW
    {
        return this_.get()->vtbl_->seteventmask_(this_.get(), mask);
    }

    inline int
    eventmask(aug::obref<aug_file> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->eventmask_(this_.get());
    }

    inline int
    events(aug::obref<aug_file> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->events_(this_.get());
    }

    template <typename T>
    class file {

        aug_file file_;

        file(const file&);

        file&
        operator =(const file&);

        static void*
        cast_(aug_file* this_, const char* id) AUG_NOTHROW
        {
            return aug::retget(static_cast<T*>(this_->impl_)->cast_(id));
        }
        static void
        retain_(aug_file* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->retain_();
        }
        static void
        release_(aug_file* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->release_();
        }
        static aug_result
        close_(aug_file* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->close_();
        }
        static aug_bool
        dispatch_(aug_file* this_, aug_filecb_t cb) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->dispatch_(cb);
        }
        static aug_result
        setnonblock_(aug_file* this_, aug_bool on) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->setnonblock_(on);
        }
        static aug_result
        seteventmask_(aug_file* this_, unsigned short mask) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->seteventmask_(mask);
        }
        static int
        eventmask_(aug_file* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->eventmask_();
        }
        static int
        events_(aug_file* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->events_();
        }
        static const aug_filevtbl*
        vtbl()
        {
            static const aug_filevtbl local = {
                cast_,
                retain_,
                release_,
                close_,
                dispatch_,
                setnonblock_,
                seteventmask_,
                eventmask_,
                events_
            };
            return &local;
        }
    public:
        explicit
        file(T* impl = 0)
        {
            this->file_.vtbl_ = this->vtbl();
            this->file_.impl_ = impl;
        }
        void
        reset(T* impl)
        {
            this->file_.impl_ = impl;
        }
        aug_file*
        get() AUG_NOTHROW
        {
            return &this->file_;
        }
        operator aug::obref<aug_file>() AUG_NOTHROW
        {
            return this->get();
        }
    };

    template <typename T>
    class basic_file {
        file<basic_file<T> > file_;
        T impl_;
        int refs_;
        explicit
        basic_file(const T& impl)
            : impl_(impl),
              refs_(1)
        {
            this->file_.reset(this);
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_file>(id))
                return aug::object_retain<aug_object>(this->file_);
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
        close_() AUG_NOTHROW
        {
            return this->impl_.close_();
        }
        aug_bool
        dispatch_(aug_filecb_t cb) AUG_NOTHROW
        {
            return this->impl_.dispatch_(cb);
        }
        aug_result
        setnonblock_(aug_bool on) AUG_NOTHROW
        {
            return this->impl_.setnonblock_(on);
        }
        aug_result
        seteventmask_(unsigned short mask) AUG_NOTHROW
        {
            return this->impl_.seteventmask_(mask);
        }
        int
        eventmask_() AUG_NOTHROW
        {
            return this->impl_.eventmask_();
        }
        int
        events_() AUG_NOTHROW
        {
            return this->impl_.events_();
        }
        static aug::smartob<aug_file>
        create(const T& impl = T())
        {
            basic_file* ptr(new basic_file(impl));
            return aug::object_attach<aug_file>(ptr->file_);
        }
    };

    template <typename T>
    class scoped_file {
        file<scoped_file<T> > file_;
        T impl_;
    public:
        explicit
        scoped_file(const T& impl = T())
            : impl_(impl)
        {
            this->file_.reset(this);
        }
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_file>(id))
                return aug::object_retain<aug_object>(this->file_);
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
        close_() AUG_NOTHROW
        {
            return this->impl_.close_();
        }
        aug_bool
        dispatch_(aug_filecb_t cb) AUG_NOTHROW
        {
            return this->impl_.dispatch_(cb);
        }
        aug_result
        setnonblock_(aug_bool on) AUG_NOTHROW
        {
            return this->impl_.setnonblock_(on);
        }
        aug_result
        seteventmask_(unsigned short mask) AUG_NOTHROW
        {
            return this->impl_.seteventmask_(mask);
        }
        int
        eventmask_() AUG_NOTHROW
        {
            return this->impl_.eventmask_();
        }
        int
        events_() AUG_NOTHROW
        {
            return this->impl_.events_();
        }
        aug_object*
        base() AUG_NOTHROW
        {
            return aug::obref<aug_object>(this->file_).get();
        }
        aug_file*
        get() AUG_NOTHROW
        {
            return this->file_.get();
        }
        operator aug::obref<aug_file>() AUG_NOTHROW
        {
            return this->file_;
        }
    };
}
#endif /* __cplusplus */
AUGSYS_API aug_file*
aug_attachfd(aug_mpool* mpool, aug_muxer_t muxer, aug_fd fd);

AUGSYS_API aug_file*
aug_attachsd(aug_mpool* mpool, aug_muxer_t muxer, aug_sd sd);

#endif /* AUGSYS_FILE_H */
