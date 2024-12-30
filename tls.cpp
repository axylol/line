#include "tls.h"
#include <unordered_map>
#include <windows.h>
#include <iostream>

// really slow and shitty tls_get_addr implementation
// it should be fine as long as we dont call it 24/7 since its designed just for exception handling

DWORD tlsSlot = 0;

#define TLS_STACK_SIZE 128

struct TlsInfo {
    std::unordered_map<uint32_t, void*> data = {};
};

void* TlsGetAddr(uint32_t module, uint32_t offset) {
    TlsInfo* info = (TlsInfo*)TlsGetValue(tlsSlot);
    if (!info) {
        info = new TlsInfo();
        TlsSetValue(tlsSlot, info);
    }

    void* stack;
    if (info->data.count(module) < 1) {
        stack = memalign(16, TLS_STACK_SIZE);
        info->data.insert_or_assign(module, stack);
    } else {
        stack = info->data[module];
    }
    return (void*)((uintptr_t)stack + offset);
}

void TlsInit() {
    tlsSlot = TlsAlloc();
    if (tlsSlot == TLS_OUT_OF_INDEXES)
        printf("[TLS] Error allocating Thread Local Storage\n");
}