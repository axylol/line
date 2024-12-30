#pragma once
#include <cstdint>
#include <elfio/elfio.hpp>
#include "symbol.h"

struct Import
{
    char name[1024];
};

struct Relocation
{
    ELFIO::Elf64_Addr offset;
    char symbol[1024];
    ELFIO::Elf_Word type;
    ELFIO::Elf_Sxword addend;
    ELFIO::Elf_Sxword calc_value;
    ELFIO::Elf64_Addr symbol_value;

    ELFIO::Elf_Word symbol_word;

    // Our values
    Symbol* symbol_ptr;
};

class Module
{
private:
    char name[1024];
    char path[1024];

    uintptr_t base_address;
    size_t base_size = 0;

    // The .so file base address
    uintptr_t real_base_address = UINTPTR_MAX;

    bool is_shared_lib = false;
    uintptr_t init_virtual_address = 0;

    uintptr_t entry_address = 0;

    uintptr_t init_array_virtual_address = 0;
    uintptr_t init_array_size = 0;

    uintptr_t fde_table_address = 0;
    
    uintptr_t text_address = 0;

    uintptr_t load_address = UINTPTR_MAX;
    uint32_t load_size;

    std::vector<Import> imports = {};
    std::vector<Symbol> symbols = {};
    std::vector<Relocation> relocations = {};
    std::vector<Symbol> symbols_exported = {};

    bool failed = false;
public:
    Module(uint8_t *data, size_t data_size, const char *name);

    void Parse(uint8_t *data, size_t data_size);

    void CallInit(size_t args, void *argp, void *param);
    void CallEntry();

    const char *GetName()
    {
        return this->name;
    }

    uintptr_t GetBaseAddress()
    {
        return this->base_address;
    }

    std::vector<Import> GetImports()
    {
        return this->imports;
    }

    std::vector<Symbol> GetSymbols()
    {
        return this->symbols;
    }

    std::vector<Relocation> GetRelocations()
    {
        return this->relocations;
    }

    bool IsSharedLib()
    {
        return this->is_shared_lib;
    }

    void MemoryPatch(uintptr_t dst, uintptr_t src, size_t length);
    void MemorySet(uintptr_t dst, uint8_t val, size_t length);

    void RegisterFrames();

    SymbolResolver LookupSymbol(Symbol* symbol);
    SymbolResolver LookupSymbol(const char* name);

    bool DidFail() {
        return this->failed;
    }

    uintptr_t GetLoadAddress() {
        return this->load_address;
    }

    uint32_t GetLoadSize() {
        return this->load_size;
    }
};