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
#define AUGSYS_BUILD
#include "augsys/socket.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/errinfo.h"
#include "augctx/errno.h"

#if !defined(_WIN32)
# define SETAFNOSUPPORT_() \
    aug_setposixerror(aug_tlx, __FILE__, __LINE__, EAFNOSUPPORT)
# include "augsys/posix/socket.c"
#else /* _WIN32 */
# define SETAFNOSUPPORT_() \
    aug_setwin32error(aug_tlx, __FILE__, __LINE__, WSAEAFNOSUPPORT)
# include "augsys/win32/socket.c"
# define snprintf _snprintf
#endif /* _WIN32 */

#include <assert.h>
#include <stdio.h>

struct ipv4_ {
    short family_;
    union {
        unsigned char data_[4];
        struct in_addr ipv4_;
#if HAVE_IPV6
        struct sockaddr_in6 ipv6_;
#endif /* HAVE_IPV6 */
        char pad_[16];
    } un_;
};

static const struct ipv4_ ipv4any_ = {
    AF_INET,
    { { 0, 0, 0, 0 } }
};

static const struct ipv4_ ipv4loopback_ = {
    AF_INET,
    { { 127, 0, 0, 1 } }
};

#if HAVE_IPV6
struct ipv6_ {
    short family_;
    union {
        unsigned char data_[16];
        struct in_addr ipv4_;
        struct in6_addr ipv6_;
        char pad_[16];
    } un_;
};

static const struct ipv6_ ipv6any_ = {
    AF_INET6,
    { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }
};

static const struct ipv6_ ipv6loopback_ = {
    AF_INET6,
    { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } }
};
#endif /* HAVE_IPV6 */

AUGSYS_API char*
aug_endpointntop(const struct aug_endpoint* src, char* dst, socklen_t len)
{
    struct aug_inetaddr addr;
    char host[AUG_MAXHOSTNAMELEN + 1];
    const char* fmt;
    int ret;

    assert(src && dst && len);

    /* Get the hostname. */

    if (!aug_getinetaddr(src, &addr)
        || !aug_inetntop(&addr, host, sizeof(host)))
        return NULL;

    /* Select format based on family. */

    switch (src->un_.family_) {
    case AF_INET:
        fmt = "%s:%d";
        break;
#if HAVE_IPV6
    case AF_INET6:
        fmt = "[%s]:%d";
        break;
#endif /* HAVE_IPV6 */
    default:
#if !defined(_WIN32)
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, EAFNOSUPPORT);
#else /* _WIN32 */
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, WSAEAFNOSUPPORT);
#endif /* _WIN32 */
        return NULL;
    }

    /* Null termination is _not_ guaranteed by snprintf(). */

    ret = snprintf(dst, len, fmt, host, (int)ntohs(src->un_.all_.port_));
    AUG_SNTRUNCF(dst, len, ret);

    if (ret < 0) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return NULL;
    }

    return dst;
}

AUGSYS_API aug_result
aug_setreuseaddr(aug_sd sd, aug_bool on)
{
    int optval = on ? 1 : 0;
    return aug_setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval,
                          sizeof(optval));
}

AUGSYS_API struct aug_endpoint*
aug_getendpoint(const struct addrinfo* addr, struct aug_endpoint* ep)
{
    ep->len_ = (socklen_t)addr->ai_addrlen;
    memcpy(&ep->un_, addr->ai_addr, addr->ai_addrlen);
    return ep;
}

AUGSYS_API struct aug_endpoint*
aug_setinetaddr(struct aug_endpoint* ep, const struct aug_inetaddr* addr)
{
    switch (ep->un_.family_ = addr->family_) {
    case AF_INET:
        ep->len_ = sizeof(ep->un_.ipv4_);
        ep->un_.ipv4_.sin_addr.s_addr = addr->un_.ipv4_.s_addr;
        break;
#if HAVE_IPV6
    case AF_INET6:
        ep->len_ = sizeof(ep->un_.ipv6_);
        memcpy(&ep->un_.ipv6_.sin6_addr, &addr->un_.ipv6_,
               sizeof(addr->un_.ipv6_));
        break;
#endif /* HAVE_IPV6 */
    default:
        SETAFNOSUPPORT_();
        return NULL;
    }
    return ep;
}

AUGSYS_API struct aug_inetaddr*
aug_getinetaddr(const struct aug_endpoint* ep, struct aug_inetaddr* addr)
{
    switch (addr->family_ = ep->un_.family_) {
    case AF_INET:
        addr->un_.ipv4_.s_addr = ep->un_.ipv4_.sin_addr.s_addr;
        break;
#if HAVE_IPV6
    case AF_INET6:
        memcpy(&addr->un_, &ep->un_.ipv6_.sin6_addr,
               sizeof(ep->un_.ipv6_.sin6_addr));
        break;
#endif /* HAVE_IPV6 */
    default:
        SETAFNOSUPPORT_();
        return NULL;
    }
    return addr;
}

AUGSYS_API const struct aug_inetaddr*
aug_inetany(int af)
{
    const void* addr;
    switch (af) {
    case AF_INET:
        addr = &ipv4any_;
        break;
#if HAVE_IPV6
    case AF_INET6:
        addr = &ipv6any_;
        break;
#endif /* HAVE_IPV6 */
    default:
        SETAFNOSUPPORT_();
        return NULL;
    }
    return addr;
}

AUGSYS_API const struct aug_inetaddr*
aug_inetloopback(int af)
{
    const void* addr;
    switch (af) {
    case AF_INET:
        addr = &ipv4loopback_;
        break;
#if HAVE_IPV6
    case AF_INET6:
        addr = &ipv6loopback_;
        break;
#endif /* HAVE_IPV6 */
    default:
        SETAFNOSUPPORT_();
        return NULL;
    }
    return addr;
}

AUGSYS_API aug_bool
aug_acceptagain(struct aug_errinfo* errinfo)
{
    /* FIXME: would be better if aug_tlerr were used. */

    switch (aug_errno(errinfo)) {
    case ECONNABORTED:

    /* Linux accept() passes already-pending network errors on the new socket
       as an error code from accept().  This behaviour differs from other BSD
       socket implementations.  For reliable operation the application should
       detect the network errors defined for the protocol after accept() and
       treat them like EAGAIN by retrying.  In case of TCP/IP these are
       ENETDOWN, EPROTO, ENOPROTOOPT, EHOSTDOWN, ENONET, EHOSTUNREACH,
       EOPNOTSUPP, and ENETUNREACH. */

#if defined(ENETDOWN)
    case ENETDOWN:
#endif /* ENETDOWN */
#if defined(EPROTO)
    case EPROTO:
#endif /* EPROTO */
#if defined(ENOPROTOOPT)
    case ENOPROTOOPT:
#endif /* ENOPROTOOPT */
#if defined(EHOSTDOWN)
    case EHOSTDOWN:
#endif /* EHOSTDOWN */
#if defined(ENONET)
    case ENONET:
#endif /* ENONET */
#if defined(EHOSTUNREACH)
    case EHOSTUNREACH:
#endif /* EHOSTUNREACH */
#if defined(EOPNOTSUPP)
    case EOPNOTSUPP:
#endif /* EOPNOTSUPP */
#if defined(ENETUNREACH)
    case ENETUNREACH:
#endif /* ENETUNREACH */

    case EMFILE: /* Too many files. */
    case EWOULDBLOCK:
        return AUG_TRUE;
    }

    return AUG_FALSE;
}

AUGSYS_API int
aug_getsockerr(aug_sd sd)
{
    int err = 0;
    socklen_t len = sizeof(err);

    /* All errors returned in err. */

    aug_getsockopt(sd, SOL_SOCKET, SO_ERROR, &err, &len);
    return err;
}

AUGSYS_API void
aug_setsockerrinfo(struct aug_errinfo* errinfo, const char* file, int line,
                   aug_sd sd)
{
    aug_setposixerrinfo(errinfo, file, line, aug_getsockerr(sd));
}

AUGSYS_API void
aug_setsockerror(aug_ctx* ctx, const char* file, int line, aug_sd sd)
{
    aug_setposixerror(ctx, file, line, aug_getsockerr(sd));
}
