#pragma once

class Plugin
{
private:
    void *hModule;

    void (__cdecl* func_OnInitialize)(int, void**) = 0;
    void (__cdecl* func_OnPreExecute)(const char*, void*) = 0;
    bool (__cdecl* func_OnDlOpen)(const char*, void**) = 0;
    bool (__cdecl* func_OnDlSym)(void*, const char*, void**) = 0;
    bool (__cdecl* func_OnResolveSymbol)(const char*, void**) = 0;
public:
    void Initialize(const char *name);
    void OnPreExecute(const char *lib_name, void *base_address);

    bool OnDlOpen(const char *lib_name, void **result);
    bool OnDlSym(void *handle, const char *symbol, void **result);
    bool OnResolveSymbol(const char *symbol, void **result);
};

extern Plugin *g_plugin;