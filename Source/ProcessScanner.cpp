#include "pch.h"
#include "ProcessScanner.h"
#include "NetworkManager.h"

ProcessScanner::ProcessScanner() : m_running(false)
{
    // Whitelisted OS critical processes
    m_whitelist = {
        L"csrss.exe", L"services.exe", L"ekrn.exe", L"lsass.exe", L"smss.exe", L"explorer.exe"
    };

    // Blacklisted hack tools, injectors, clickers, debuggers
    std::vector<std::wstring> list = {
        L"autoclicker.exe", L"cheatengine.exe", L"artmoney.exe", L"taskmanagerdeluxe",
        L"systeminformer", L"speedhack.dll", L"injector.exe", L"xradar.exe",
        L"wpepro.exe", L"trainer.exe", L"gameguardian.exe", L"memhack.exe",
        L"sbiectrl.exe", L"macrorecorder.exe", L"noping.exe", L"hackshield.exe",
        L"processhacker.exe", L"processhacker", L"taskmgr.exe", L"resourcehacker.exe",
        L"ollydbg.exe", L"idapro.exe", L"x64dbg.exe", L"x32dbg.exe",
        L"wireshark.exe", L"fiddler.exe", L"cain.exe", L"netlimiter.exe",
        L"dbgview.exe", L"hxd.exe", L"autohotkey.exe", L"autoit.exe",
        L"hpbot.exe", L"autohp.exe", L"clickbot.exe", L"mouseclicker.exe",
        L"keyboardmacro.exe", L"jitbitmacrorecorder.exe", L"tinytask.exe", L"botmek.exe",
        L"murgeemon.exe", L"gsmacro.exe", L"pulovermacrocreator.exe", L"cheatenginetrainer.exe",
        L"ce_trainer.exe", L"darkbyteengine.exe", L"memoryeditor.exe", L"speedgear.exe",
        L"tsearch.exe", L"mhs.exe"
    };

    for (const auto& item : list)
        m_blacklist.insert(item);
}

bool ProcessScanner::EnableDebugPrivilege()
{
    HANDLE hToken = NULL;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return false;

    LUID luid;
    if (!LookupPrivilegeValueW(NULL, L"SeDebugPrivilege", &luid))
    {
        CloseHandle(hToken);
        return false;
    }

    TOKEN_PRIVILEGES tp;
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    BOOL result = AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
    CloseHandle(hToken);
    return (result != FALSE && GetLastError() != ERROR_NOT_ALL_ASSIGNED);
}

void ProcessScanner::Start()
{
    if (m_running) return;
    EnableDebugPrivilege();
    m_running = true;
    m_scanThread = std::thread(&ProcessScanner::ScanLoop, this);
}

void ProcessScanner::Stop()
{
    m_running = false;
    if (m_scanThread.joinable())
        m_scanThread.join();
}

bool ProcessScanner::IsBlacklisted(const std::wstring& processName)
{
    std::wstring lower = processName;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    if (m_whitelist.find(lower) != m_whitelist.end())
        return false;

    for (const auto& blocked : m_blacklist)
    {
        if (lower.find(blocked) != std::wstring::npos)
            return true;
    }
    return false;
}

bool ProcessScanner::CheckProcesses()
{
    PWTS_PROCESS_INFOW pProcessInfo = NULL;
    DWORD processCount = 0;

    if (WTSEnumerateProcessesW(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pProcessInfo, &processCount))
    {
        for (DWORD i = 0; i < processCount; ++i)
        {
            if (pProcessInfo[i].pProcessName)
            {
                std::wstring name = pProcessInfo[i].pProcessName;
                if (IsBlacklisted(name))
                {
                    WTSFreeMemory(pProcessInfo);
                    return true;
                }
            }
        }
        WTSFreeMemory(pProcessInfo);
    }
    return false;
}

void ProcessScanner::ScanLoop()
{
    while (m_running)
    {
        if (CheckProcesses())
        {
            NetworkManager::Instance().SendCheatReport("Blacklisted program detected");
            MessageBoxA(NULL, "Hacker program detected. Exiting game...", "Hacker Program Detected", MB_OK | MB_ICONERROR);
            ExitProcess(0);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2500));
    }
}

