#include "stat.h"
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "../config.h"
#include "../whitelist.h"

#pragma push(pack, 1)
struct linux_stat
{
    unsigned long st_dev; // 0
    unsigned long st_dev_high;
    char pad[4];            // 8
    unsigned long st_ino;   // 12
    unsigned long st_mode;  // 16
    unsigned long st_nlink; // 20
    unsigned long st_uid;   // 24
    unsigned long st_gid;   // 28
    unsigned long st_rdev;  // 32
    unsigned long st_rdev_high;
    char pad2[4];             // 40
    long st_size;             // 44
    unsigned long st_blksize; // 48
    unsigned long st_blocks;  // 52
    timespec st_atim;         // 56
    timespec st_mtim;         // 64
    timespec st_ctim;         // 72
    char pad3[8];             // 80
};

struct linux_stat64
{
    unsigned long st_dev; // 0
    unsigned long st_dev_high;
    char pad[8];            // 8
    unsigned long st_mode;  // 16
    unsigned long st_nlink; // 20
    unsigned long st_uid;   // 24
    unsigned long st_gid;   // 28
    unsigned long st_rdev;  // 32
    unsigned long st_rdev_high;
    char pad2[4];             // 40
    long st_size;             // 44
    long st_size_high;        // 48
    unsigned long st_blksize; // 52
    unsigned long st_blocks;  // 56
    unsigned long st_blocks_high;
    timespec st_atim;     // 64
    timespec st_mtim;     // 72
    timespec st_ctim;     // 80
    unsigned long st_ino; // 88
    char pad3[4];
};
#pragma pop(pack)

static_assert(sizeof(struct linux_stat) == 88, "size mismatch");
static_assert(sizeof(struct linux_stat64) == 96, "size mismatch");

void stat_msys_to_linux(struct stat temp, struct linux_stat *stat_buf)
{
    memset(stat_buf, 0, sizeof(struct linux_stat));

    stat_buf->st_dev = temp.st_dev;
    stat_buf->st_dev_high = 0;
    stat_buf->st_ino = temp.st_ino;
    stat_buf->st_mode = temp.st_mode;
    stat_buf->st_nlink = temp.st_nlink;
    stat_buf->st_uid = temp.st_uid;
    stat_buf->st_gid = temp.st_gid;
    stat_buf->st_rdev = temp.st_rdev;
    stat_buf->st_rdev_high = 0;
    stat_buf->st_size = temp.st_size;
    stat_buf->st_blksize = temp.st_blksize;
    stat_buf->st_blocks = temp.st_blocks;
    stat_buf->st_atim.tv_sec = temp.st_atim.tv_sec;
    stat_buf->st_atim.tv_nsec = temp.st_atim.tv_nsec;
    stat_buf->st_mtim.tv_sec = temp.st_mtim.tv_sec;
    stat_buf->st_mtim.tv_nsec = temp.st_mtim.tv_nsec;
    stat_buf->st_ctim.tv_sec = temp.st_ctim.tv_sec;
    stat_buf->st_ctim.tv_nsec = temp.st_ctim.tv_nsec;
}

void stat64_msys_to_linux(struct stat temp, struct linux_stat64 *stat_buf)
{
    memset(stat_buf, 0, sizeof(struct linux_stat64));

    stat_buf->st_dev = temp.st_dev;
    stat_buf->st_dev_high = 0;
    stat_buf->st_mode = temp.st_mode;
    stat_buf->st_nlink = temp.st_nlink;
    stat_buf->st_uid = temp.st_uid;
    stat_buf->st_gid = temp.st_gid;
    stat_buf->st_rdev = temp.st_rdev;
    stat_buf->st_rdev_high = 0;
    stat_buf->st_size = temp.st_size;
    stat_buf->st_size_high = 0;
    stat_buf->st_blksize = temp.st_blksize;
    stat_buf->st_blocks = temp.st_blocks;
    stat_buf->st_blocks_high = 0;
    stat_buf->st_atim.tv_sec = temp.st_atim.tv_sec;
    stat_buf->st_atim.tv_nsec = temp.st_atim.tv_nsec;
    stat_buf->st_mtim.tv_sec = temp.st_mtim.tv_sec;
    stat_buf->st_mtim.tv_nsec = temp.st_mtim.tv_nsec;
    stat_buf->st_ctim.tv_sec = temp.st_ctim.tv_sec;
    stat_buf->st_ctim.tv_nsec = temp.st_ctim.tv_nsec;
    stat_buf->st_ino = temp.st_ino;
}

int jmp___xstat(int ver, const char *_path, struct linux_stat *stat_buf)
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

    struct stat temp;
    int ret = stat(path, &temp);
    if (ret < 0)
        return ret;

    stat_msys_to_linux(temp, stat_buf);

    return 0;
}

int jmp___xstat64(int ver, const char *_path, struct linux_stat64 *stat_buf)
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
    struct stat temp;
    int ret = stat(path, &temp);
    if (ret < 0)
        return ret;

    stat64_msys_to_linux(temp, stat_buf);
    return 0;
}

int jmp___fxstat(int ver, int fd, struct linux_stat *stat_buf)
{
    struct stat temp;
    int ret = fstat(fd, &temp);
    if (ret < 0)
        return ret;

    stat_msys_to_linux(temp, stat_buf);
    return 0;
}

int jmp___fxstat64(int ver, int fd, struct linux_stat64 *stat_buf)
{
    struct stat temp;
    int ret = fstat(fd, &temp);
    if (ret < 0)
        return ret;

    stat64_msys_to_linux(temp, stat_buf);
    return 0;
}

int jmp___lxstat(int ver, const char *_path, struct linux_stat *stat_buf)
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
    struct stat temp;
    int ret = lstat(path, &temp);
    if (ret < 0)
        return ret;

    stat_msys_to_linux(temp, stat_buf);
    return 0;
}

int jmp___lxstat64(int ver, const char *_path, struct linux_stat64 *stat_buf)
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
    struct stat temp;
    int ret = lstat(path, &temp);
    if (ret < 0)
        return ret;

    stat64_msys_to_linux(temp, stat_buf);
    return 0;
}

int jmp_fstat(int fd, struct linux_stat *stat_buf)
{
    struct stat temp;
    int ret = fstat(fd, &temp);
    if (ret < 0)
        return ret;

    stat_msys_to_linux(temp, stat_buf);
    return 0;
}

int jmp_statvfs64(const char *path, void *buf)
{
    printf("statvfs64 stub called\n");
    errno = ENOSYS;
    return -1;
}

Stubs GetStatStubs()
{
    return {
        DEF_STUB(__xstat),
        DEF_STUB(__xstat64),
        DEF_STUB(__fxstat),
        DEF_STUB(__fxstat64),
        DEF_STUB(__lxstat),
        DEF_STUB(__lxstat64),
        DEF_STUB(fstat),
        DEF_STUB(statvfs64),
    };
}