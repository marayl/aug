/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_SOCKET_H
#define AUGSYS_SOCKET_H

#include "augsys/config.h"
#include "augsys/types.h"

#if !defined(_WIN32)
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>
#else /* _WIN32 */
# include <winsock2.h>
# include <ws2tcpip.h>
# if !defined(bzero)
#  define bzero ZeroMemory
# endif /* !bzero */
# if !defined(SHUT_RD)
#  define SHUT_RD SD_RECV
#  define SHUT_WR SD_SEND
#  define SHUT_RDWR SD_BOTH
# endif /* !SHUT_RD */
typedef int socklen_t;
#endif /* _WIN32 */

#define AUG_INETLEN(x) (AF_INET6 == x \
                        ? sizeof(struct sockaddr_in6) \
                        : sizeof(struct sockaddr_in))

#define AUG_MAXSOCKADDR 128

AUGSYS_API int
aug_socket(int domain, int type, int protocol);

AUGSYS_API int
aug_accept(int s, struct sockaddr* addr, socklen_t* addrlen);

AUGSYS_API int
aug_bind(int s, const struct sockaddr* addr, socklen_t addrlen);

/** Remember that, for non-blocking sockets, connect() can fail with
    EINPROGRESS.  Use poll() or select() on write-status to determine
    completion. */

AUGSYS_API int
aug_connect(int s, const struct sockaddr* addr, socklen_t addrlen);

AUGSYS_API int
aug_getpeername(int s, struct sockaddr* addr, socklen_t* addrlen);

AUGSYS_API int
aug_getsockname(int s, struct sockaddr* addr, socklen_t* addrlen);

AUGSYS_API int
aug_listen(int s, int backlog);

AUGSYS_API ssize_t
aug_recv(int s, void* buf, size_t len, int flags);

AUGSYS_API ssize_t
aug_recvfrom(int s, void* buf, size_t len, int flags, struct sockaddr* from,
             socklen_t* fromlen);

AUGSYS_API ssize_t
aug_send(int s, const void* buf, size_t len, int flags);

AUGSYS_API ssize_t
aug_sendto(int s, const void* buf, size_t len, int flags,
           const struct sockaddr* to, socklen_t tolen);

AUGSYS_API int
aug_getsockopt(int s, int level, int optname, void* optval,
               socklen_t* optlen);

AUGSYS_API int
aug_setsockopt(int s, int level, int optname, const void* optval,
               socklen_t optlen);

AUGSYS_API int
aug_shutdown(int s, int how);

AUGSYS_API int
aug_socketpair(int domain, int type, int protocol, int sv[2]);

#define AUG_INETLEN(x) (AF_INET6 == x \
                        ? sizeof(struct sockaddr_in6) \
                        : sizeof(struct sockaddr_in))

AUGSYS_API const char*
aug_inetntop(int af, const char* src, char* dst, socklen_t size);

AUGSYS_API int
aug_inetpton(int af, const char* src, void* dst);

AUGSYS_API void
aug_freeaddrinfo(struct addrinfo* res);

AUGSYS_API int
aug_getaddrinfo(const char* host, const char* serv,
                const struct addrinfo* hints, struct addrinfo** res);

AUGSYS_API int
aug_setreuseaddr(int s, int on);

AUGSYS_API int
aug_getsockaf(int s);

#endif /* AUGSYS_SOCKET_H */
