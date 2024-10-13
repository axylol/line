#include "fcntl.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <mqueue.h>
#include "../config.h"
#include "../whitelist.h"
#include <limits.h>

int convertLinuxToMsysFlags(int arg)
{
    int flags = 0;
    if (arg & 1)
        flags |= O_WRONLY;
    if (arg & 2)
        flags |= O_RDWR;
    if (arg & 64)
        flags |= O_CREAT;
    if (arg & 128)
        flags |= O_EXCL;
    if (arg & 256)
        flags |= O_NOCTTY;
    if (arg & 512)
        flags |= O_TRUNC;
    if (arg & 1024)
        flags |= O_APPEND;
    if (arg & 2048)
        flags |= O_NONBLOCK;
    if (arg & 4096)
        flags |= O_SYNC;
    if (arg & 8192)
        flags |= FASYNC;
    if (arg & 16384)
        flags |= O_DIRECT;
    if (arg & 65536)
        flags |= O_DIRECTORY;
    if (arg & 131072)
        flags |= O_NOFOLLOW;
    if (arg & 524288)
        flags |= O_CLOEXEC;
    return flags;
}

int convertMsysToLinuxFlags(int arg)
{
    int flags = 0;
    if (arg & O_WRONLY)
        flags |= 1;
    if (arg & O_RDWR)
        flags |= 2;
    if (arg & O_CREAT)
        flags |= 64;
    if (arg & O_EXCL)
        flags |= 128;
    if (arg & O_NOCTTY)
        flags |= 256;
    if (arg & O_TRUNC)
        flags |= 512;
    if (arg & O_APPEND)
        flags |= 1024;
    if (arg & O_NONBLOCK)
        flags |= 2048;
    if (arg & O_SYNC)
        flags |= 4096;
    if (arg & FASYNC)
        flags |= 8192;
    if (arg & O_DIRECT)
        flags |= 16384;
    if (arg & O_DIRECTORY)
        flags |= 65536;
    if (arg & O_NOFOLLOW)
        flags |= 131072;
    if (arg & O_CLOEXEC)
        flags |= 524288;
    return flags;
}

#pragma push(pack, 1)
struct linux_flock
{
    short l_type;
    short l_whence;
    long l_start;
    long l_len;
    int l_pid;
};

struct linux_flock64
{
    short l_type;
    short l_whence;
    long l_start;
    long l_start_high;
    long l_len;
    long l_len_high;
    int l_pid;
};
#pragma pop(pack)

static_assert(sizeof(struct linux_flock) == 16, "size mismatch");
static_assert(sizeof(struct linux_flock64) == 24, "size mismatch");

int jmp_fcntl(int fd, int op, int arg)
{
    switch (op)
    {
    case 1:
    {
        return fcntl(fd, F_GETFD);
    }
    case 2:
    {
        op = F_SETFD;
        break;
    }
    case 3:
    {
        int ret = fcntl(fd, F_GETFL);
        if (ret < 0)
        {
            printf("fcntl F_GETFL error fd=%d errno=%d\n", fd, errno);
            return ret;
        }
        return convertMsysToLinuxFlags(ret);
    }
    case 4:
    {
        op = F_SETFL;
        int ret = fcntl(fd, F_SETFL, convertLinuxToMsysFlags(arg));
        if (ret < 0)
        {
            printf("fcntl F_SETFL error fd=%d errno=%d\n", fd, errno);
        }
        return ret;
    }
    case 5: // F_GETLK
    {
        int ret = fcntl(fd, F_GETLK, arg);
        if (ret < 0)
            return ret;

        auto f = (struct flock *)arg;

        struct linux_flock temp;
        temp.l_type = f->l_type - 1;
        temp.l_whence = f->l_whence;
        temp.l_start = f->l_start;
        temp.l_len = f->l_len;
        temp.l_pid = f->l_pid;

        memcpy(f, &temp, sizeof(temp));

        return ret;
    }
    case 6:
    {
        op = F_SETLK;

        auto f = (struct linux_flock *)arg;

        struct flock temp;
        memset(&temp, 0, sizeof(temp));
        temp.l_type = f->l_type + 1;
        temp.l_whence = f->l_whence;
        temp.l_start = f->l_start;
        temp.l_len = f->l_len;
        temp.l_pid = f->l_pid;
        memcpy(f, &temp, sizeof(temp));
        break;
    }
    case 9:
    {
        return fcntl(fd, F_GETOWN);
    }
    case 8:
    {
        op = F_SETOWN;
        break;
    }
    case 12: // F_GETLK64
    {
        int ret = fcntl(fd, F_GETLK, arg);
        if (ret < 0)
            return ret;

        auto f = (struct flock *)arg;

        struct linux_flock64 temp;
        temp.l_type = f->l_type - 1;
        temp.l_whence = f->l_whence;
        temp.l_start = f->l_start;
        temp.l_len = f->l_len;
        temp.l_pid = f->l_pid;

        memcpy(f, &temp, sizeof(temp));

        return ret;
    }
    case 13: // F_SETLK64
    {
        op = F_SETLK;

        auto f = (struct linux_flock64 *)arg;

        struct flock temp;
        memset(&temp, 0, sizeof(temp));
        temp.l_type = f->l_type + 1;
        temp.l_whence = f->l_whence;
        temp.l_start = f->l_start;
        temp.l_len = f->l_len;
        temp.l_pid = f->l_pid;
        memcpy(f, &temp, sizeof(temp));
        break;
    }
    default:
    {
        printf("Untranslated fcntl %d\n", op);
        break;
    }
    }

    int ret = fcntl(fd, op, arg);
    if (ret == -1)
        printf("fcntl error %d %d\n", op, errno);
    return ret;
}

int jmp_open(const char *_file, int oflag, ...)
{
#ifdef ENV_REDIRECTION
    char file[PATH_MAX];
    fixPathIfNeeded(file, _file);
#else
    const char *file = _file;
#endif
#ifdef AGGRESIVE_FILE_CHECK
    if (!isPathWhitelisted(file))
    {
        errno = EACCES;
        return -1;
    }
#endif

    int flags = convertLinuxToMsysFlags(oflag);
    if (flags & O_CREAT)
    {
        // O_CREAT is broken, returns fd 0 for some reason
        // hacky ass fix

        va_list arg;
        va_start(arg, oflag);
        int mode = va_arg(arg, int);
        va_end(arg);

        FILE *f = fopen(file, "r");
        if (!f)
        {
            creat(file, mode);
        }
        else
        {
            fclose(f);
        }

        flags &= ~O_CREAT;
    }

    return open(file, flags);
}

int jmp_open64(const char *_file, int oflag, ...)
{
#ifdef ENV_REDIRECTION
    char file[PATH_MAX];
    fixPathIfNeeded(file, _file);
#else
    const char *file = _file;
#endif
#ifdef AGGRESIVE_FILE_CHECK
    if (!isPathWhitelisted(file))
    {
        errno = EACCES;
        return -1;
    }
#endif

    int flags = convertLinuxToMsysFlags(oflag);
    if (flags & O_CREAT)
    {
        // O_CREAT is broken, returns fd 0 for some reason
        // hacky ass fix

        va_list arg;
        va_start(arg, oflag);
        int mode = va_arg(arg, int);
        va_end(arg);

        FILE *f = fopen(file, "r");
        if (!f)
        {
            creat(file, mode);
        }
        else
        {
            fclose(f);
        }

        flags &= ~O_CREAT;
    }

    return open(file, flags);
}

int jmp___open_2(const char *file, int oflag)
{
    return jmp_open(file, oflag);
}

int jmp_creat(const char *_name, mode_t mode)
{
#ifdef ENV_REDIRECTION
    char name[PATH_MAX];
    fixPathIfNeeded(name, _name);
#else
    const char *name = _name;
#endif
#ifdef AGGRESIVE_FILE_CHECK
    if (!isPathWhitelisted(name))
    {
        errno = EACCES;
        return -1;
    }
#endif
    return creat(name, mode);
}

Stubs GetFcntlStubs()
{
    return {
        DEF_STUB(fcntl),

        DEF_STUB(open),
        DEF_STUB(open64),
        DEF_STUB(__open_2),

        DEF_STUB(creat),
    };
}