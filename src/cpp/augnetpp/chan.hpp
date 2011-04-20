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
#ifndef AUGNETPP_CHAN_HPP
#define AUGNETPP_CHAN_HPP

#include "augnetpp/config.hpp"

#include "augsyspp/types.hpp" // sdref

#include "augctxpp/exception.hpp"

#include "augnet/chan.h"

#include "augsys/muxer.h"

#include "augext/chan.h"
#include "augext/mpool.h"

namespace aug {

    inline chanptr
    createclient_BI(mpoolref mpool, const char* host, const char* serv,
                    aug_muxer_t muxer, ssl_ctx_st* sslctx = 0)
    {
        return object_attach<aug_chan>
            (verify(aug_createclient_BI(mpool.get(), host, serv, muxer,
                                        sslctx)));
    }

    inline chanptr
    createserver(mpoolref mpool, aug_muxer_t muxer, sdref sd,
                 ssl_ctx_st* sslctx = 0)
    {
        return object_attach<aug_chan>
            (verify(aug_createserver(mpool.get(), muxer, sd.get(), sslctx)));
    }

    inline chanptr
    createplain(mpoolref mpool, aug_id id, aug_muxer_t muxer, sdref sd,
                unsigned short mask)
    {
        return object_attach<aug_chan>
            (verify(aug_createplain(mpool.get(), id, muxer, sd.get(), mask)));
    }
}

#endif // AUGNETPP_CHAN_HPP
