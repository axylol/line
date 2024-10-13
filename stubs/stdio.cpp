#include "stdio.h"
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include "../config.h"
#include "../whitelist.h"

FILE *jmp_fopen(const char *_filename, const char *mode)
{
#ifdef ENV_REDIRECTION
    char filename[PATH_MAX];
    fixPathIfNeeded(filename, _filename);
#else
    const char *filename = _filename;
#endif

#ifdef AGGRESIVE_FILE_CHECK
    if (!isPathWhitelisted(filename))
    {
        errno = EACCES;
        return NULL;
    }
#endif
    return fopen(filename, mode);
}

FILE *jmp_fopen64(const char *filename, const char *mode)
{
    return jmp_fopen(filename, mode);
}

FILE *jmp_freopen(const char *_pathname, const char *mode, FILE *stream)
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
        return NULL;
    }
#endif
    return freopen(pathname, mode, stream);
}

int jmp_fseeko64(FILE *fp, int64_t offset, int whence)
{
    return fseeko(fp, offset, whence);
}

int64_t jmp_ftello64(FILE *fp)
{
    return ftell(fp);
}

int jmp_remove(const char *_name)
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
    return remove(name);
}

Stubs GetStdioStubs()
{
    return {
        DEF_STUB(fopen),
        DEF_STUB(fopen64),
        DEF_STUB(freopen),

        DEF_STUB(fseeko64),
        DEF_STUB(ftello64),

        DEF_STUB(remove)};
}