#include "pch.h"
#include "WolfEngine.h"

// Anchor export for static PE import table injection (IAT patching)
extern "C" __declspec(dllexport) void WolfEntry()
{
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
        OSVERSIONINFOEXW osInfo = { 0 };
        osInfo.dwOSVersionInfoSize = sizeof(osInfo);
#pragma warning(push)
#pragma warning(disable: 4996 28159)
        GetVersionExW((LPOSVERSIONINFOW)&osInfo);
#pragma warning(pop)

        DisableThreadLibraryCalls(hinstDLL);

        // Start WolfEngine asynchronously or directly
        WolfEngine::Instance().Initialize(hinstDLL);
        break;
    }
    case DLL_PROCESS_DETACH:
    {
        WolfEngine::Instance().Shutdown();
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}
