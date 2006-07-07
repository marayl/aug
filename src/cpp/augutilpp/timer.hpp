/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_TIMER_HPP
#define AUGUTILPP_TIMER_HPP

#include "augutilpp/var.hpp"

#include "augsyspp/exception.hpp"

#include "augutil/list.h"
#include "augutil/timer.h"

#include "augsys/errno.h"
#include "augsys/log.h"

namespace aug {

    class AUGUTILPP_API timers {
    public:
        typedef struct aug_timers ctype;
    private:

        friend class timer;

        struct aug_timers timers_;

        timers(const timers&);

        timers&
        operator =(const timers&);

    public:
        ~timers() NOTHROW
        {
            if (-1 == aug_freetimers(&timers_))
                aug_perrinfo("aug_freetimers() failed");
        }

        timers()
        {
            AUG_INIT(&timers_);
        }

        operator struct aug_timers&()
        {
            return timers_;
        }

        operator const struct aug_timers&() const
        {
            return timers_;
        }

        bool
        empty() const
        {
            return AUG_EMPTY(&timers_);
        }
    };

    inline void
    processtimers(struct aug_timers& timers, bool force)
    {
        if (-1 == aug_processtimers(&timers, force ? 1 : 0, 0))
            throwerrinfo("aug_processtimers() failed");
    }

    inline void
    processtimers(struct aug_timers& timers, bool force, struct timeval& next)
    {
        if (-1 == aug_processtimers(&timers, force ? 1 : 0, &next))
            throwerrinfo("aug_processtimers() failed");
    }

    class AUGUTILPP_API expire_base {

        virtual void
        do_expire(int id) = 0;

    public:
        virtual
        ~expire_base() NOTHROW
        {
        }

        void
        expire(int id)
        {
            do_expire(id);
        }
    };

    class AUGUTILPP_API timer {

        struct aug_timers& timers_;
        expire_base& action_;
        int id_;
        bool pending_;

        timer(const timer&);

        timer&
        operator =(const timer&);

        static void
        expire_(const struct aug_var* arg, int id)
        {
            try {
                timer* ptr = static_cast<
                    timer*>(aug_getvarp(arg));
                ptr->pending_ = false;
                ptr->action_.expire(id);
            } AUG_SETERRINFOCATCH;
        }

    public:
        ~timer() NOTHROW
        {
            if (pending() && -1 == aug_canceltimer(&timers_, id_))
                aug_perrinfo("aug_canceltimer() failed");
        }

        timer(timers& timers, expire_base& action)
            : timers_(timers.timers_),
              action_(action),
              id_(-1),
              pending_(false)
        {
        }

        timer(timers& timers, unsigned int ms, expire_base& action)
            : timers_(timers.timers_),
              action_(action),
              id_(-1),
              pending_(false)
        {
            reset(ms);
        }

        void
        cancel()
        {
            if (pending()) {

                int id(id_);
                id_ = -1;
                pending_ = false;

                if (-1 == aug_canceltimer(&timers_, id))
                    throwerrinfo("aug_canceltimer() failed");
            }
        }

        void
        reset(unsigned int ms)
        {
            cancel();
            var v(this);
            int id(aug_settimer(&timers_, ms, expire_, cptr(v)));
            if (-1 == id)
                throwerrinfo("aug_settimer() failed");
            id_ = id;
            pending_ = true;
        }

        int
        get() const
        {
            return id_;
        }

        bool
        pending() const
        {
            return -1 != id_ && pending_;
        }
    };
}

#endif // AUGUTILPP_TIMER_HPP
