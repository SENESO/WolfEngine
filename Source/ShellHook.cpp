#include "pch.h"
#include "ShellHook.h"
#include "HookManager.h"

ShellHook::FnShellExecuteA ShellHook::s_OrigShellExecuteA = nullptr;

bool ShellHook::Initialize()
{
    HMODULE hShell32 = GetModuleHandleA("Shell32.dll");
    if (!hShell32)
        hShell32 = LoadLibraryA("Shell32.dll");
    if (!hShell32) return false;

    s_OrigShellExecuteA = (FnShellExecuteA)GetProcAddress(hShell32, "ShellExecuteA");
    if (s_OrigShellExecuteA)
    {
        HookManager::Instance().AttachHook((PVOID*)&s_OrigShellExecuteA, (PVOID)HookShellExecuteA);
        return true;
    }
    return false;
}

HINSTANCE WINAPI ShellHook::HookShellExecuteA(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd)
{
    // Return SE_ERR_FNF (2) to suppress external browser/URL pops from client
    return (HINSTANCE)2;
}

