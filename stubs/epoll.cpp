#include "epoll.h"
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/time.h>
#include "../config.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#define LINUX_EPOLL_CTL_ADD 1
#define LINUX_EPOLL_CTL_DEL 2
#define LINUX_EPOLL_CTL_MOD 3

#pragma push(pack, 1)
struct linux_epoll_event
{
    uint32_t events;
    char data[8];
};
#pragma pop(pack)

static_assert(sizeof(linux_epoll_event) == 12, "size mismatch");

int epoll_fd_counter = 0;
int open_dummy_epoll_fd(const char *name)
{
    // Clear file if exists
    FILE *file = fopen(name, "w");
    if (file)
    {
        fclose(file);
    }
    else
    {
        creat(name, 0600);
    }
    return open(name, O_RDWR);
}

#ifdef SHIT_EPOLL

struct EpollSubData
{
    bool used;
    int fd;
    int events;
    char data[8];
};
struct EpollData
{
    int fd;
    EpollSubData sub[1024];
};

EpollData *epolls[1024];

int create_epoll()
{
    char name[1024];
    sprintf(name, "./line/%s_%d", "epoll", epoll_fd_counter++);
    int ret = open_dummy_epoll_fd(name);
    if (ret < 0)
        return ret;

    EpollData *data = new EpollData();
    data->fd = ret;
    memset(data->sub, 0, sizeof(data->sub));

    epolls[ret] = data;

    errno = 0;
    return ret;
}

int jmp_epoll_create1(int flags)
{
    return create_epoll();
}

int jmp_epoll_create(int size)
{
    return create_epoll();
}

int jmp_epoll_ctl(int epfd, int op, int fd, linux_epoll_event *event)
{
    if (epfd < 0 || epfd >= 1024)
    {
        printf("epoll ctl bad epfd\n");

        errno = EBADF;
        return -1;
    }

    if (fd < 0 || fd >= 1024)
    {
        printf("epoll ctl bad fd\n");

        errno = EBADF;
        return -1;
    }

    if (epfd == fd)
    {
        printf("epoll ctl epfd == fd\n");

        errno = EINVAL;
        return -1;
    }

    if (epolls[epfd] == NULL)
    {
        printf("epoll ctl bad epfd\n");

        errno = EBADF;
        return -1;
    }

    switch (op)
    {
    case LINUX_EPOLL_CTL_ADD:
    {
        EpollSubData *sub = &epolls[epfd]->sub[fd];
        if (sub->used)
        {
            printf("epoll already added\n");
            errno = EEXIST;
            return -1;
        }

        sub->fd = fd;
        sub->events = event->events;
        memcpy(sub->data, event->data, 8);
        sub->used = true;

        errno = 0;
        return 0;
    }
    case LINUX_EPOLL_CTL_DEL:
    {
        EpollSubData *sub = &epolls[epfd]->sub[fd];
        if (!sub->used)
        {
            errno = ENOENT;
            return -1;
        }

        sub->used = false;

        errno = 0;
        return 0;
    }
    case LINUX_EPOLL_CTL_MOD:
    {
        EpollSubData *sub = &epolls[epfd]->sub[fd];
        if (!sub->used)
        {
            errno = ENOENT;
            return -1;
        }

        sub->events = event->events;
        memcpy(sub->data, event->data, 8);

        errno = 0;
        return 0;
    }
    }

    printf("UNHANDLED COMMAND EPOLL\n");
    errno = EINVAL;
    return -1;
}

int epollLinuxToMsysEvents(int e)
{
    int events = 0;
    if (e & 1) // EPOLLIN
        events |= POLLIN;
    if (e & 2) // EPOLLPRI
        events |= POLLPRI;
    if (e & 4) // EPOLLOUT
        events |= POLLOUT;
    if (e & 8) // EPOLLERR
        events |= POLLERR;
    if (e & 16) // EPOLLHUP
        events |= POLLHUP;
    if (e & 32) // EPOLLNVAL
        events |= POLLNVAL;
    if (e & 64) // EPOLLRDNORM
        events |= POLLRDNORM;
    if (e & 128) // EPOLLRDBAND
        events |= POLLRDBAND;
    if (e & 256) // EPOLLWRNORM
        events |= POLLWRNORM;
    if (e & 512) // EPOLLWRBAND
        events |= POLLWRBAND;
    return events;
}

int epollMsysToLinuxEvents(int e)
{
    int events = 0;
    if (e & POLLIN)
        events |= 1; // EPOLLIN
    if (e & POLLPRI) // EPOLLPRI
        events |= 2;
    if (e & POLLOUT) // EPOLLOUT
        events |= 4;
    if (e & POLLERR) // EPOLLERR
        events |= 8;
    if (e & POLLHUP) // EPOLLHUP
        events |= 16;
    if (e & POLLNVAL)
        events |= 32;
    if (e & POLLRDNORM) // EPOLLRDNORM
        events |= 64;
    if (e & POLLRDBAND) // EPOLLRDBAND
        events |= 128;
    if (e & POLLWRNORM) // EPOLLWRNORM
        events |= 256;
    if (e & POLLWRBAND) // EPOLLWRBAND
        events |= 512;
    return events;
}

int jmp_epoll_wait(int epfd, linux_epoll_event *events,
                   int maxevents, int timeout)
{
    if (!events || maxevents < 1)
    {
        printf("epoll wait bad events\n");
        errno = EINVAL;
        return -1;
    }
    if (epfd < 0 || epfd >= 1024)
    {
        printf("epoll wait bad epfd\n");
        errno = EBADF;
        return -1;
    }

    EpollData *epoll = epolls[epfd];
    if (epoll == NULL)
    {
        printf("epoll wait not found\n");
        errno = EBADF;
        return -1;
    }

    for (int i = 0; i < maxevents; i++)
        events[i].events = 0;

    while (true)
    {
        std::vector<EpollSubData *> subs = {};

        for (size_t i = 0; i < 1024; i++)
        {
            if (subs.size() >= maxevents)
                break;
            EpollSubData *sub = &epolls[epfd]->sub[i];
            if (!sub->used)
                continue;
            subs.push_back(sub);
        }

        struct pollfd *pollSockets = (struct pollfd *)malloc(sizeof(struct pollfd) * subs.size());
        for (size_t i = 0; i < subs.size(); i++)
        {
            pollSockets[i].fd = subs[i]->fd;
            pollSockets[i].events = epollLinuxToMsysEvents(subs[i]->events);
            pollSockets[i].revents = 0;
        }

        int ret = poll(pollSockets, subs.size(), timeout < 0 ? 1000 : timeout);
        if (ret < 0)
        {
            free(pollSockets);
            return ret;
        }

        for (int i = 0; i < ret; i++)
        {
            events[i].events = epollMsysToLinuxEvents(pollSockets[i].revents);
            memcpy(events[i].data, epolls[epfd]->sub[pollSockets[i].fd].data, 8);
        }

        if (ret < 1 && timeout < 0)
        {
            free(pollSockets);
            continue;
        }

        free(pollSockets);

        usleep(1000);

        return ret;
    }

    return 0;
}

#else

int create_epoll()
{
    char name[1024];
    sprintf(name, "./line/%s_%d", "epoll", epoll_fd_counter++);
    int ret = open_dummy_epoll_fd(name);
    if (ret < 0)
        return ret;
    errno = 0;
    return ret;
}

int jmp_epoll_create1(int flags)
{
    return create_epoll();
}

int jmp_epoll_create(int size)
{
    return create_epoll();
}

int jmp_epoll_ctl(int epfd, int op, int fd, linux_epoll_event *event)
{
    errno = 0;
    return 0;
}

#include <unistd.h>
int jmp_epoll_wait(int epfd, linux_epoll_event *events,
                   int maxevents, int timeout)
{
    /*int timeout_us = timeout * 1000;
    if (timeout_us < 0)
        timeout_us = 16000;
    usleep(timeout_us);*/

    // TODO: this is for wmmt only
    usleep(16000);

    errno = 0;
    return 0;
}
#endif

int jmp_timerfd_create(int a1, int a2)
{
    return -1;
}

int jmp_close(int fd)
{
#ifdef CLOSE_DEBUG
    printf("close %d\n", fd);
#endif

#ifdef SHIT_EPOLL
    if (fd >= 0 && fd < 1024)
    {
        if (epolls[fd])
        {
            // WARNING: MEMORY LEAK
            epolls[fd] = NULL;
        }
    }

    for (int i = 0; i < 1024; i++)
    {
        if (!epolls[i])
            continue;

        if (epolls[i]->sub[fd].used)
        {
            epolls[i]->sub[fd].used = false;
        }
    }
#endif
    return close(fd);
}

Stubs GetEpollStubs()
{
    static bool initialized = false;
    if (!initialized)
    {
        initialized = true;

#ifdef SHIT_EPOLL
        memset(epolls, 0, sizeof(epolls));
#endif
    }

    return {
        DEF_STUB(epoll_create1),
        DEF_STUB(epoll_create),
        DEF_STUB(epoll_ctl),
        DEF_STUB(epoll_wait),

        DEF_STUB(timerfd_create),

        DEF_STUB(close)};
}