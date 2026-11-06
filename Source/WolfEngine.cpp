#include "pch.h"
#include "WolfEngine.h"
#include "Environment.h"
#include "HookManager.h"
#include "GraphicHook.h"
#include "ShellHook.h"
#include "TQDatHook.h"
#include "NdacHook.h"
#include "LuaManager.h"
#include "ProcessScanner.h"
#include "MacroDetector.h"
#include "MemoryIntegrity.h"
#include "DiscordRPC.h"
#include "SteamProtocol.h"

bool WolfEngine::Initialize(HMODULE hModule)
{
    if (m_initialized) return true;
    m_hModule = hModule;

    // 1. Validate environment (Conquer.exe and DirectX mode)
    if (!Environment::Instance().Initialize())
        return false;

    // 2. Apply initial memory patch (5 NOPs on x????xxxxxxxxxx)
    uintptr_t patchAddr = HookManager::Instance().PatternScan(NULL, "\xE8\x00\x00\x00\x00\x85\xC0", "x????xx");
    if (patchAddr)
    {
        HookManager::Instance().NopMemory(patchAddr, 5);
    }

    // 3. Initialize engine hooks
    ShellHook::Instance().Initialize();
    GraphicHook::Instance().Initialize(Environment::Instance().IsDX9());
    TQDatHook::Instance().Initialize();
    NdacHook::Instance().Initialize();

    // 4. Initialize Lua subsystem and GUI modifications
    LuaManager::Instance().Initialize();

    // 5. Start security & anti-cheat monitors
    ProcessScanner::Instance().Start();
    MacroDetector::Instance().Initialize(hModule);
    MemoryIntegrity::Instance().Start();

    // 6. Start Discord Rich Presence & Steam protocol
    DiscordRPC::Instance().Initialize("1230904445949186220");
    SteamProtocol::Instance().RegisterProtocol("conquer", "1230904445949186220");

    m_initialized = true;
    return true;
}

void WolfEngine::Shutdown()
{
    if (!m_initialized) return;

    ProcessScanner::Instance().Stop();
    MacroDetector::Instance().Uninitialize();
    MemoryIntegrity::Instance().Stop();
    DiscordRPC::Instance().Shutdown();

    m_initialized = false;
}

