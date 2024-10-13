#pragma once
#include <vector>

struct Stub {
    const char* name;
    void* function;
};

typedef std::vector<Stub> Stubs;

#define DEF_STUB(name) {#name, (void *)jmp_##name}