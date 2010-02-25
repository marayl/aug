/* -*- c++ -*- */
/* Automatically generated by augidl */
/* at Thu Feb 25 09:28:50 +0000 2010 */

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

#include "augtypes.h"

/* Interface declarations. */

AUG_INTERFACE(aug_chandler);
AUG_INTERFACE(aug_chan);

/* Interface definitions. */

struct aug_errinfo;

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
    aug_bool (*auth_)(aug_chandler*, aug_id, const char*, const char*);
    void (*clear_)(aug_chandler*, aug_id);
    void (*error_)(aug_chandler*, aug_chan*, const struct aug_errinfo*);
    aug_bool (*estab_)(aug_chandler*, aug_chan*, aug_id);
    aug_bool (*ready_)(aug_chandler*, aug_chan*, unsigned short);
};

#define aug_authchan(this_, id, subject, issuer) \
    (this_)->vtbl_->auth_(this_, id, subject, issuer)

#define aug_clearchan(this_, id) \
    (this_)->vtbl_->clear_(this_, id)

#define aug_errorchan(this_, chan, errinfo) \
    (this_)->vtbl_->error_(this_, chan, errinfo)

#define aug_estabchan(this_, chan, parent) \
    (this_)->vtbl_->estab_(this_, chan, parent)

#define aug_readychan(this_, chan, events) \
    (this_)->vtbl_->ready_(this_, chan, events)

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
        static aug_bool
        auth_(aug_chandler* this_, aug_id id, const char* subject, const char* issuer) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->authchan_(id, subject, issuer);
        }
        static void
        clear_(aug_chandler* this_, aug_id id) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->clearchan_(id);
        }
        static void
        error_(aug_chandler* this_, aug_chan* chan, const struct aug_errinfo* errinfo) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->errorchan_(chan, *errinfo);
        }
        static aug_bool
        estab_(aug_chandler* this_, aug_chan* chan, aug_id parent) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->estabchan_(chan, parent);
        }
        static aug_bool
        ready_(aug_chandler* this_, aug_chan* chan, unsigned short events) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->readychan_(chan, events);
        }
        static const aug_chandlervtbl*
        vtbl()
        {
            static const aug_chandlervtbl local = {
                cast_,
                retain_,
                release_,
                auth_,
                clear_,
                error_,
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

    typedef aug::obref<aug_chandler> chandlerref;
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
    aug_result (*setwantwr_)(aug_chan*, aug_bool);
    aug_id (*getid_)(aug_chan*);
    char* (*getname_)(aug_chan*, char*, unsigned);
    aug_bool (*isready_)(aug_chan*);
};

#define aug_closechan(this_) \
    (this_)->vtbl_->close_(this_)

#define aug_processchan(this_, handler, fork) \
    (this_)->vtbl_->process_(this_, handler, fork)

#define aug_setchanwantwr(this_, wantwr) \
    (this_)->vtbl_->setwantwr_(this_, wantwr)

#define aug_getchanid(this_) \
    (this_)->vtbl_->getid_(this_)

#define aug_getchanname(this_, dst, size) \
    (this_)->vtbl_->getname_(this_, dst, size)

#define aug_ischanready(this_) \
    (this_)->vtbl_->isready_(this_)

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
        setwantwr_(aug_chan* this_, aug_bool wantwr) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->setchanwantwr_(wantwr);
        }
        static aug_id
        getid_(aug_chan* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->getchanid_();
        }
        static char*
        getname_(aug_chan* this_, char* dst, unsigned size) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->getchanname_(dst, size);
        }
        static aug_bool
        isready_(aug_chan* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->ischanready_();
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
                setwantwr_,
                getid_,
                getname_,
                isready_
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

    typedef aug::obref<aug_chan> chanref;
    typedef aug::smartob<aug_chan> chanptr;
}
#endif /* __cplusplus */

/* C++ definitions. */

#if defined(__cplusplus)
namespace aug {

    inline aug_bool
    authchan(aug::obref<aug_chandler> this_, aug_id id, const char* subject, const char* issuer) AUG_NOTHROW
    {
        return this_.get()->vtbl_->auth_(this_.get(), id, subject, issuer);
    }

    inline void
    clearchan(aug::obref<aug_chandler> this_, aug_id id) AUG_NOTHROW
    {
        this_.get()->vtbl_->clear_(this_.get(), id);
    }

    inline void
    errorchan(aug::obref<aug_chandler> this_, aug::obref<aug_chan> chan, const struct aug_errinfo& errinfo) AUG_NOTHROW
    {
        this_.get()->vtbl_->error_(this_.get(), chan.get(), &errinfo);
    }

    inline aug_bool
    estabchan(aug::obref<aug_chandler> this_, aug::obref<aug_chan> chan, aug_id parent) AUG_NOTHROW
    {
        return this_.get()->vtbl_->estab_(this_.get(), chan.get(), parent);
    }

    inline aug_bool
    readychan(aug::obref<aug_chandler> this_, aug::obref<aug_chan> chan, unsigned short events) AUG_NOTHROW
    {
        return this_.get()->vtbl_->ready_(this_.get(), chan.get(), events);
    }

    template <typename T>
    class chandler_base {
        chandler<T> chandler_;
        int refs_;
    protected:
        ~chandler_base()
        {
            /* Always deleted via derived, so no need to be virtual. */
        }
        chandler_base()
            : refs_(1)
        {
            this->chandler_.reset(static_cast<T*>(this));
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
                delete static_cast<T*>(this);
        }
        static aug::smartob<aug_chandler>
        attach(T* ptr)
        {
            return aug::object_attach<aug_chandler>(ptr->chandler_);
        }
    };

    template <typename T>
    class scoped_chandler_base {
        chandler<T> chandler_;
    protected:
        ~scoped_chandler_base()
        {
        }
        scoped_chandler_base()
        {
            this->chandler_.reset(static_cast<T*>(this));
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
        }
        void
        release_() AUG_NOTHROW
        {
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

    template <typename T>
    class chandler_wrapper {
        chandler<chandler_wrapper<T> > chandler_;
        T impl_;
        int refs_;
        explicit
        chandler_wrapper(const T& impl)
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
        aug_bool
        authchan_(aug_id id, const char* subject, const char* issuer) AUG_NOTHROW
        {
            return this->impl_.authchan_(id, subject, issuer);
        }
        void
        clearchan_(aug_id id) AUG_NOTHROW
        {
            this->impl_.clearchan_(id);
        }
        void
        errorchan_(aug::obref<aug_chan> chan, const struct aug_errinfo& errinfo) AUG_NOTHROW
        {
            this->impl_.errorchan_(chan, errinfo);
        }
        aug_bool
        estabchan_(aug::obref<aug_chan> chan, aug_id parent) AUG_NOTHROW
        {
            return this->impl_.estabchan_(chan, parent);
        }
        aug_bool
        readychan_(aug::obref<aug_chan> chan, unsigned short events) AUG_NOTHROW
        {
            return this->impl_.readychan_(chan, events);
        }
        static aug::smartob<aug_chandler>
        create(const T& impl = T())
        {
            chandler_wrapper* ptr(new chandler_wrapper(impl));
            return aug::object_attach<aug_chandler>(ptr->chandler_);
        }
    };

    template <typename T>
    class scoped_chandler_wrapper {
        chandler<scoped_chandler_wrapper<T> > chandler_;
        T impl_;
    public:
        explicit
        scoped_chandler_wrapper(const T& impl = T())
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
        aug_bool
        authchan_(aug_id id, const char* subject, const char* issuer) AUG_NOTHROW
        {
            return this->impl_.authchan_(id, subject, issuer);
        }
        void
        clearchan_(aug_id id) AUG_NOTHROW
        {
            this->impl_.clearchan_(id);
        }
        void
        errorchan_(aug::obref<aug_chan> chan, const struct aug_errinfo& errinfo) AUG_NOTHROW
        {
            this->impl_.errorchan_(chan, errinfo);
        }
        aug_bool
        estabchan_(aug::obref<aug_chan> chan, aug_id parent) AUG_NOTHROW
        {
            return this->impl_.estabchan_(chan, parent);
        }
        aug_bool
        readychan_(aug::obref<aug_chan> chan, unsigned short events) AUG_NOTHROW
        {
            return this->impl_.readychan_(chan, events);
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
namespace aug {

    inline aug_result
    closechan(aug::obref<aug_chan> this_)
    {
        return verify(this_.get()->vtbl_->close_(this_.get()));
    }

    inline aug::smartob<aug_chan>
    processchan(aug::obref<aug_chan> this_, aug::obref<aug_chandler> handler, aug_bool& fork)
    {
        return aug::object_attach<aug_chan>(verify(this_.get()->vtbl_->process_(this_.get(), handler.get(), &fork)));
    }

    inline aug_result
    setchanwantwr(aug::obref<aug_chan> this_, aug_bool wantwr)
    {
        return verify(this_.get()->vtbl_->setwantwr_(this_.get(), wantwr));
    }

    inline aug_id
    getchanid(aug::obref<aug_chan> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->getid_(this_.get());
    }

    inline char*
    getchanname(aug::obref<aug_chan> this_, char* dst, unsigned size)
    {
        return verify(this_.get()->vtbl_->getname_(this_.get(), dst, size));
    }

    inline aug_bool
    ischanready(aug::obref<aug_chan> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->isready_(this_.get());
    }

    template <typename T>
    class chan_base {
        chan<T> chan_;
        int refs_;
    protected:
        ~chan_base()
        {
            /* Always deleted via derived, so no need to be virtual. */
        }
        chan_base()
            : refs_(1)
        {
            this->chan_.reset(static_cast<T*>(this));
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
                delete static_cast<T*>(this);
        }
        static aug::smartob<aug_chan>
        attach(T* ptr)
        {
            return aug::object_attach<aug_chan>(ptr->chan_);
        }
    };

    template <typename T>
    class scoped_chan_base {
        chan<T> chan_;
    protected:
        ~scoped_chan_base()
        {
        }
        scoped_chan_base()
        {
            this->chan_.reset(static_cast<T*>(this));
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
        }
        void
        release_() AUG_NOTHROW
        {
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

    template <typename T>
    class chan_wrapper {
        chan<chan_wrapper<T> > chan_;
        T impl_;
        int refs_;
        explicit
        chan_wrapper(const T& impl)
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
        setchanwantwr_(aug_bool wantwr) AUG_NOTHROW
        {
            return this->impl_.setchanwantwr_(wantwr);
        }
        aug_id
        getchanid_() AUG_NOTHROW
        {
            return this->impl_.getchanid_();
        }
        char*
        getchanname_(char* dst, unsigned size) AUG_NOTHROW
        {
            return this->impl_.getchanname_(dst, size);
        }
        aug_bool
        ischanready_() AUG_NOTHROW
        {
            return this->impl_.ischanready_();
        }
        static aug::smartob<aug_chan>
        create(const T& impl = T())
        {
            chan_wrapper* ptr(new chan_wrapper(impl));
            return aug::object_attach<aug_chan>(ptr->chan_);
        }
    };

    template <typename T>
    class scoped_chan_wrapper {
        chan<scoped_chan_wrapper<T> > chan_;
        T impl_;
    public:
        explicit
        scoped_chan_wrapper(const T& impl = T())
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
        setchanwantwr_(aug_bool wantwr) AUG_NOTHROW
        {
            return this->impl_.setchanwantwr_(wantwr);
        }
        aug_id
        getchanid_() AUG_NOTHROW
        {
            return this->impl_.getchanid_();
        }
        char*
        getchanname_(char* dst, unsigned size) AUG_NOTHROW
        {
            return this->impl_.getchanname_(dst, size);
        }
        aug_bool
        ischanready_() AUG_NOTHROW
        {
            return this->impl_.ischanready_();
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
