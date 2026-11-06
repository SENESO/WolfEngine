#include "pch.h"
#include "NdacHook.h"
#include "HookManager.h"

NdacHook::FnCloseSocket NdacHook::s_OrigCloseSocket = nullptr;

bool NdacHook::Initialize()
{
    m_hNdac = GetModuleHandleA("ndac.dll");
    if (!m_hNdac)
        m_hNdac = LoadLibraryA("ndac.dll");

    if (m_hNdac)
    {
        uintptr_t funcAddr = HookManager::Instance().PatternScan("ndac.dll", "\x55\x8B\xEC\x83\xEC\x10\x53\x56\x57", "xxxxxxxxx");
        if (funcAddr)
        {
            // Install bypass / trampoline
            uint8_t jmpCode[5] = { 0xE9, 0, 0, 0, 0 };
            // Patch
        }
    }

    HMODULE hWs2 = GetModuleHandleA("ws2_32.dll");
    if (hWs2)
    {
        s_OrigCloseSocket = (FnCloseSocket)GetProcAddress(hWs2, "closesocket");
        if (s_OrigCloseSocket)
        {
            HookManager::Instance().AttachHook((PVOID*)&s_OrigCloseSocket, (PVOID)HookCloseSocket);
        }
    }
    return true;
}

int WSAAPI NdacHook::HookCloseSocket(SOCKET s)
{
    if (s_OrigCloseSocket)
        return s_OrigCloseSocket(s);
    return 0;
}

