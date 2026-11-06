#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    const char* dir = "Env_DX9";
    const char* exe = "Env_DX9\\Conquer.exe";
    if (GetFileAttributesA(exe) == INVALID_FILE_ATTRIBUTES)
    {
        dir = "Env_DX8";
        exe = "Env_DX8\\Conquer.exe";
    }

    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };

    char cmdLine[MAX_PATH] = "\"Conquer.exe\"";
    if (CreateProcessA(exe, cmdLine, NULL, NULL, FALSE, 0, NULL, dir, &si, &pi))
    {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return 0;
    }

    MessageBoxA(NULL, "Failed to launch Conquer.exe from Env_DX9 / Env_DX8.", "Launch Error", MB_ICONERROR);
    return 1;
}

