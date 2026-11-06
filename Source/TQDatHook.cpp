#include "pch.h"
#include "TQDatHook.h"
#include "HookManager.h"

TQDatHook::FnTQDatInit TQDatHook::s_OrigTQDatInit = nullptr;

bool TQDatHook::Initialize()
{
    // Scan pattern in client
    uintptr_t addr1 = HookManager::Instance().PatternScan(NULL, "\x74\x00\x00\x00\x00\x00", "x?xxxx?????x");
    uintptr_t addr2 = HookManager::Instance().PatternScan(NULL, "\x55\x8B\xEC\xFF\x75\x0C\x6A\xFF\xFF\x75\x08\xE8\x00\x00\x00\x00", "xxxxxxxxxxxx????xxx");

    m_hTQDat = LoadLibraryA("TQDat.dll");
    if (m_hTQDat)
    {
        s_OrigTQDatInit = (FnTQDatInit)GetProcAddress(m_hTQDat, "TQDatInit");
        if (s_OrigTQDatInit)
        {
            // Initialize TQ dat loader with custom key parameters
            uint8_t datKey[16] = { 0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x20 };
            s_OrigTQDatInit(datKey, 5, 0xAA);
        }
    }
    return true;
}

