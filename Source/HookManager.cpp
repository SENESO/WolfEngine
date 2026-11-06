#include "pch.h"
#include "HookManager.h"

uintptr_t HookManager::PatternScan(const char* moduleName, const char* pattern, const char* mask)
{
    HMODULE hMod = moduleName ? GetModuleHandleA(moduleName) : GetModuleHandleA(NULL);
    if (!hMod)
    {
        if (moduleName)
            hMod = LoadLibraryA(moduleName);
        if (!hMod) return 0;
    }

    MODULEINFO modInfo = { 0 };
    if (!K32GetModuleInformation(GetCurrentProcess(), hMod, &modInfo, sizeof(modInfo)))
        return 0;

    return PatternScan((uintptr_t)modInfo.lpBaseOfDll, modInfo.SizeOfImage, pattern, mask);
}

uintptr_t HookManager::PatternScan(uintptr_t base, size_t size, const char* pattern, const char* mask)
{
    size_t patternLength = strlen(mask);
    const unsigned char* pBase = (const unsigned char*)base;
    const unsigned char* pPattern = (const unsigned char*)pattern;

    for (size_t i = 0; i <= size - patternLength; ++i)
    {
        bool found = true;
        for (size_t j = 0; j < patternLength; ++j)
        {
            if (mask[j] != '?' && pPattern[j] != pBase[i + j])
            {
                found = false;
                break;
            }
        }
        if (found)
            return base + i;
    }
    return 0;
}

bool HookManager::PatchMemory(uintptr_t address, const void* data, size_t size)
{
    DWORD oldProtect = 0;
    if (!VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READWRITE, &oldProtect))
        return false;

    memcpy((void*)address, data, size);
    VirtualProtect((LPVOID)address, size, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), (LPCVOID)address, size);
    return true;
}

bool HookManager::NopMemory(uintptr_t address, size_t count)
{
    std::vector<uint8_t> nops(count, 0x90);
    return PatchMemory(address, nops.data(), count);
}

bool HookManager::CreateJmpHook(uintptr_t source, uintptr_t destination, size_t length)
{
    if (length < 5) return false;

    DWORD oldProtect = 0;
    if (!VirtualProtect((LPVOID)source, length, PAGE_EXECUTE_READWRITE, &oldProtect))
        return false;

    uintptr_t relativeOffset = destination - source - 5;
    *(uint8_t*)source = 0xE9; // JMP opcode
    *(uint32_t*)(source + 1) = (uint32_t)relativeOffset;

    for (size_t i = 5; i < length; ++i)
        *(uint8_t*)(source + i) = 0x90; // NOP padding

    VirtualProtect((LPVOID)source, length, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), (LPCVOID)source, length);
    return true;
}

bool HookManager::AttachHook(PVOID* ppPointer, PVOID pDetour)
{
    if (!ppPointer || !*ppPointer || !pDetour)
        return false;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    LONG status = DetourAttach(ppPointer, pDetour);
    DetourTransactionCommit();

    return (status == NO_ERROR);
}

bool HookManager::DetachHook(PVOID* ppPointer, PVOID pDetour)
{
    if (!ppPointer || !*ppPointer || !pDetour)
        return false;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    LONG status = DetourDetach(ppPointer, pDetour);
    DetourTransactionCommit();

    return (status == NO_ERROR);
}
