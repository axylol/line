#include "socket.h"
#include <sys/socket.h>
#include <errno.h>
#include <unordered_map>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include "../config.h"

int jmp_socket(int domain, int type, int protocol)
{
#ifdef SOCKET_DEBUG
    printf("socket(%d, %d, %d)\n", domain, type, protocol);
#endif

    // AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE
    if (domain == 16 && type == 2 && protocol == 0)
    {
        errno = EAFNOSUPPORT;
        return -1;
    }

    int ret = socket(domain, type, protocol);
    if (ret < 0)
    {
        printf("socket error domain=%d type=%d protocol%d errno=%d\n", domain, type, protocol, errno);
    }
    else
    {
#ifdef SOCKET_DEBUG
        printf("socket created %d\n", ret);
#endif
    }
    return ret;
}

#define TSO_TRANSLATED -1
#define TSO_UNTRANSLATED -2
#define TSO_RET_0 -3

#define LEVEL_TRANSLATE -1

std::unordered_map<int, std::unordered_map<int, int>> SOCK_OPTS = {
    {0,
     {
         {LEVEL_TRANSLATE, SOL_IP},
         {10, IP_MULTICAST_TTL},
         {11, IP_MULTICAST_LOOP},

         {29, TSO_RET_0},

         // https://students.mimuw.edu.pl/SO/Linux/Kod/include/linux/socket.h.html
         {33, IP_MULTICAST_TTL},
         {34, IP_MULTICAST_LOOP},
         {35, IP_ADD_MEMBERSHIP},
         {36, IP_DROP_MEMBERSHIP},
     }},
    {1,
     {
         {LEVEL_TRANSLATE, SOL_SOCKET},
         {1, SO_DEBUG},
         {2, SO_REUSEADDR},
         {4, SO_ERROR},
         {5, SO_DONTROUTE},
         {6, SO_BROADCAST},
         {7, SO_SNDBUF},
         {8, SO_RCVBUF},
         {9, SO_KEEPALIVE},
         {10, SO_OOBINLINE},
         {13, SO_LINGER},
         {18, SO_RCVLOWAT},
         {19, SO_SNDLOWAT},
         {20, 4102},
         {21, 4101},
     }},
    {6,
     {{LEVEL_TRANSLATE, IPPROTO_TCP},
      {1, TCP_NODELAY}}}};

int translateSockOpt(int *level, int *option_name)
{
    for (auto &it : SOCK_OPTS)
    {
        if (it.first == *level)
        {
            for (auto &it2 : it.second)
            {
                if (it2.first == LEVEL_TRANSLATE)
                {
                    *level = it2.second;
                }
            }

            for (auto &it2 : it.second)
            {
                if (it2.first == *option_name)
                {
                    if (it2.second < 0)
                        return it2.second;
                    *option_name = it2.second;
                    return TSO_TRANSLATED;
                }
            }

            return TSO_UNTRANSLATED;
        }
    }

    return TSO_UNTRANSLATED;
}

int jmp_setsockopt(int socket, int level, int option_name,
                   const void *option_value, socklen_t option_len)
{
#ifdef SOCKET_DEBUG
    printf("setsockopt socket=%d level=%d optname=%d\n", socket, level, option_name);
#endif

    int lvl = level;
    int opt_name = option_name;
    switch (translateSockOpt(&lvl, &opt_name))
    {
    case TSO_RET_0:
    {
        errno = 0;
        return 0;
    }
    case TSO_UNTRANSLATED:
    {
        printf("untranslated setsockopt level=%d optname=%d\n", level, option_name);
        break;
    }
    }

    int ret = setsockopt(socket, lvl, opt_name, option_value, option_len);
    if (ret < 0)
    {
        printf("setsockopt error socket=%d level=%d optname=%d, errno=%d\n", socket, level, option_name, errno);

        // just act like its successful
        // maybe it doesnt like changing TCP_NODELAY while its connecting?
        if (lvl == IPPROTO_TCP && opt_name == TCP_NODELAY && errno == EINVAL)
        {
            errno = 0;
            return 0;
        }
    }
    return ret;
}

int jmp_getsockopt(int socket, int level, int option_name,
                   void *option_value, socklen_t *option_len)
{
#ifdef SOCKET_DEBUG
    printf("getsockopt socket=%d level=%d optname=%d\n", socket, level, option_name);
#endif

    int lvl = level;
    int opt_name = option_name;
    switch (translateSockOpt(&lvl, &opt_name))
    {
    case TSO_RET_0:
    {
        errno = 0;
        return 0;
    }
    case TSO_UNTRANSLATED:
    {
        printf("untranslated getsockopt level=%d optname=%d\n", level, option_name);
        break;
    }
    }

    int ret = getsockopt(socket, lvl, opt_name, option_value, option_len);
    if (ret < 0)
    {
        printf("getsockopt error socket=%d level=%d optname=%d errno=%d\n", socket, level, option_name, errno);
    }
    else
    {
#ifdef SOCKET_DEBUG
        if (lvl == SOL_SOCKET && opt_name == SO_ERROR)
            printf("SO_ERROR = %d\n", *(int *)option_value);
#endif
    }
    return ret;
}

int jmp_connect(int sockfd, const struct sockaddr *addr,
                socklen_t addrlen)
{
    struct sockaddr_in *addrin = (struct sockaddr_in *)addr;
#ifdef SOCKET_DEBUG
    if (addrin)
    {
        printf("connecting %d to %s %d\n", sockfd,
               inet_ntoa(addrin->sin_addr), ntohs(addrin->sin_port));
    }
#endif

    int ret = connect(sockfd, addr, addrlen);
    if (ret < 0)
    {
        switch (errno)
        {
        case ETIMEDOUT:
        {
            errno = 110;
            break;
        }
        case EINPROGRESS:
        {
            errno = 115;
            break;
        }
        default:
        {
            printf("connect error socket=%d errno=%d\n", sockfd, errno);
            break;
        }
        }
    }
    else
    {
#ifdef SOCKET_DEBUG
        if (addrin)
        {
            printf("connected %d to %s %d\n", sockfd,
                   inet_ntoa(addrin->sin_addr), ntohs(addrin->sin_port));
        }
#endif
    }
    return ret;
}

int jmp_accept(int sockfd, struct sockaddr *addr,
               socklen_t *addrlen)
{
    int ret = accept(sockfd, addr, addrlen);
    if (ret < 0)
    {
        switch (errno)
        {
        case EAGAIN:
            break;
        default:
        {
            printf("accept error socket=%d errno=%d\n", sockfd, errno);
            break;
        }
        }
    }
    else
    {
        if (addr)
        {
#ifdef SOCKET_DEBUG
            struct sockaddr_in *addrin = (struct sockaddr_in *)addr;

            printf("socket %d accepted %d from %s %d\n", sockfd, ret,
                   inet_ntoa(addrin->sin_addr), ntohs(addrin->sin_port));
#endif
        }
        else
        {
#ifdef SOCKET_DEBUG
            printf("socket %d accepted %d\n", sockfd, ret);
#endif
        }
    }

    return ret;
}

int jmp_listen(int sockfd, int backlog)
{
    int ret = listen(sockfd, backlog);
    if (ret < 0)
    {
        printf("listen error %d %d\n", sockfd, errno);
    }
    else
    {
#ifdef SOCKET_DEBUG
        printf("socket %d is now listening\n", sockfd);
#endif
    }
    return ret;
}

int jmp_bind(int sockfd, struct sockaddr *addr,
             socklen_t addrlen)
{
    struct sockaddr_in *addrin = (struct sockaddr_in *)addr;

    int ret = bind(sockfd, addr, addrlen);
    if (ret < 0)
    {
        if (addrin)
        {
            printf("bind error socket=%d errno=%d address=%s %d\n", sockfd, errno, inet_ntoa(addrin->sin_addr), ntohs(addrin->sin_port));
        }
        else
        {
            printf("bind error socket=%d errno=%d\n", sockfd, errno);
        }
    }
    else
    {
#ifdef SOCKET_DEBUG
        printf("socket %d bound to %s %d\n", sockfd,
               inet_ntoa(addrin->sin_addr), ntohs(addrin->sin_port));
#endif
    }

    return ret;
}

int jmp_getsockname(int sockfd, struct sockaddr *addr,
                    socklen_t *addrlen)
{
    struct sockaddr_in *addrin = (struct sockaddr_in *)addr;

    int ret = getsockname(sockfd, addr, addrlen);
    if (ret < 0)
    {

        printf("getsockname error socket=%d errno=%d\n", sockfd, errno);
    }
    else
    {
#ifdef SOCKET_DEBUG
        if (addrin)
        {
            printf("sockname %d is %s %d\n", sockfd,
                   inet_ntoa(addrin->sin_addr), ntohs(addrin->sin_port));
        }
#endif
    }

    return ret;
}

int jmp_getpeername(int sockfd, struct sockaddr *addr,
                    socklen_t *addrlen)
{
    struct sockaddr_in *addrin = (struct sockaddr_in *)addr;

    int ret = getpeername(sockfd, addr, addrlen);
    if (ret < 0)
    {
        printf("getpeername error socket=%d errno=%d\n", sockfd, errno);
    }
    else
    {
#ifdef SOCKET_DEBUG
        if (addrin)
        {
            printf("peername %d is %s %d\n", sockfd,
                   inet_ntoa(addrin->sin_addr), ntohs(addrin->sin_port));
        }
#endif
    }

    return ret;
}

int convertSendFlags(int f)
{
    int flags = 0;
    if (f & 1)
        flags |= MSG_OOB;
    if (f & 2)
        flags |= MSG_PEEK;
    if (f & 4)
        flags |= MSG_DONTROUTE;
    if (f & 8)
        flags |= MSG_CTRUNC;
    if (f & 32)
        flags |= MSG_TRUNC;
    if (f & 64)
        flags |= MSG_DONTWAIT;
    if (f & 128)
        flags |= MSG_EOR;
    if (f & 256)
        flags |= MSG_WAITALL;
    if (f & 16384)
        flags |= MSG_NOSIGNAL;
    return flags;
}

ssize_t jmp_sendmsg(int socket, const struct msghdr *message, int flags)
{
    int ret = sendmsg(socket, message, convertSendFlags(flags));
    if (ret < 0)
    {
        switch (errno)
        {
        case EAGAIN:
            break;
        default:
        {
            printf("sendmsg error socket=%d errno=%d\n", socket, errno);
            break;
        }
        }
    }
    return ret;
}

ssize_t jmp_recvmsg(int socket, struct msghdr *message, int flags)
{
    int ret = recvmsg(socket, message, convertSendFlags(flags));
    if (ret < 0)
    {
        switch (errno)
        {
        case EAGAIN:
            break;
        default:
        {
            printf("recvmsg error socket=%d errno=%d\n", socket, errno);
            break;
        }
        }
    }
    return ret;
}

ssize_t jmp_send(int sockfd, const void *buf, size_t len, int flags)
{
    ssize_t ret = send(sockfd, buf, len, convertSendFlags(flags));
    if (ret < 0)
    {
        switch (errno)
        {
        case EAGAIN:
            break;
        default:
        {
            printf("send error socket=%d len=%d errno=%d\n", socket, len, errno);
            break;
        }
        }
    }
    return ret;
}

ssize_t jmp_recv(int sockfd, void *buf, size_t len, int flags)
{
    ssize_t ret = recv(sockfd, buf, len, convertSendFlags(flags));
    if (ret < 0)
    {
        switch (errno)
        {
        case EAGAIN:
            break;
        default:
        {
            printf("recv error socket=%d len=%d errno=%d\n", socket, len, errno);
            break;
        }
        }
    }
    return ret;
}

Stubs GetSocketStubs()
{
    return {
        DEF_STUB(socket),
        DEF_STUB(setsockopt),
        DEF_STUB(getsockopt),

        DEF_STUB(connect),
        DEF_STUB(accept),
        DEF_STUB(listen),
        DEF_STUB(bind),

        DEF_STUB(getsockname),
        DEF_STUB(getpeername),

        DEF_STUB(sendmsg),
        DEF_STUB(recvmsg),

        DEF_STUB(send),
        DEF_STUB(recv),
    };
}