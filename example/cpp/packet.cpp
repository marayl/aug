/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include "augservpp.hpp"
#include "augnetpp.hpp"
#include "augutilpp.hpp"
#include "augsyspp.hpp"
#include "augctxpp.hpp"

#include <csignal>
#include <iostream>

using namespace aug;
using namespace std;

// packet 239.1.1.1 1000 3

namespace {

    struct window_exception : std::exception { };

    struct duplicate_exception : window_exception {
        const char*
        what() const throw()
        {
            return "aug::duplicate_exception";
        }
    };

    struct maxwindow_exception : window_exception {
        const char*
        what() const throw()
        {
            return "aug::maxwindow_exception";
        }
    };

    struct timeout_exception : window_exception {
        const char*
        what() const throw()
        {
            return "aug::timeout_exception";
        }
    };

    typedef unsigned long seqno_t;

    class window {
        struct message {
            char buf_[AUG_PACKETSIZE];
            aug_timeval tv_;
        };
        const unsigned size_;
        message* const ring_;
        seqno_t begin_, end_;
        bool seed_;
        static void
        clear(message& m)
        {
            m.tv_.tv_sec = 0;
        }
        static bool
        empty(const message& m)
        {
            return 0 == m.tv_.tv_sec;
        }
    public:
        ~window() AUG_NOTHROW
        {
            delete[] ring_;
        }
        explicit
        window(unsigned size)
            : size_(size),
              ring_(new message[size]),
              begin_(1),
              end_(1),
              seed_(true)
        {
        }
        char*
        insert(seqno_t seq, const aug_timeval& tv)
        {
            if (seed_) {
                begin_ = seq;
                end_ = seq + 1;
                seed_ = false;
            } else {
                const long diff(seq - begin_);
                if (diff < 0)
                    throw duplicate_exception();
                if (size_ <= static_cast<seqno_t>(diff))
                    throw maxwindow_exception();
                if (end_ <= seq) {
                    for (seqno_t i(end_); i < seq; ++i)
                        clear(ring_[i]);
                    end_ = seq + 1;
                }
            }
            message& m(ring_[seq % size_]);
            m.tv_.tv_sec = tv.tv_sec;
            m.tv_.tv_usec = tv.tv_usec;
            return m.buf_;
        }
        bool
        next(aug_packet& pkt, aug_timeval& tv)
        {
            if (empty())
                return false;
            message& m(ring_[++begin_ % size_]);
            aug_decodepacket(&pkt, m.buf_);
            tv.tv_sec = m.tv_.tv_sec;
            tv.tv_usec = m.tv_.tv_usec;
            clear(m);
            return true;
        }
        void
        skip()
        {
            if (begin_ != end_ && !seed_)
                ++begin_;
        }
        bool
        empty() const
        {
            return begin_ == end_ || empty(ring_[begin_ % size_]);
        }
    };

    class expirywindow {
        clockptr clock_;
        window window_;
        aug_timeval timeout_;
        aug_timeval expiry_;
    public:
        explicit
        expirywindow(clockref clock, unsigned size, unsigned timeout)
            : clock_(object_retain(clock)),
              window_(size)
        {
            // 20% tolerance.
            mstotv(timeout + timeout / 5, timeout_);

            gettimeofday(clock, expiry_);
            tvadd(expiry_, timeout_);
        }
        void
        insert(const char* src)
        {
            unsigned seqno;
            verify(aug_decodeseqno(&seqno, src));

            aug_timeval tv;
            gettimeofday(clock_, tv);
            char* dst(window_.insert(seqno, tv));
            memcpy(dst, src, AUG_PACKETSIZE);
        }
        bool
        next(aug_packet& pkt)
        {
            aug_timeval tv;
            if (window_.empty()) {
                gettimeofday(clock_, tv);
                if (timercmp(&expiry_, &tv, <=)) {
                    // Expiry time has passed.
                    window_.skip();
                    // Advance expiry time.
                    tvadd(expiry_, timeout_);
                    throw timeout_exception();
                }
                return false;
            }
            window_.next(pkt, tv);
            tvadd(tv, timeout_);
            if (timercmp(&expiry_, &tv, <)) {
                // Advance expiry time.
                expiry_.tv_sec = tv.tv_sec;
                expiry_.tv_usec = tv.tv_usec;
            }
            return true;
        }
        unsigned
        expiry() const
        {
            // Milliseconds to expiry.
            aug_timeval tv;
            gettimeofday(clock_, tv);
            return AUG_MIN(tvtoms(tvsub(tv, expiry_)), 0);
        }
    };

    volatile bool stop_ = false;

    void
    sigcatch(int sig)
    {
        stop_ = true;
    }

    class packet : aug_packet, public mpool_ops {
        size_t
        sendhead(sdref ref, unsigned type, const endpoint& ep)
        {
            type_ = type;
            ++seqno_;

            char buf[AUG_PACKETSIZE];
            aug_encodepacket(buf, static_cast<aug_packet*>(this));
            return aug::sendto(ref, buf, sizeof(buf), 0, ep);
        }
    public:
        explicit
        packet(const char* addr)
        {
            verno_ = 1;
            type_ = 0;
            seqno_ = 0;
            aug_strlcpy(addr_, addr, sizeof(addr_));
        }
        size_t
        sendopen(sdref ref, const endpoint& ep)
        {
            return sendhead(ref, AUG_PKTOPEN, ep);
        }
        size_t
        sendclose(sdref ref, const endpoint& ep)
        {
            return sendhead(ref, AUG_PKTCLOSE, ep);
        }
        size_t
        sendhbeat(sdref ref, const endpoint& ep)
        {
            return sendhead(ref, AUG_PKTHBEAT, ep);
        }
        size_t
        sendlost(sdref ref, const endpoint& ep)
        {
            return sendhead(ref, AUG_PKTLOST, ep);
        }
        size_t
        sendevent(sdref ref, const char* method, const char* uri,
                  const endpoint& ep)
        {
            aug_strlcpy(content_.event_.method_, method,
                        sizeof(content_.event_.method_));
            aug_strlcpy(content_.event_.uri_, uri,
                        sizeof(content_.event_.uri_));
            return sendhead(ref, AUG_PKTEVENT, ep);
        }
    };

    class session : public mpool_ops {
        sdref ref_;
        const endpoint& ep_;
        expirywindow window_;
        timer rdwait_;
        timer wrwait_;

    public:
        ~session() AUG_NOTHROW
        {
        }
        session(sdref ref, const endpoint& ep, timers& ts)
            : ref_(ref),
              ep_(ep),
              window_(getclock(aug_tlx), 8, 2000),
              rdwait_(ts, null),
              wrwait_(ts, null)
        {
            rdwait_.set(window_.expiry(), *this);
            wrwait_.set(2000, *this);
        }
        void
        recv(sdref ref)
        {
            char buf[AUG_PACKETSIZE];
            endpoint from(null);
            recvfrom(ref, buf, sizeof(buf), 0, from);
            window_.insert(buf);
        }
        void
        timercb(aug_id id, unsigned& ms)
        {
            if (idref(id) == wrwait_.id()) {

                aug_ctxinfo(aug_tlx, "hbint timeout");
                packet pkt("test");
                pkt.sendhbeat(ref_, ep_);
            }
        }
        void
        process()
        {
            try {
                aug_packet pkt;
                while (window_.next(pkt)) {
                    aug_ctxinfo(aug_tlx, "recv: seqno=[%u], type=[%u]",
                                pkt.seqno_, pkt.type_);
                }
            } catch (const window_exception& e) {
                aug_ctxerror(aug_tlx, "error: %s", e.what());
            }
            rdwait_.set(window_.expiry(), *this);
        }
    };

    void
    run(sdref ref, const endpoint& ep)
    {
        endpoint addr(null);
        getsockname(ref, addr);
        aug_ctxinfo(aug_tlx, "bound to: [%s]", endpointntop(addr).c_str());

        muxer mux(getmpool(aug_tlx));
        timers ts(getmpool(aug_tlx), getclock(aug_tlx));
        session sess(ref, ep, ts);
        setmdeventmask(mux, ref, AUG_MDEVENTRDEX);

        aug_timeval tv;
        unsigned ready(!0);
        while (!stop_) {

            processexpired(ts, 0 == ready, tv);
            aug_ctxinfo(aug_tlx, "timeout in: tv_sec=%d, tv_usec=%d",
                        (int)tv.tv_sec, (int)tv.tv_usec);

            try {
                ready = waitmdevents(mux, tv);
            } catch (const intr_exception&) {
                ready = !0; // Not timeout.
                continue;
            }

            aug_ctxinfo(aug_tlx, "waitmdevents: %u", ready);

            if (ready)
                sess.recv(ref);

            sess.process();
        }
    }
}

int
main(int argc, char* argv[])
{
    signal(SIGINT, sigcatch);

    try {

        autotlx();
        setdaemonlog(aug_tlx);

        aug_timeval tv;
        aug::gettimeofday(getclock(aug_tlx), tv);

        if (argc < 3) {
            aug_ctxerror(aug_tlx,
                         "usage: heartbeat <mcast> <serv> [ifname]");
            return 1;
        }

        inetaddr in(argv[1]);
        autosd sfd(aug::socket(family(in), SOCK_DGRAM));
        setreuseaddr(sfd, true);

        // Set outgoing multicast interface.

        if (4 == argc)
            setmcastif(sfd, argv[3]);

        // Don't receive packets from self.

        endpoint ep(inetany(family(in)), htons(atoi(argv[2])));
        aug::bind(sfd, ep);

        joinmcast(sfd, in, 4 == argc ? argv[3] : 0);
        setinetaddr(ep, in);
        run(sfd, ep);
        return 0;

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1;
}
