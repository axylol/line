#include "unistd.h"
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include "../config.h"
#include "../whitelist.h"

ssize_t jmp_write(int fd, const void *buf, size_t count)
{
    ssize_t ret = write(fd, buf, count);
    return ret;
}

ssize_t jmp_read(int fd, void *buf, size_t count)
{
    ssize_t ret = read(fd, buf, count);
    return ret;
}

int jmp_ftruncate64(int fd, int64_t length)
{
    return ftruncate(fd, length);
}

int64_t jmp_lseek64(int fd, int64_t offset, int whence)
{
    return lseek(fd, offset, whence);
}

int jmp_chmod(const char *_path, mode_t mode)
{
#ifdef ENV_REDIRECTION
    char path[PATH_MAX];
    fixPathIfNeeded(path, _path);
#else
    const char *path = _path;
#endif
#ifdef AGGRESIVE_FILE_CHECK
    if (!isPathWhitelisted(path))
    {
        errno = EACCES;
        return -1;
    }
#endif
    return chmod(path, mode);
}

int jmp_chown(const char *_path, uid_t owner, gid_t group)
{
#ifdef ENV_REDIRECTION
    char path[PATH_MAX];
    fixPathIfNeeded(path, _path);
#else
    const char *path = _path;
#endif
#ifdef AGGRESIVE_FILE_CHECK
    if (!isPathWhitelisted(path))
    {
        errno = EACCES;
        return -1;
    }
#endif
    return chown(path, owner, group);
}

int jmp_rename(const char *_old, const char *_neww)
{
#ifdef ENV_REDIRECTION
    char old[PATH_MAX];
    fixPathIfNeeded(old, _old);

    char neww[PATH_MAX];
    fixPathIfNeeded(neww, _neww);
#else
    const char *old = _old;
    const char *neww = _neww;
#endif
#ifdef AGGRESIVE_FILE_CHECK
    if (!isPathWhitelisted(old))
    {
        errno = EACCES;
        return -1;
    }
    if (!isPathWhitelisted(neww))
    {
        errno = EACCES;
        return -1;
    }
#endif
    return rename(old, neww);
}

int jmp_symlink(const char *_from, const char *_to)
{
#ifdef ENV_REDIRECTION
    char from[PATH_MAX];
    fixPathIfNeeded(from, _from);
    char to[PATH_MAX];
    fixPathIfNeeded(to, _to);
#else
    const char *from = _from;
    const char *to = _to;
#endif
#ifdef AGGRESIVE_FILE_CHECK
    if (!isPathWhitelisted(from))
    {
        errno = EACCES;
        return -1;
    }
    if (!isPathWhitelisted(to))
    {
        errno = EACCES;
        return -1;
    }
#endif
    return symlink(from, to);
}

int jmp_link(const char *_from, const char *_to)
{
#ifdef ENV_REDIRECTION
    char from[PATH_MAX];
    fixPathIfNeeded(from, _from);
    char to[PATH_MAX];
    fixPathIfNeeded(to, _to);
#else
    const char *from = _from;
    const char *to = _to;
#endif
#ifdef AGGRESIVE_FILE_CHECK
    if (!isPathWhitelisted(from))
    {
        errno = EACCES;
        return -1;
    }
    if (!isPathWhitelisted(to))
    {
        errno = EACCES;
        return -1;
    }
#endif
    return link(from, to);
}

int jmp_unlink(const char *_name)
{
#ifdef ENV_REDIRECTION
    char name[PATH_MAX];
    fixPathIfNeeded(name, _name);
#else
    const char *name = _name;
#endif
    if (!isPathWhitelisted(name))
    {
        errno = EACCES;
        return -1;
    }
    return unlink(name);
}

Stubs GetUnistdStubs()
{
    return {
        DEF_STUB(write),
        DEF_STUB(read),

        DEF_STUB(ftruncate64),
        DEF_STUB(lseek64),

        DEF_STUB(chmod),
        DEF_STUB(chown),

        DEF_STUB(rename),
        DEF_STUB(symlink),
        DEF_STUB(link),
        DEF_STUB(unlink),
    };
}