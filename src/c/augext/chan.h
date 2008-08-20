/* -*- c++ -*- */
/* Automatically generated by augidl */
/* at Tue Aug 19 18:41:34 GMT Daylight Time 2008 */

#ifndef AUGEXT_CHAN_H
#define AUGEXT_CHAN_H

#include "augabi.h"

#if defined(__cplusplus)

#include "augabipp.hpp"

namespace aug {
    template <typename T>
    struct object_traits;
}

#endif /* __cplusplus */

/* Includes. */

#include "augext/stream.h"
#include "augtypes.h"

/* Interface declarations. */

AUG_INTERFACE(aug_chandler);
AUG_INTERFACE(aug_chan);

/* Interface definitions. */

#define AUG_MAXCHANNAMELEN 255

/**
 * @defgroup aug_chandler aug_chandler
 *
 * @ingroup Object
 *
 * @{
 */

struct aug_chandlervtbl {
    AUG_VTBL(aug_chandler);
    void (*clear_)(aug_chandler*, unsigned);
    aug_bool (*estab_)(aug_chandler*, unsigned, aug_stream*, unsigned);
    aug_bool (*ready_)(aug_chandler*, unsigned, aug_stream*, unsigned short);
};

#define aug_clearchan(this_, id) \
    (this_)->vtbl_->clear_(this_, id)

#define aug_estabchan(this_, id, stream, parent) \
    (this_)->vtbl_->estab_(this_, id, stream, parent)

#define aug_readychan(this_, id, stream, events) \
    (this_)->vtbl_->ready_(this_, id, stream, events)

/** @} */

#if defined(__cplusplus)
namespace aug {
    template <>
    struct object_traits<aug_chandler> {
        typedef aug_chandlervtbl vtbl;
        static const char*
        id() AUG_NOTHROW
        {
            return aug_chandlerid;
        }
    };
}

namespace aug {

    typedef aug::obref<aug_chandler> chandlerref;

    inline void
    clearchan(aug::obref<aug_chandler> this_, unsigned id) AUG_NOTHROW
    {
        this_.get()->vtbl_->clear_(this_.get(), id);
    }

    inline aug_bool
    estabchan(aug::obref<aug_chandler> this_, unsigned id, aug::obref<aug_stream> stream, unsigned parent) AUG_NOTHROW
    {
        return this_.get()->vtbl_->estab_(this_.get(), id, stream.get(), parent);
    }

    inline aug_bool
    readychan(aug::obref<aug_chandler> this_, unsigned id, aug::obref<aug_stream> stream, unsigned short events) AUG_NOTHROW
    {
        return this_.get()->vtbl_->ready_(this_.get(), id, stream.get(), events);
    }

    template <typename T>
    class chandler {

        aug_chandler chandler_;

        chandler(const chandler&);

        chandler&
        operator =(const chandler&);

        static void*
        cast_(aug_chandler* this_, const char* id) AUG_NOTHROW
        {
            return aug::retget(static_cast<T*>(this_->impl_)->cast_(id));
        }
        static void
        retain_(aug_chandler* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->retain_();
        }
        static void
        release_(aug_chandler* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->release_();
        }
        static void
        clear_(aug_chandler* this_, unsigned id) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->clearchan_(id);
        }
        static aug_bool
        estab_(aug_chandler* this_, unsigned id, aug_stream* stream, unsigned parent) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->estabchan_(id, stream, parent);
        }
        static aug_bool
        ready_(aug_chandler* this_, unsigned id, aug_stream* stream, unsigned short events) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->readychan_(id, stream, events);
        }
        static const aug_chandlervtbl*
        vtbl()
        {
            static const aug_chandlervtbl local = {
                cast_,
                retain_,
                release_,
                clear_,
                estab_,
                ready_
            };
            return &local;
        }
    public:
        explicit
        chandler(T* impl = 0)
        {
            this->chandler_.vtbl_ = this->vtbl();
            this->chandler_.impl_ = impl;
        }
        void
        reset(T* impl)
        {
            this->chandler_.impl_ = impl;
        }
        aug_chandler*
        get() AUG_NOTHROW
        {
            return &this->chandler_;
        }
        operator aug::obref<aug_chandler>() AUG_NOTHROW
        {
            return this->get();
        }
    };

    template <typename T>
    class basic_chandler {
        chandler<basic_chandler<T> > chandler_;
        T impl_;
        int refs_;
        explicit
        basic_chandler(const T& impl)
            : impl_(impl),
              refs_(1)
        {
            this->chandler_.reset(this);
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_chandler>(id))
                return aug::object_retain<aug_object>(this->chandler_);
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
        void
        clearchan_(unsigned id) AUG_NOTHROW
        {
            this->impl_.clearchan_(id);
        }
        aug_bool
        estabchan_(unsigned id, aug::obref<aug_stream> stream, unsigned parent) AUG_NOTHROW
        {
            return this->impl_.estabchan_(id, stream, parent);
        }
        aug_bool
        readychan_(unsigned id, aug::obref<aug_stream> stream, unsigned short events) AUG_NOTHROW
        {
            return this->impl_.readychan_(id, stream, events);
        }
        static aug::smartob<aug_chandler>
        create(const T& impl = T())
        {
            basic_chandler* ptr(new basic_chandler(impl));
            return aug::object_attach<aug_chandler>(ptr->chandler_);
        }
    };

    template <typename T>
    class scoped_chandler {
        chandler<scoped_chandler<T> > chandler_;
        T impl_;
    public:
        explicit
        scoped_chandler(const T& impl = T())
            : impl_(impl)
        {
            this->chandler_.reset(this);
        }
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_chandler>(id))
                return aug::object_retain<aug_object>(this->chandler_);
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
        void
        clearchan_(unsigned id) AUG_NOTHROW
        {
            this->impl_.clearchan_(id);
        }
        aug_bool
        estabchan_(unsigned id, aug::obref<aug_stream> stream, unsigned parent) AUG_NOTHROW
        {
            return this->impl_.estabchan_(id, stream, parent);
        }
        aug_bool
        readychan_(unsigned id, aug::obref<aug_stream> stream, unsigned short events) AUG_NOTHROW
        {
            return this->impl_.readychan_(id, stream, events);
        }
        aug_object*
        base() AUG_NOTHROW
        {
            return aug::obref<aug_object>(this->chandler_).get();
        }
        aug_chandler*
        get() AUG_NOTHROW
        {
            return this->chandler_.get();
        }
        operator aug::obref<aug_chandler>() AUG_NOTHROW
        {
            return this->chandler_;
        }
    };

    typedef aug::smartob<aug_chandler> chandlerptr;
}
#endif /* __cplusplus */

/**
 * @defgroup aug_chan aug_chan
 *
 * @ingroup Object
 *
 * @{
 */

/**
 * All channels have non-blocking semantics.
 */

struct aug_chanvtbl {
    AUG_VTBL(aug_chan);
    aug_result (*close_)(aug_chan*);
    aug_chan* (*process_)(aug_chan*, aug_chandler*, aug_bool*);
    aug_result (*setmask_)(aug_chan*, unsigned short);
    int (*getmask_)(aug_chan*);
    unsigned (*getid_)(aug_chan*);
    char* (*getname_)(aug_chan*, char*, unsigned);
};

#define aug_closechan(this_) \
    (this_)->vtbl_->close_(this_)

#define aug_processchan(this_, handler, fork) \
    (this_)->vtbl_->process_(this_, handler, fork)

#define aug_setchanmask(this_, mask) \
    (this_)->vtbl_->setmask_(this_, mask)

#define aug_getchanmask(this_) \
    (this_)->vtbl_->getmask_(this_)

#define aug_getchanid(this_) \
    (this_)->vtbl_->getid_(this_)

#define aug_getchanname(this_, dst, size) \
    (this_)->vtbl_->getname_(this_, dst, size)

/** @} */

#if defined(__cplusplus)
namespace aug {
    template <>
    struct object_traits<aug_chan> {
        typedef aug_chanvtbl vtbl;
        static const char*
        id() AUG_NOTHROW
        {
            return aug_chanid;
        }
    };
}

namespace aug {

    typedef aug::obref<aug_chan> chanref;

    inline aug_result
    closechan(aug::obref<aug_chan> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->close_(this_.get());
    }

    inline aug::smartob<aug_chan>
    processchan(aug::obref<aug_chan> this_, aug::obref<aug_chandler> handler, aug_bool& fork) AUG_NOTHROW
    {
        return aug::object_attach<aug_chan>(this_.get()->vtbl_->process_(this_.get(), handler.get(), &fork));
    }

    inline aug_result
    setchanmask(aug::obref<aug_chan> this_, unsigned short mask) AUG_NOTHROW
    {
        return this_.get()->vtbl_->setmask_(this_.get(), mask);
    }

    inline int
    getchanmask(aug::obref<aug_chan> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->getmask_(this_.get());
    }

    inline unsigned
    getchanid(aug::obref<aug_chan> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->getid_(this_.get());
    }

    inline char*
    getchanname(aug::obref<aug_chan> this_, char* dst, unsigned size) AUG_NOTHROW
    {
        return this_.get()->vtbl_->getname_(this_.get(), dst, size);
    }

    template <typename T>
    class chan {

        aug_chan chan_;

        chan(const chan&);

        chan&
        operator =(const chan&);

        static void*
        cast_(aug_chan* this_, const char* id) AUG_NOTHROW
        {
            return aug::retget(static_cast<T*>(this_->impl_)->cast_(id));
        }
        static void
        retain_(aug_chan* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->retain_();
        }
        static void
        release_(aug_chan* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->release_();
        }
        static aug_result
        close_(aug_chan* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->closechan_();
        }
        static aug_chan*
        process_(aug_chan* this_, aug_chandler* handler, aug_bool* fork) AUG_NOTHROW
        {
            return aug::retget(static_cast<T*>(this_->impl_)->processchan_(handler, *fork));
        }
        static aug_result
        setmask_(aug_chan* this_, unsigned short mask) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->setchanmask_(mask);
        }
        static int
        getmask_(aug_chan* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->getchanmask_();
        }
        static unsigned
        getid_(aug_chan* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->getchanid_();
        }
        static char*
        getname_(aug_chan* this_, char* dst, unsigned size) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->getchanname_(dst, size);
        }
        static const aug_chanvtbl*
        vtbl()
        {
            static const aug_chanvtbl local = {
                cast_,
                retain_,
                release_,
                close_,
                process_,
                setmask_,
                getmask_,
                getid_,
                getname_
            };
            return &local;
        }
    public:
        explicit
        chan(T* impl = 0)
        {
            this->chan_.vtbl_ = this->vtbl();
            this->chan_.impl_ = impl;
        }
        void
        reset(T* impl)
        {
            this->chan_.impl_ = impl;
        }
        aug_chan*
        get() AUG_NOTHROW
        {
            return &this->chan_;
        }
        operator aug::obref<aug_chan>() AUG_NOTHROW
        {
            return this->get();
        }
    };

    template <typename T>
    class basic_chan {
        chan<basic_chan<T> > chan_;
        T impl_;
        int refs_;
        explicit
        basic_chan(const T& impl)
            : impl_(impl),
              refs_(1)
        {
            this->chan_.reset(this);
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_chan>(id))
                return aug::object_retain<aug_object>(this->chan_);
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
        closechan_() AUG_NOTHROW
        {
            return this->impl_.closechan_();
        }
        aug::smartob<aug_chan>
        processchan_(aug::obref<aug_chandler> handler, aug_bool& fork) AUG_NOTHROW
        {
            return this->impl_.processchan_(handler, fork);
        }
        aug_result
        setchanmask_(unsigned short mask) AUG_NOTHROW
        {
            return this->impl_.setchanmask_(mask);
        }
        int
        getchanmask_() AUG_NOTHROW
        {
            return this->impl_.getchanmask_();
        }
        unsigned
        getchanid_() AUG_NOTHROW
        {
            return this->impl_.getchanid_();
        }
        char*
        getchanname_(char* dst, unsigned size) AUG_NOTHROW
        {
            return this->impl_.getchanname_(dst, size);
        }
        static aug::smartob<aug_chan>
        create(const T& impl = T())
        {
            basic_chan* ptr(new basic_chan(impl));
            return aug::object_attach<aug_chan>(ptr->chan_);
        }
    };

    template <typename T>
    class scoped_chan {
        chan<scoped_chan<T> > chan_;
        T impl_;
    public:
        explicit
        scoped_chan(const T& impl = T())
            : impl_(impl)
        {
            this->chan_.reset(this);
        }
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_chan>(id))
                return aug::object_retain<aug_object>(this->chan_);
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
        closechan_() AUG_NOTHROW
        {
            return this->impl_.closechan_();
        }
        aug::smartob<aug_chan>
        processchan_(aug::obref<aug_chandler> handler, aug_bool& fork) AUG_NOTHROW
        {
            return this->impl_.processchan_(handler, fork);
        }
        aug_result
        setchanmask_(unsigned short mask) AUG_NOTHROW
        {
            return this->impl_.setchanmask_(mask);
        }
        int
        getchanmask_() AUG_NOTHROW
        {
            return this->impl_.getchanmask_();
        }
        unsigned
        getchanid_() AUG_NOTHROW
        {
            return this->impl_.getchanid_();
        }
        char*
        getchanname_(char* dst, unsigned size) AUG_NOTHROW
        {
            return this->impl_.getchanname_(dst, size);
        }
        aug_object*
        base() AUG_NOTHROW
        {
            return aug::obref<aug_object>(this->chan_).get();
        }
        aug_chan*
        get() AUG_NOTHROW
        {
            return this->chan_.get();
        }
        operator aug::obref<aug_chan>() AUG_NOTHROW
        {
            return this->chan_;
        }
    };

    typedef aug::smartob<aug_chan> chanptr;
}
#endif /* __cplusplus */

#endif /* AUGEXT_CHAN_H */
