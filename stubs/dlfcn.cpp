#include "dlfcn.h"
#include "../linker.h"
#include "../plugin.h"
#include "../config.h"

void *jmp_dlopen(const char *filename, int flags)
{
    void *result = NULL;
    if (g_plugin->OnDlOpen(filename, &result))
        return result;
#ifdef DLOPEN
#ifdef AGGRESIVE_FILE_CHECK
    if (!isPathWhitelisted(from))
        return NULL;
#endif
    return g_linker->DlOpen(filename);
#else
    return NULL;
#endif
}

void *jmp_dlsym(void *handle, const char *symbol)
{
    void *result = NULL;
    if (g_plugin->OnDlSym(handle, symbol, &result))
        return result;
    return g_linker->DlSym(handle, symbol);
}

char *jmp_dlerror()
{
    return NULL;
}

int jmp_dlclose(void *handle)
{
    return 0;
}

Stubs GetDlfcnStubs()
{
    return {
        DEF_STUB(dlopen),
        DEF_STUB(dlsym),
        DEF_STUB(dlerror),
        DEF_STUB(dlclose)};
}