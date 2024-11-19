#include "linker.h"
#include "stubs.h"

#include <string.h>

#include "plugin.h"

Module *Linker::AddModule(uint8_t *data, size_t data_size, const char *name)
{
    for (size_t i = 0; i < this->modules.size(); i++)
        if (strcmp(this->modules[i]->GetName(), name) == 0)
            return this->modules[i];

    Module *module = new Module(data, data_size, name);
    if (!module->DidFail())
    {
        this->modules.push_back(module);

        printf("[Linker] Loaded module %s\n", name);

        std::vector<Import> imports = module->GetImports();
        for (size_t i = 0; i < imports.size(); i++)
            this->AddModuleFromFile(imports[i].name);

        return module;
    }
    // let it leak for now
    // delete module;
    return NULL;
}

Module *Linker::AddModuleFromFile(const char *file_name)
{
    if (!file_name)
        return NULL;

    std::vector<std::string> paths = {};
    paths.push_back(file_name);

    // :sob:
    char *LINE_LIBRARY_PATH = getenv("LINE_LIBRARY_PATH");
    if (LINE_LIBRARY_PATH)
    {
        std::string path(LINE_LIBRARY_PATH);
        path += "/";
        path += file_name;

        paths.push_back(path);
    }

    for (size_t i = 0; i < paths.size(); i++)
    {
        std::ifstream file(paths[i], std::ios::binary);
        if (!file) {
            //printf("[Linker] Can't open %s\n", paths[i].c_str());
            continue;
        }
        file.seekg(0, std::ios::end);
        size_t file_size = file.tellg();
        file.seekg(0, std::ios::beg);

        uint8_t *data = (uint8_t *)new uint8_t[file_size];
        file.read((char *)data, file_size);

        Module *ret = this->AddModule(data, file_size, file_name);

        delete[] data;
        file.close();
        return ret;
    }
    printf("[Linker] Can't find module %s\n", file_name);
    return NULL;
}

void Linker::Start()
{
    printf("[Linker] Doing relocation\n");

    this->RelocateAll();

    for (size_t i = 0; i < this->modules.size(); i++)
    {
        if (!this->modules[i]->IsSharedLib())
            continue;
        g_plugin->OnPreExecute(this->modules[i]->GetName(), (void *)this->modules[i]->GetBaseAddress());

        printf("[Linker] Executing lib %s\n", this->modules[i]->GetName());
        this->modules[i]->CallInit(0, NULL, NULL);
    }

    printf("[Linker] Executing main\n");

    for (size_t i = 0; i < this->modules.size(); i++)
    {
        if (this->modules[i]->IsSharedLib())
            continue;
        g_plugin->OnPreExecute(this->modules[i]->GetName(), (void *)this->modules[i]->GetBaseAddress());

        this->modules[i]->CallEntry();
    }
}

RelocationStatus Linker::Relocate(Module *module, int i, Relocation *rel)
{
    using namespace ELFIO;

    uintptr_t base_address = module->GetBaseAddress();
    uintptr_t rel_address = base_address + rel->offset;

    switch (rel->type)
    {
    case R_386_RELATIVE:
    {
        uintptr_t value = base_address + *(size_t *)rel_address;
        module->MemoryPatch(rel_address, (uintptr_t)&value, sizeof(value));
        return RelocationStatus::Success;
    }
    case R_386_GLOB_DAT:
    case R_386_JMP_SLOT:
    case R_386_COPY:
    case R_386_32:
    {
        if (!rel->symbol_ptr)
        {
            printf("[Relocate][%s] cant find symbol\n", module->GetName());
            return RelocationStatus::Error;
        }
        auto sym = rel->symbol_ptr;

        SymbolResolver res = SymbolResolver();

        switch (sym->bind)
        {
        case STB_LOCAL:
        {
            res = SymbolResolver(*sym);
            break;
        }
        case STB_WEAK:
        case STB_GLOBAL:
        case STB_LOOS:
        {
            res = this->LookupGlobalSymbol(sym->name, rel_address);
            break;
        }
        }

        if (!res.found)
        {
            printf("[Relocate][%s] can't find symbol to import %s\n", module->GetName(), sym->name);
            return RelocationStatus::Error;
        }

        if (res.address)
        {
            if (rel->type == R_386_32)
            {
                uintptr_t value = res.address + *(size_t *)rel_address;
                module->MemoryPatch(rel_address, (uintptr_t)&value, sizeof(value));
            }
            else if (rel->type == R_386_COPY)
            {
                module->MemoryPatch(rel_address, res.address, sym->size);
            }
            else
            {
                module->MemoryPatch(rel_address, (uintptr_t)&res.address, sizeof(res.address));
            }
            return RelocationStatus::Success;
        }
        break;
    }
    default:
    {
        printf("[Relocate][%s] unhandled type %d\n", module->GetName(), rel->type);
        break;
    }
    }

    return RelocationStatus::Error;
}

void Linker::RelocateAll()
{
    const auto do_relocation = [this](Module *module, ELFIO::Elf_Word type)
    {
        printf("[Linker] Relocating %s\n", module->GetName());

        std::vector<Relocation> relocations = module->GetRelocations();
        for (size_t i = 0; i < relocations.size(); i++)
        {
            auto rel = &relocations[i];

            if (rel->type == type)
            {
                RelocationStatus status = this->Relocate(module, i, rel);
                if (status != RelocationStatus::Success)
                {
                    printf("[Relocate][%s] relocation error\n", module->GetName());
                }
            }
        }
    };

    // idk if this is the right way to do these, but it works

    for (size_t i = 0; i < this->modules.size(); i++)
    {
        do_relocation(this->modules[i], ELFIO::R_386_RELATIVE);
    }

    for (size_t i = 0; i < this->modules.size(); i++)
    {
        do_relocation(this->modules[i], ELFIO::R_386_GLOB_DAT);
    }

    for (size_t i = 0; i < this->modules.size(); i++)
    {
        do_relocation(this->modules[i], ELFIO::R_386_JMP_SLOT);
    }

    for (size_t i = 0; i < this->modules.size(); i++)
    {
        do_relocation(this->modules[i], ELFIO::R_386_32);
    }

    for (size_t i = 0; i < this->modules.size(); i++)
    {
        do_relocation(this->modules[i], ELFIO::R_386_COPY);
    }
}

SymbolResolver Linker::LookupGlobalSymbol(const char *name, uintptr_t isnt)
{
    uintptr_t address = 0;

    void *plugin_ret;
    if (g_plugin->OnResolveSymbol(name, &plugin_ret))
        return SymbolResolver((uintptr_t)plugin_ret, ELFIO::STB_GLOBAL);

    address = ResolveStub(name);
    if (address)
        return SymbolResolver(address, ELFIO::STB_GLOBAL);

    // i kinda dont know what im doing

    SymbolResolver weak_result = SymbolResolver();
    for (size_t i = 0; i < this->modules.size(); i++)
    {
        auto res = this->modules[i]->LookupSymbol(name);
        if (res.found)
        {
            if (isnt && res.address == isnt)
                continue;

            if (res.bind == ELFIO::STB_GLOBAL)
                return res;

            if (res.bind == ELFIO::STB_WEAK && !weak_result.found)
                return res; // weak_result = res;

            if (res.bind == ELFIO::STB_LOOS)
                return res;
        }
    }
    return weak_result;
}

void *Linker::DlOpen(const char *name)
{
    if (!name)
        return NULL;
    for (size_t i = 0; i < this->modules.size(); i++)
        if (strcmp(this->modules[i]->GetName(), name) == 0)
            return (void *)this->modules[i]->GetBaseAddress();

    Module *module = this->AddModuleFromFile(name);
    if (!module)
        return NULL;

    printf("[Linker] Relocating %s\n", module->GetName());

    std::vector<Relocation> relocations = module->GetRelocations();
    for (size_t i = 0; i < relocations.size(); i++)
    {
        auto rel = &relocations[i];

        RelocationStatus status = this->Relocate(module, i, rel);
        if (status != RelocationStatus::Success)
            printf("[Relocate][%s] relocation error\n", module->GetName());
    }

    if (module->IsSharedLib())
        module->CallInit(0, NULL, NULL);

    return (void *)module->GetBaseAddress();
}

void *Linker::DlSym(void *handle, const char *name)
{
    if (!name)
        return NULL;
    for (size_t i = 0; i < this->modules.size(); i++)
    {
        if (this->modules[i]->GetBaseAddress() == (uintptr_t)handle)
        {
            auto res = this->modules[i]->LookupSymbol(name);
            if (!res.found)
                return NULL;
            return (void *)res.address;
        }
    }
    return NULL;
}

Module *Linker::GetModuleByBaseAddress(uintptr_t baseAddress)
{
    for (size_t i = 0; i < this->modules.size(); i++)
        if (this->modules[i]->GetBaseAddress() == baseAddress)
            return this->modules[i];
    return NULL;
}

Linker *g_linker;