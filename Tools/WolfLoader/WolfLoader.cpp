#include <windows.h>
#include <iostream>
#include <string>

bool InjectDll(HANDLE hProcess, const std::string& dllPath)
{
    LPVOID pRemoteBuf = VirtualAllocEx(hProcess, NULL, dllPath.length() + 1, MEM_COMMIT, PAGE_READWRITE);
    if (!pRemoteBuf)
    {
        std::cerr << "VirtualAllocEx failed: " << GetLastError() << "\n";
        return false;
    }

    if (!WriteProcessMemory(hProcess, pRemoteBuf, dllPath.c_str(), dllPath.length() + 1, NULL))
    {
        std::cerr << "WriteProcessMemory failed: " << GetLastError() << "\n";
        VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);
        return false;
    }

    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    FARPROC pLoadLibraryA = GetProcAddress(hKernel32, "LoadLibraryA");

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibraryA, pRemoteBuf, 0, NULL);
    if (!hThread)
    {
        std::cerr << "CreateRemoteThread failed: " << GetLastError() << "\n";
        VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);
        return false;
    }

    WaitForSingleObject(hThread, 5000);
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);
    return true;
}

int main(int argc, char* argv[])
{
    std::cout << "========================================\n"
              << " WolfLoader - Conquer Online Injector   \n"
              << "========================================\n";

    std::string conquerPath = "Conquer.exe";
    std::string dllPath = "WolfEngine.dll";

    if (argc >= 2) conquerPath = argv[1];
    if (argc >= 3) dllPath = argv[2];

    char fullDllPath[MAX_PATH] = { 0 };
    GetFullPathNameA(dllPath.c_str(), MAX_PATH, fullDllPath, NULL);

    std::cout << "Launching: " << conquerPath << " blacknull\n";
    std::cout << "Injecting: " << fullDllPath << "\n";

    // Conquer.exe requires "blacknull" argument to bypass "Please run Play.exe file."
    char cmdLine[MAX_PATH * 2] = { 0 };
    snprintf(cmdLine, sizeof(cmdLine), "\"%s\" blacknull", conquerPath.c_str());

    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };

    // Launch Conquer.exe in suspended state with blacknull parameter
    BOOL success = CreateProcessA(
        NULL,
        cmdLine,
        NULL,
        NULL,
        FALSE,
        CREATE_SUSPENDED,
        NULL,
        NULL,
        &si,
        &pi
    );

    if (!success)
    {
        std::cerr << "Failed to start Conquer.exe (Error: " << GetLastError() << ")\n";
        return 1;
    }

    std::cout << "Process created (PID: " << pi.dwProcessId << "). Injecting WolfEngine.dll...\n";

    if (InjectDll(pi.hProcess, fullDllPath))
    {
        std::cout << "WolfEngine.dll successfully injected!\n";
    }
    else
    {
        std::cerr << "Failed to inject WolfEngine.dll!\n";
    }

    // Resume execution
    ResumeThread(pi.hThread);
    std::cout << "Process resumed.\n";

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return 0;
}
