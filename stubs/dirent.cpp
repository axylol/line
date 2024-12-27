#include "dirent.h"
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "../config.h"
#include "../whitelist.h"

typedef int32_t linux_ino_t;
typedef int32_t linux_off_t;
typedef int32_t linux_time_t;
typedef uint32_t linux_syscall_ulong_t;
typedef int32_t linux_blksize_t;
typedef int32_t linux_blkcnt_t;
typedef int32_t linux_dev_t;
typedef int32_t linux_mode_t;
typedef int32_t linux_nlink_t;
typedef int32_t linux_uid_t;
typedef int32_t linux_gid_t;

#pragma pack(push, 1)
struct linux_dirent
{
    linux_ino_t d_ino;
    linux_off_t d_off;
    uint16_t d_reclen;
    uint8_t d_type;
    char d_name[256];
    char pad[1];
};
#pragma pack(pop)

static_assert(sizeof(linux_dirent) == 268, "size mismatch");

DIR *jmp_opendir(const char *_path)
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
        return NULL;
    }
#endif
    return opendir(path);
}

struct linux_dirent *jmp_readdir(DIR *dir)
{
    struct dirent *res = readdir(dir);
    if (!res)
    {
        // TODO: wmmt4 doesnt check for null if its a null return value..
        errno = EBADF;
        return NULL;
    }

    struct linux_dirent temp;
    temp.d_ino = res->d_ino;
    temp.d_off = 0;
    temp.d_reclen = 0;
    temp.d_type = res->d_type;
    memcpy(temp.d_name, res->d_name, 256);

    memcpy(res, &temp, sizeof(temp));
    return (struct linux_dirent *)res;
}

// TODO: filter, compar support
int jmp_scandir(const char *_dirp, struct linux_dirent ***namelist, void *filter, void *compar)
{
#ifdef ENV_REDIRECTION
    char dirp[PATH_MAX];
    fixPathIfNeeded(dirp, _dirp);
#else
    const char *dirp = _dirp;
#endif
#ifdef AGGRESIVE_FILE_CHECK
    if (!isPathWhitelisted(dirp))
    {
        errno = EACCES;
        return -1;
    }
#endif

    printf("scandir %s\n", dirp);

    DIR *dir = opendir(dirp);
    if (!dir)
        return -1;

    std::vector<struct linux_dirent> dires = {};
    while (true)
    {
        struct dirent *dire = readdir(dir);
        if (!dire)
            break;

        struct linux_dirent temp;
        temp.d_ino = dire->d_ino;
        temp.d_off = 0;
        temp.d_reclen = 0;
        temp.d_type = dire->d_type;
        memcpy(temp.d_name, dire->d_name, 256);

        dires.push_back(temp);
    }

    closedir(dir);

    struct linux_dirent **names = (struct linux_dirent **)malloc(sizeof(struct linux_dirent *) * dires.size());
    for (size_t i = 0; i < dires.size(); i++)
    {
        struct linux_dirent *dire = (struct linux_dirent *)malloc(sizeof(struct linux_dirent));
        memcpy(dire, &dires[i], sizeof(struct linux_dirent));

        names[i] = dire;
    }
    *namelist = names;

    errno = 0;
    return 0;
}

void jmp_readdir64()
{
    printf("readdir64 stub\n");
}

void jmp_readdir64_r()
{
    printf("readdir64_r stub\n");
}

int jmp_rmdir(const char *_path)
{
#ifdef ENV_REDIRECTION
    char path[PATH_MAX];
    fixPathIfNeeded(path, _path);
#else
    const char *path = _path;
#endif
    if (!isPathWhitelisted(path))
    {
        errno = EACCES;
        return -1;
    }
    return rmdir(path);
}

int jmp_mkdir(const char *_pathname, mode_t mode)
{
#ifdef ENV_REDIRECTION
    char pathname[PATH_MAX];
    fixPathIfNeeded(pathname, _pathname);
#else
    const char *pathname = _pathname;
#endif
#ifdef AGGRESIVE_FILE_CHECK
    if (!isPathWhitelisted(pathname))
    {
        errno = EACCES;
        return -1;
    }
#endif
    return mkdir(pathname, mode);
}

Stubs GetDirentStubs()
{
    return {
        DEF_STUB(opendir),
        DEF_STUB(readdir),
        DEF_STUB(scandir),
        DEF_STUB(readdir64),
        DEF_STUB(readdir64_r),
        DEF_STUB(rmdir),
        DEF_STUB(mkdir)};
}