#include "module.h"
#include <string.h>
#include <iostream>
#include <sstream>
#include <inttypes.h>
#include "config.h"
#include "linker.h"
#include <elfio/elfio_dump.hpp>
#include "windows.h"

Module::Module(uint8_t *data, size_t data_size, const char *name)
{
    strcpy(this->name, name);
    strcpy(this->path, name);

    this->Parse(data, data_size);
}

void Module::Parse(uint8_t *data, size_t data_size)
{
    using namespace ELFIO;
    elfio reader;

    printf("[Module][%s] Loading\n", this->name);

    std::istringstream stream(std::string((char *)data, data_size));
    if (!reader.load(stream))
    {
        printf("[%s] Failed to parse elf\n", this->name);
        this->failed = true;
        return;
    }

    /*std::string dump_file_name = "dump/" + std::string(this->name) + ".txt";
    std::ofstream dump_file(dump_file_name);

    dump::header(dump_file, reader);
    dump::header(dump_file, reader);
    dump::section_headers(dump_file, reader);
    dump::segment_headers(dump_file, reader);
    dump::symbol_tables(dump_file, reader);*/

    this->is_shared_lib = reader.get_type() == ET_DYN;

    this->entry_address = reader.get_entry();

    for (size_t z = 0; z < reader.segments.size(); z++)
    {
        auto seg = reader.segments[z];

        switch (seg->get_type())
        {
        case PT_LOAD:
        case PT_GNU_RELRO:
        {
            if (seg->get_memory_size() < 1)
                break;

            size_t aligned_size = seg->get_align() != 0 ? (seg->get_memory_size() + (seg->get_align() - 1)) & ~(seg->get_align() - 1)
                                                        : seg->get_memory_size();

            size_t last_addr = seg->get_virtual_address() + aligned_size;
            if (last_addr > this->base_size)
                this->base_size = last_addr;

            if (seg->get_virtual_address() < this->real_base_address)
                this->real_base_address = seg->get_virtual_address();
            break;
        }
        case PT_TLS:
        {
            size_t aligned_size = seg->get_align() != 0 ? (seg->get_memory_size() + (seg->get_align() - 1)) & ~(seg->get_align() - 1)
                                                        : seg->get_memory_size();

            printf("[Module][%s] TLS IS NOT IMPLEMENTED YET mem size = %d file size = %d\n", this->name, aligned_size, seg->get_file_size());

            break;
        }
        }
    }

    if (this->real_base_address)
    {
#ifdef ALLOC_LATER
        void *res = WindowsVirtualAlloc((void *)this->real_base_address, this->base_size + (MEMORY_ALIGN * 2), WINDOWS_MEM_COMMIT, WINDOWS_PAGE_EXECUTE_READWRITE);
        if (!res)
        {
            printf("[Module][%s] cant allocate, error=%d\n", this->name, GetLastError());
            this->failed = true;
            return;
        }
#endif

        this->base_address = 0;
    }
    else
    {
        // printf("Allocating %d mb\n", this->base_size / 1000000);
        this->base_address = (uintptr_t)WindowsVirtualAlloc(NULL, this->base_size + (MEMORY_ALIGN * 2), WINDOWS_MEM_COMMIT | WINDOWS_MEM_RESERVE, WINDOWS_PAGE_EXECUTE_READWRITE);
        if (!this->base_address)
        {
            printf("[Module][%s] cant allocate, error=%d\n", this->name, WindowsGetLastError());
            this->failed = true;
            return;
        }
        while ((this->base_address % MEMORY_ALIGN) != 0)
            this->base_address++;
        // this->base_address -= this->real_base_address;
    }

    printf("[Module][%s] base_address is %" PRIxPTR "\n", this->name, this->base_address);

    printf("[Module][%s] mapping to memory\n", this->name);

    for (size_t z = 0; z < reader.segments.size(); z++)
    {
        auto seg = reader.segments[z];

        switch (seg->get_type())
        {
        case PT_LOAD:
        case PT_GNU_RELRO:
        {
            if (seg->get_memory_size() < 1)
                break;

            uintptr_t address = this->base_address + seg->get_virtual_address();
            
            if (address < load_address) {
                load_address = address;
                load_size = seg->get_memory_size();
            }

            this->MemorySet(address, 0, seg->get_memory_size());
            this->MemoryPatch(address, (uintptr_t)seg->get_data(), seg->get_file_size());
            break;
        }
        }
    }

    for (size_t z = 0; z < reader.sections.size(); z++)
    {
        auto sec = reader.sections[z];

        std::string section_name = sec->get_name();
        // printf("[%s] section %s, type=%d\n", this->name, section_name.c_str(), sec->get_type());

        switch (sec->get_type())
        {
        case SHT_DYNAMIC:
        {
            // printf("[%s] dynamic\n", this->name);

            const dynamic_section_accessor dynamic(reader, sec);
            for (size_t i = 0; i < dynamic.get_entries_num(); i++)
            {
                Elf_Xword dynamic_tag;
                Elf_Xword dynamic_value;
                std::string dynamic_name;

                dynamic.get_entry(i, dynamic_tag, dynamic_value, dynamic_name);

                switch (dynamic_tag)
                {
                case DT_NEEDED:
                {
                    Import imp{0};
                    strcpy(imp.name, dynamic_name.c_str());
                    this->imports.push_back(imp);
                    break;
                }
                case DT_INIT:
                {
                    this->init_virtual_address = dynamic_value;
                    break;
                }
                case DT_INIT_ARRAY: {
                    this->init_array_virtual_address = dynamic_value;
                    break;
                }
                case DT_INIT_ARRAYSZ: {
                    this->init_array_size = dynamic_value;
                    break;
                }
                }

                // printf("[%s] add module import %s\n", this->name, dynamic_name.c_str());
            }
            break;
        }
        case SHT_DYNSYM:
        {
            // printf("[%s] exported symbols\n", this->name);

            const symbol_section_accessor symbols(reader, sec);
            for (size_t i = 0; i < symbols.get_symbols_num(); i++)
            {
                std::string symbol_name;
                Elf64_Addr symbol_value;
                Elf_Xword symbol_size;
                uint8_t symbol_bind;
                uint8_t symbol_type;
                Elf_Half symbol_section_index;
                uint8_t symbol_other;

                symbols.get_symbol(i, symbol_name, symbol_value, symbol_size, symbol_bind, symbol_type, symbol_section_index, symbol_other);

                // printf("[%s] add symbol export %s\n", this->name, symbol_name.c_str());
                // if (symbol_bind != STB_GLOBAL && symbol_bind != STB_WEAK)
                //     continue;

                Symbol sym{0};
                strcpy(sym.name, symbol_name.c_str());
                sym.value = symbol_value;
                sym.size = symbol_size;
                sym.bind = symbol_bind;
                sym.type = symbol_type;
                sym.section_index = symbol_section_index;
                sym.other = symbol_other;

                if ((symbol_bind != STB_GLOBAL && symbol_bind != STB_WEAK) ||
                    (symbol_type != STT_FUNC && symbol_type != STT_OBJECT))
                {
                    sym.junk = true;
                }

                sym.address = this->base_address + symbol_value;

                this->symbols.push_back(sym);
            }

            break;
        }
        case SHT_REL:
        {
            // printf("relocation table name %s\n", sec->get_name().c_str());
            const relocation_section_accessor relocations(reader, sec);
            for (size_t i = 0; i < relocations.get_entries_num(); i++)
            {
                Elf64_Addr rec_offset;
                Elf64_Addr rec_symbol_value;
                std::string rec_symbol_name;
                Elf_Word rec_type;
                Elf_Sxword rec_addend;
                Elf_Sxword rec_calc_value;

                Elf_Word rec_symbol_word;

                relocations.get_entry(i, rec_offset, rec_symbol_value, rec_symbol_name, rec_type, rec_addend, rec_calc_value);

                relocations.get_entry(i, rec_offset, rec_symbol_word, rec_type, rec_addend);

                // printf("[%s] relocation n=%s t=%d o=%p\n", this->name, rec_symbol_name.c_str(), rec_type, rec_offset);

                Relocation reloc{0};
                reloc.offset = rec_offset;
                strcpy(reloc.symbol, rec_symbol_name.c_str());
                reloc.type = rec_type;
                reloc.addend = rec_addend;
                reloc.calc_value = rec_calc_value;
                reloc.symbol_value = rec_symbol_value;
                reloc.symbol_word = rec_symbol_word;

                this->relocations.push_back(reloc);
            }
            break;
        }
        default:
        {
            // printf("unhandled section %s %d\n", sec->get_name().c_str(), sec->get_type());
            break;
        }
        }
    }

    for (size_t i = 0; i < this->relocations.size(); i++)
        this->relocations[i].symbol_ptr = &this->symbols[this->relocations[i].symbol_word];

    printf("[Module][%s] imports=", this->name);
    for (size_t i = 0; i < this->imports.size(); i++)
    {
        printf("%s,", this->imports[i].name);
    }
    printf("\n");
}

void Module::CallInit(size_t args, void *argp, void *param)
{
    // printf("base=%p\n init address = %p\nPress enter to execute init\n", this->base_address, this->base_address + this->init_virtual_address);
    // std::cin.get();

    
    using EntryFunc = int (*)();
    if (this->init_virtual_address != 0)
        ((EntryFunc)(this->base_address + this->init_virtual_address))();

    if (this->init_array_virtual_address != 0) {
        printf("[Module][%s] init array size=%d\n", this->name, this->init_array_size);

        int size = this->init_array_size / sizeof(uintptr_t);
        uintptr_t init_array = this->base_address + this->init_array_virtual_address;
        for (int i = 0; i < size; i++) {
            uintptr_t init_address = *(uintptr_t*)init_array;
            ((EntryFunc)init_address)();
            init_array += sizeof(uintptr_t);
        }
    }
}

void Module::CallEntry()
{
    uintptr_t entry_point = this->base_address + this->entry_address;
    printf("entrypoint=%" PRIxPTR "\n", entry_point);

    // std::cin.get();

    void(__cdecl * main_func)(size_t a1, void *a2);
    *(void **)&main_func = (void *)entry_point;

    main_func(0, NULL);
}

void Module::MemoryPatch(uintptr_t dst, uintptr_t src, size_t length)
{
    if (dst < this->base_address || dst + length >= this->base_address + this->base_size)
    {
        printf("[Module][%s] MemoryPatch out of bounds\n", this->name);
        throw 1;
    }
    memcpy((void *)dst, (void *)src, length);
}

void Module::MemorySet(uintptr_t dst, uint8_t val, size_t length)
{
    if (dst < this->base_address || dst + length >= this->base_address + this->base_size)
    {
        printf("[Module][%s] MemorySet out of bounds\n", this->name);
        throw 1;
    }
    memset((void *)dst, val, length);
}

SymbolResolver Module::LookupSymbol(Symbol *symbol)
{
    if (!symbol->value || symbol->bind == ELFIO::STB_WEAK)
        return g_linker->LookupGlobalSymbol(symbol->name);
    return SymbolResolver(*symbol);
}

SymbolResolver Module::LookupSymbol(const char *name)
{
    for (size_t i = 0; i < this->symbols.size(); i++)
    {
        // if (this->symbols[i].junk)
        //     continue;
        auto sym = &this->symbols[i];
        if (strcmp(sym->name, name) == 0)
        {
            if (sym->bind != ELFIO::STB_GLOBAL && sym->bind != ELFIO::STB_WEAK && sym->bind != ELFIO::STB_LOOS)
                continue;
            if (!sym->value)
                continue;
            if (sym->section_index == 0)
                continue;
            return SymbolResolver(*sym);
        }
    }
    return SymbolResolver();
}