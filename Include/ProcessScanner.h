#pragma once
#include "framework.h"

class ProcessScanner
{
public:
    static ProcessScanner& Instance()
    {
        static ProcessScanner instance;
        return instance;
    }

    void Start();
    void Stop();
    static bool EnableDebugPrivilege();

private:
    ProcessScanner();
    void ScanLoop();
    bool CheckProcesses();
    bool IsBlacklisted(const std::wstring& processName);

    std::atomic<bool> m_running;
    std::thread m_scanThread;
    std::set<std::wstring> m_blacklist;
    std::set<std::wstring> m_whitelist;
};

