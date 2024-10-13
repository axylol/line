#include "poll.h"
#include <sys/poll.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include "../config.h"

int pollLinuxToMsysEvents(int e)
{
    int events = 0;
    if (e & 1)
        events |= POLLIN;
    if (e & 2)
        events |= POLLPRI;
    if (e & 4)
        events |= POLLOUT;
    if (e & 8)
        events |= POLLERR;
    if (e & 16)
        events |= POLLHUP;
    if (e & 32)
        events |= POLLNVAL;
    if (e & 64)
        events |= POLLRDNORM;
    if (e & 128)
        events |= POLLRDBAND;
    if (e & 256)
        events |= POLLWRNORM;
    if (e & 512)
        events |= POLLWRBAND;
    return events;
}

int pollMsysToLinuxEvents(int e)
{
    int events = 0;
    if (e & POLLIN)
        events |= 1;
    if (e & POLLPRI)
        events |= 2;
    if (e & POLLOUT)
        events |= 4;
    if (e & POLLERR)
        events |= 8;
    if (e & POLLHUP)
        events |= 16;
    if (e & POLLNVAL)
        events |= 32;
    if (e & POLLRDNORM)
        events |= 64;
    if (e & POLLRDBAND)
        events |= 128;
    if (e & POLLWRNORM)
        events |= 256;
    if (e & POLLWRBAND)
        events |= 512;
    return events;
}

#pragma push(pack, 1)
struct linux_pollfd
{
    int fd;        /* file descriptor */
    short events;  /* requested events */
    short revents; /* returned events */
};
#pragma pop(pack)

int jmp_poll(struct linux_pollfd *fds, nfds_t nfds, int timeout)
{
    struct pollfd *fdsm = (struct pollfd *)malloc(sizeof(struct pollfd) * nfds);
    for (nfds_t i = 0; i < nfds; i++)
    {
        fdsm[i].fd = fds[i].fd;
        fdsm[i].events = pollLinuxToMsysEvents(fds[i].events);
        fdsm[i].revents = 0;
    }

    int ret = poll(fdsm, nfds, timeout);
    if (ret < 0)
    {
        free(fdsm);

        printf("poll error nfds=%d errno=%d", nfds, errno);
        return ret;
    }

    for (int i = 0; i < ret; i++)
    {
        fds[i].fd = fdsm[i].fd;
        fds[i].events = pollMsysToLinuxEvents(fdsm[i].events);
        fds[i].revents = pollMsysToLinuxEvents(fdsm[i].revents);
    }

    free(fdsm);

    return ret;
}

Stubs GetPollStubs()
{
    return {
        DEF_STUB(poll),
    };
}