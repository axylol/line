#include "linker.h"
#include "stubs.h"
#include <sstream>
#include "config.h"
#include <inttypes.h>
#include "plugin.h"
#include "whitelist.h"
#include "windows.h"

char *MODULE_NAME;
char *PLUGIN_NAME;

void *entrypoint(void *)
{
    WindowsMH_Initialize();
    printf("[LINE] Initialized MinHook\n");

    StubsInit();
    printf("[LINE] Initialized Stubs\n");

    InitPathWhitelist();
    printf("[LINE] Initialized Whitelist\n");

    g_plugin = new Plugin();
    g_plugin->Initialize(PLUGIN_NAME);
    printf("[LINE] Initialized Plugin\n");

    g_linker = new Linker();
    g_linker->AddModuleFromFile(MODULE_NAME);
    g_linker->Start();

    return NULL;
}

int main(int argc, char *argv[])
{
#ifndef ALLOC_LATER
    void *baseAddress = (void *)0x8048000;
    uint32_t baseSize = 0x93f5770;

    char *LINE_BASE_ADDRESS = getenv("LINE_BASE_ADDRESS");
    if (LINE_BASE_ADDRESS)
        baseAddress = (void *)strtoull(LINE_BASE_ADDRESS, NULL, 16);
    char *LINE_BASE_SIZE = getenv("LINE_BASE_SIZE");
    if (LINE_BASE_SIZE)
        baseSize = strtoul(LINE_BASE_SIZE, NULL, 16);
#endif

    char *LINE_EXECUTABLE = getenv("LINE_EXECUTABLE");
    if (LINE_EXECUTABLE)
        MODULE_NAME = strdup(LINE_EXECUTABLE);
    else
        MODULE_NAME = strdup("WMN4r");
    char *LINE_PLUGIN = getenv("LINE_PLUGIN");
    if (LINE_PLUGIN)
        PLUGIN_NAME = strdup(LINE_PLUGIN);
    else
        PLUGIN_NAME = strdup("msys-line-mt4.dll");

    if (argc >= 2)
    {
        if (strcmp(argv[1], "calculate_elf") == 0)
        {
            std::ifstream file(MODULE_NAME, std::ios::binary);
            if (!file)
                throw 1;

            file.seekg(0, std::ios::end);
            size_t file_size = file.tellg();
            file.seekg(0, std::ios::beg);

            uint8_t *data = (uint8_t *)new uint8_t[file_size];
            file.read((char *)data, file_size);

            using namespace ELFIO;
            elfio reader;

            std::istringstream stream(std::string((char *)data, file_size));
            if (!reader.load(stream))
                throw 1;

            uintptr_t real_base_address = UINTPTR_MAX;
            uintptr_t base_size = 0;
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
                    if (last_addr > base_size)
                        base_size = last_addr;

                    if (seg->get_virtual_address() < real_base_address)
                        real_base_address = seg->get_virtual_address();
                    break;
                }
                }
            }
            printf("base_address=%" PRIxPTR " base_size=%" PRIxPTR, real_base_address, base_size);

            delete[] data;
            file.close();
            return 0;
        }
        printf("invalid argument\nsupported: calculate_elf\n");
        return 0;
    }

#ifndef ALLOC_LATER
    // TODO: AUTOMATE GETTING BASE ADDRESS AND SIZE BY CALCULATE_ELF SO WE STILL GET ACCESS TO THE VIRTUAL MEMORY
    if (!WindowsVirtualAlloc(baseAddress, baseSize + (MEMORY_ALIGN * 2), WINDOWS_MEM_COMMIT | WINDOWS_MEM_RESERVE, WINDOWS_PAGE_EXECUTE_READWRITE))
    {
        printf("Failed to allocate base address\n");
        throw 1;
    }
#endif

#ifdef CUSTOM_STACK
    void *stack = memalign(MEMORY_ALIGN, CUSTOM_STACK_SIZE * 2);
    if (!stack)
    {
        printf("Error allocating stack\n");
        return 1;
    }
    memset(stack, 0, CUSTOM_STACK_SIZE * 2);

    void *stackMiddle = (void *)((uintptr_t)stack + CUSTOM_STACK_SIZE);

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstack(&attr, stackMiddle, CUSTOM_STACK_SIZE);
    pthread_t thread;
    pthread_create(&thread, &attr, entrypoint, NULL);
    pthread_join(thread, NULL);
#else
    entrypoint(NULL);
#endif
    return 0;
}