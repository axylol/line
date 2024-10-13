#pragma once
#include <cstdint>
#include <elfio/elfio.hpp>

struct Symbol
{
    char name[1024];
    ELFIO::Elf64_Addr value;
    ELFIO::Elf_Xword size;
    uint8_t bind;
    uint8_t type;
    ELFIO::Elf_Half section_index;
    uint8_t other;

    bool junk;
    
    uintptr_t address;
};

struct SymbolResolver
{
    bool found;
    uintptr_t address;
    uint8_t bind;
    size_t size;
    uint8_t type;

    SymbolResolver()
    {
        this->found = false;
    }

    SymbolResolver(uintptr_t address, uint8_t bind)
    {
        this->found = true;
        this->address = address;
        this->bind = bind;
        this->size = 0;
        this->type = 0;
    }

    SymbolResolver(uintptr_t address, uint8_t bind, size_t size)
    {
        this->found = true;
        this->address = address;
        this->bind = bind;
        this->size = size;
        this->type = 0;
    }

    SymbolResolver(Symbol symbol) {
        this->found = true;
        this->address = symbol.address;
        this->bind = symbol.bind;
        this->size = symbol.size;
        this->type = symbol.type;
    }


};