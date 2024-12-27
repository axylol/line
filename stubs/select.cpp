
#include "select.h"
#include <sys/select.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "../config.h"

#define LINUX_FDS_BITS(set) ((set)->__fds_bits)

#define LINUX_NFDBITS (8 * (int)sizeof(long int))
#define LINUX_FD_SETSIZE 1024

#define LINUX_FD_ELT(d) ((d) / LINUX_NFDBITS)
#define LINUX_FD_MASK(d) ((__fd_mask)(1UL << ((d) % LINUX_NFDBITS)))

#pragma pack(push, 1)
struct linux_fd_set
{
    long int __fds_bits[LINUX_FD_SETSIZE / LINUX_NFDBITS];
};
#pragma pack(pop)

static_assert(sizeof(linux_fd_set) == 128, "size mismatch");

#define LINUX_FD_ZERO(s)                                                    \
    do                                                                      \
    {                                                                       \
        unsigned int __i;                                                   \
        linux_fd_set *__arr = (s);                                          \
        for (__i = 0; __i < sizeof(linux_fd_set) / sizeof(long int); ++__i) \
            LINUX_FDS_BITS(__arr)                                           \
        [__i] = 0;                                                          \
    } while (0)
#define LINUX_FD_SET(d, s) ((void)(LINUX_FDS_BITS(s)[LINUX_FD_ELT(d)] |= LINUX_FD_MASK(d)))
#define LINUX_FD_CLR(d, s) ((void)(LINUX_FDS_BITS(s)[LINUX_FD_ELT(d)] &= ~LINUX_FD_MASK(d)))
#define LINUX_FD_ISSET(d, s) ((LINUX_FDS_BITS(s)[LINUX_FD_ELT(d)] & LINUX_FD_MASK(d)) != 0)

int jmp_select(int nfds, linux_fd_set *readfds, linux_fd_set *writefds, linux_fd_set *exceptfds, struct timeval *timeout)
{
    fd_set *readfds_real = NULL;
    fd_set *writefds_real = NULL;
    fd_set *exceptfds_real = NULL;

    if (readfds)
    {
        readfds_real = (fd_set *)malloc(128);
        FD_ZERO(readfds_real);
        for (int i = 0; i < nfds; i++)
            if (LINUX_FD_ISSET(i, readfds))
                FD_SET(i, readfds_real);
    }
    if (writefds)
    {
        writefds_real = (fd_set *)malloc(128);
        FD_ZERO(writefds_real);
        for (int i = 0; i < nfds; i++)
            if (LINUX_FD_ISSET(i, writefds))
                FD_SET(i, writefds_real);
    }
    if (exceptfds)
    {
        exceptfds_real = (fd_set *)malloc(128);
        FD_ZERO(exceptfds_real);
        for (int i = 0; i < nfds; i++)
            if (LINUX_FD_ISSET(i, exceptfds))
                FD_SET(i, exceptfds_real);
    }

    int ret = select(nfds, readfds_real, writefds_real, exceptfds_real, timeout);
    if (ret > 0)
    {
        if (readfds)
        {
            LINUX_FD_ZERO(readfds);

            for (int i = 0; i < nfds; i++)
                if (FD_ISSET(i, readfds_real))
                    LINUX_FD_SET(i, readfds);
        }
        if (writefds)
        {
            LINUX_FD_ZERO(writefds);

            for (int i = 0; i < nfds; i++)
                if (FD_ISSET(i, writefds_real))
                    LINUX_FD_SET(i, writefds);
        }
        if (exceptfds)
        {
            LINUX_FD_ZERO(exceptfds);

            for (int i = 0; i < nfds; i++)
                if (FD_ISSET(i, exceptfds_real))
                    LINUX_FD_SET(i, exceptfds);
        }
    }
    else if (ret < 0)
    {
        printf("select error nfds=%d errno=%d\n", nfds, errno);
    } else {
        if (readfds)
            LINUX_FD_ZERO(readfds);
        if (writefds)
            LINUX_FD_ZERO(writefds);
        if (exceptfds)
            LINUX_FD_ZERO(exceptfds);
    }

    if (readfds_real)
        free(readfds_real);
    if (writefds_real)
        free(writefds_real);
    if (exceptfds_real)
        free(exceptfds_real);

    return ret;
}

Stubs GetSelectStubs()
{
    return {
        //DEF_STUB(select)
    };
}