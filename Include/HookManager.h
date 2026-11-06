#pragma once
#include "framework.h"

class HookManager
{
public:
    static HookManager& Instance()
    {
        static HookManager instance;
        return instance;
    }

    uintptr_t PatternScan(const char* moduleName, const char* pattern, const char* mask);
    uintptr_t PatternScan(uintptr_t base, size_t size, const char* pattern, const char* mask);
    bool PatchMemory(uintptr_t address, const void* data, size_t size);
    bool NopMemory(uintptr_t address, size_t count);
    bool CreateJmpHook(uintptr_t source, uintptr_t destination, size_t length);

    // Detour helper
    bool AttachHook(PVOID* ppPointer, PVOID pDetour);
    bool DetachHook(PVOID* ppPointer, PVOID pDetour);
};

