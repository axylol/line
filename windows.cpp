#include "windows.h"
#include <windows.h>
#include <MinHook.h>

/**
 * DO NOT INCLUDE <windows.h> ANYWHERE OTHER THAN windows.cpp
 */

int WindowsGetLastError()
{
    return GetLastError();
}

void *WindowsVirtualAlloc(void *address, size_t size, int allocationType, int protect)
{
    return VirtualAlloc(address, size, allocationType, protect);
}

int WindowsMH_Initialize()
{
    return MH_Initialize();
}

int WindowsMH_CreateHook(void *target, void *detour, void **original)
{
    return MH_CreateHook(target, detour, original);
}

int WindowsMH_EnableHook(void *target)
{
    return MH_EnableHook(target);
}

void *WindowsLoadLibraryA(const char *name)
{
    return LoadLibraryA(name);
}