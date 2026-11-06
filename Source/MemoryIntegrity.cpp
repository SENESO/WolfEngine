#include "pch.h"
#include "MemoryIntegrity.h"
#include "NetworkManager.h"

void MemoryIntegrity::Start()
{
    if (m_running) return;
    m_running = true;
    m_monitorThread = std::thread(&MemoryIntegrity::MonitorLoop, this);
}

void MemoryIntegrity::Stop()
{
    m_running = false;
    if (m_monitorThread.joinable())
        m_monitorThread.join();
}

bool MemoryIntegrity::CheckDebugger()
{
    if (IsDebuggerPresent())
        return true;

    BOOL isDebugger = FALSE;
    if (CheckRemoteDebuggerPresent(GetCurrentProcess(), &isDebugger) && isDebugger)
        return true;

    return false;
}

bool MemoryIntegrity::CheckSendInputHook()
{
    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    if (!hUser32) return false;

    FARPROC pSendInput = GetProcAddress(hUser32, "SendInput");
    if (!pSendInput) return false;

    // Check if first byte is JMP (0xE9) or INT3 (0xCC) or JMP [mem] (0xFF 0x25)
    uint8_t firstByte = *(uint8_t*)pSendInput;
    if (firstByte == 0xE9 || firstByte == 0xCC || (firstByte == 0xFF && *((uint8_t*)pSendInput + 1) == 0x25))
    {
        return true;
    }
    return false;
}

bool MemoryIntegrity::CheckMemoryHooks()
{
    // Check critical APIs in ntdll / kernel32 / ws2_32
    HMODULE hWs2 = GetModuleHandleA("ws2_32.dll");
    if (hWs2)
    {
        FARPROC pSend = GetProcAddress(hWs2, "send");
        if (pSend && (*(uint8_t*)pSend == 0xE9 || *(uint8_t*)pSend == 0xCC))
            return true;
    }
    return false;
}

void MemoryIntegrity::MonitorLoop()
{
    while (m_running)
    {
        if (CheckDebugger())
        {
            NetworkManager::Instance().SendCheatReport("Debugger detected");
            MessageBoxA(NULL, "Debugger detected! Closing game...", "Anti-Debug", MB_OK | MB_ICONERROR);
            ExitProcess(0);
        }

        if (CheckSendInputHook())
        {
            NetworkManager::Instance().SendCheatReport("SendInput Hooked");
            OutputDebugStringA("SendInput Hooked! Possible cheating detected.\n");
            ExitProcess(0);
        }

        if (CheckMemoryHooks())
        {
            NetworkManager::Instance().SendCheatReport("Memory Hook detected");
            MessageBoxA(NULL, "Memory Hook detected! Closing game...", "Memory Integrity Error", MB_OK | MB_ICONERROR);
            ExitProcess(0);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    }
}

