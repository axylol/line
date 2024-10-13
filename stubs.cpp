#include "stubs.h"
#include "stubs/stub.h"
#include <unordered_map>
#include <vector>
#include <dlfcn.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "windows.h"

#include "stubs/dirent.h"
#include "stubs/dlfcn.h"
#include "stubs/epoll.h"
#include "stubs/fcntl.h"
#include "stubs/ioctl.h"
#include "stubs/libc.h"
#include "stubs/mqueue.h"
#include "stubs/netdb.h"
#include "stubs/poll.h"
#include "stubs/pthread.h"
#include "stubs/select.h"
#include "stubs/semaphore.h"
#include "stubs/socket.h"
#include "stubs/stat.h"
#include "stubs/stdio.h"
#include "stubs/termios.h"
#include "stubs/time.h"
#include "stubs/unistd.h"
#include "stubs/wchar.h"

std::unordered_map<const char *, void *> STUB_FUNCS = {};

void *MSYS_DLL = NULL;

uintptr_t ResolveMsys(const char *name)
{
    uintptr_t address = (uintptr_t)dlsym(MSYS_DLL, name);
    if (address)
        return address;
    return 0;
}

uintptr_t
ResolveStub(const char *name)
{
    for (auto &it : STUB_FUNCS)
    {
        if (strcmp(it.first, name) == 0)
            return (uintptr_t)it.second;
    }
    return ResolveMsys(name);
}

#define ADD_STUBS(func)                                                    \
    {                                                                      \
        Stubs stubs = func;                                                \
        for (size_t i = 0; i < stubs.size(); i++)                          \
        {                                                                  \
            STUB_FUNCS.insert_or_assign(stubs[i].name, stubs[i].function); \
        }                                                                  \
    }

void StubsInit()
{
    MSYS_DLL = WindowsLoadLibraryA("msys-2.0.dll");
    if (!MSYS_DLL)
        printf("[Stubs] Error loading msys-2.0.dll\n");

    ADD_STUBS(GetDirentStubs());
    ADD_STUBS(GetDlfcnStubs());
    ADD_STUBS(GetEpollStubs());
    ADD_STUBS(GetFcntlStubs());
    ADD_STUBS(GetIoctlStubs());
    ADD_STUBS(GetLibcStubs());
    ADD_STUBS(GetMqueueStubs());
    ADD_STUBS(GetNetdbStubs());
    ADD_STUBS(GetPollStubs());
    ADD_STUBS(GetPthreadStubs());
    ADD_STUBS(GetSelectStubs());
    ADD_STUBS(GetSemaphoreStubs());
    ADD_STUBS(GetSocketStubs());
    ADD_STUBS(GetStatStubs());
    ADD_STUBS(GetStdioStubs());
    ADD_STUBS(GetTermiosStubs());
    ADD_STUBS(GetTimeStubs());
    ADD_STUBS(GetUnistdStubs());
    ADD_STUBS(GetWcharStubs());
}