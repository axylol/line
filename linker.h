#pragma once
#include "module.h"
#include <vector>

enum class RelocationStatus
{
    Success,
    Error
};
class Linker
{
private:
    std::vector<Module*> modules;

    uintptr_t address = 0x8048000;
public:
    Module* AddModule(uint8_t *data, size_t data_size, const char *name);
    Module* AddModuleFromFile(const char *file_name);

    void Start();

    SymbolResolver LookupGlobalSymbol(const char* name, uintptr_t isnt = 0);

    void* DlOpen(const char* name);
    void* DlSym(void* handle, const char* name);

    Module* GetModuleByBaseAddress(uintptr_t baseAddress);
private:
    RelocationStatus Relocate(Module *module, int i, Relocation* rel);
    void RelocateAll();
};

extern Linker* g_linker;