/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGASPP_SOCKS_HPP
#define AUGASPP_SOCKS_HPP

#include "augaspp/sock.hpp"

#include <map>

struct timeval;

namespace aug {

    class chans;

    class socks {

        typedef std::map<mod_id, sockptr, std::greater<mod_id> > container;

        container socks_;

        socks(const socks& rhs);

        socks&
        operator =(const socks& rhs);

    public:
        ~socks() AUG_NOTHROW;

        socks()
        {
        }

        void
        clear();

        void
        erase(mod_id id);

        void
        erase(const sock_base& sock);

        void
        insert(const sockptr& sock);

        void
        teardown(chans& chans, const timeval& tv);

        bool
        empty() const;

        bool
        exists(mod_id id) const;

        // Throws if id does not exist.

        sockptr
        get(mod_id id) const;
    };

    class scoped_insert {

        socks& socks_;
        sockptr sock_;

        scoped_insert(const scoped_insert& rhs);

        scoped_insert&
        operator =(const scoped_insert& rhs);

    public:
        ~scoped_insert() AUG_NOTHROW
        {
            if (null != sock_)
                socks_.erase(*sock_);
        }
        scoped_insert(socks& socks, const sockptr& sock)
            : socks_(socks),
              sock_(sock)
        {
            socks.insert(sock);
        }
        void
        commit()
        {
            sock_ = null;
        }
    };
}

#endif // AUGASPP_SOCKS_HPP
