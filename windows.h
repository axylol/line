#pragma once
#include <stdint.h>
#include <stddef.h>

#define WINDOWS_PAGE_EXECUTE_READWRITE 0x40
#define WINDOWS_MEM_COMMIT 0x1000
#define WINDOWS_MEM_RESERVE 0x2000

int WindowsGetLastError();

void *WindowsVirtualAlloc(void *address, size_t size, int allocationType, int protect);

int WindowsMH_Initialize();
int WindowsMH_CreateHook(void *target, void *detour, void **original);
int WindowsMH_EnableHook(void *target);

void *WindowsLoadLibraryA(const char *name);