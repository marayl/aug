/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_SERVCONN_HPP
#define AUGAS_SERVCONN_HPP

#include "augas/buffer.hpp"
#include "augas/conn.hpp"

namespace augas {

    class servconn : public rwtimer_base, public conn_base {

        augas_object sock_;
        buffer buffer_;
        rwtimer rwtimer_;
        established conn_;

        // rwtimer_base.

        void
        do_timercb(int id, unsigned& ms, aug_timers& timers);

        void
        do_setrwtimer(unsigned ms, unsigned flags);

        bool
        do_resetrwtimer(unsigned ms, unsigned flags);

        bool
        do_resetrwtimer(unsigned flags);

        bool
        do_cancelrwtimer(unsigned flags);

        // conn_base.

        augas_object&
        do_object();

        const augas_object&
        do_object() const;

        const servptr&
        do_serv() const;

        aug::smartfd
        do_sfd() const;

        bool
        do_accept(const aug_endpoint& ep);

        void
        do_append(aug::mplexer& mplexer, const aug_var& var);

        void
        do_append(aug::mplexer& mplexer, const void* buf, size_t size);

        void
        do_connected(const aug_endpoint& ep);

        bool
        do_process(aug::mplexer& mplexer);

        void
        do_shutdown();

        void
        do_teardown();

        const aug::endpoint&
        do_endpoint() const;

        connphase
        do_phase() const;

    public:
        ~servconn() AUG_NOTHROW;

        servconn(const servptr& serv, void* user, aug::timers& timers,
                 const aug::smartfd& sfd, const aug::endpoint& ep);
    };
}

#endif // AUGAS_SERVCONN_HPP
