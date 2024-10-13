#include "plugin.h"
#include "linker.h"
#include <dlfcn.h>
#include "windows.h"
#include "stubs.h"

void *Plugin_DlOpen(const char *name)
{
    return g_linker->DlOpen(name);
}

void *Plugin_DlSym(void *handle, const char *name)
{
    return g_linker->DlSym(handle, name);
}

void Plugin_Hook(void *target, void *detour, void **original) {
    WindowsMH_CreateHook(target, detour, original);
    WindowsMH_EnableHook(target);
}

void* Plugin_GetModuleByName(const char* name) {
    // TODO: IMPLEMENT
    return NULL;
}

void* Plugin_GetModuleStart(Module* module) {
    return (void*)module->GetLoadAddress();
}

uint32_t Plugin_GetModuleSize(Module* module) {
    return module->GetLoadSize();
}

Module* Plugin_GetModuleByHandle(void* handle) {
    return g_linker->GetModuleByBaseAddress((uintptr_t)handle);
}

void* Plugin_ResolveStub(const char* name) {
    return (void*)ResolveStub(name);
}

void *func_table[] = {
    (void *)Plugin_DlOpen,
    (void *)Plugin_DlSym,
    
    (void *)Plugin_Hook,
    
    (void *)Plugin_GetModuleByName,
    (void *)Plugin_GetModuleStart,
    (void *)Plugin_GetModuleSize,
    (void *)Plugin_GetModuleByHandle,

    (void*)Plugin_ResolveStub
    };

void Plugin::Initialize(const char *name)
{
    this->hModule = (void *)dlopen(name, RTLD_NOW);
    if (!this->hModule)
    {
        printf("[Plugin] Failed initializing plugin %s\n", name);
        return;
    }

    *(void **)&this->func_OnInitialize = dlsym(this->hModule, "OnInitialize");
    *(void **)&this->func_OnPreExecute = dlsym(this->hModule, "OnPreExecute");
    *(void **)&this->func_OnDlOpen = dlsym(this->hModule, "OnDlOpen");
    *(void **)&this->func_OnDlSym = dlsym(this->hModule, "OnDlSym");
    *(void **)&this->func_OnResolveSymbol = dlsym(this->hModule, "OnResolveSymbol");

    this->func_OnInitialize(1, func_table);
}

void Plugin::OnPreExecute(const char *lib_name, void *base_address)
{
    if (!this->func_OnPreExecute)
        return;
    this->func_OnPreExecute(lib_name, base_address);
}

// Returns true means it will block other plugins from receiving the event and set the return value to result
// Returns false to skip

bool Plugin::OnDlOpen(const char *lib_name, void **result)
{
    if (!this->func_OnDlOpen)
        return false;
    return this->func_OnDlOpen(lib_name, result);
}

bool Plugin::OnDlSym(void *handle, const char *symbol, void **result)
{
    if (!this->func_OnDlSym)
        return false;
    return this->func_OnDlSym(handle, symbol, result);
}

bool Plugin::OnResolveSymbol(const char *symbol, void **result)
{
    if (!this->func_OnResolveSymbol)
        return false;
    return this->func_OnResolveSymbol(symbol, result);
}

Plugin *g_plugin;