#pragma once
#include "framework.h"

class ShellHook
{
public:
    static ShellHook& Instance()
    {
        static ShellHook instance;
        return instance;
    }

    bool Initialize();

    typedef HINSTANCE (WINAPI *FnShellExecuteA)(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd);
    static FnShellExecuteA s_OrigShellExecuteA;

    static HINSTANCE WINAPI HookShellExecuteA(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd);
};

