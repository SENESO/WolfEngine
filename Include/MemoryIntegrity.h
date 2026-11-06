#pragma once
#include "framework.h"

class MemoryIntegrity
{
public:
    static MemoryIntegrity& Instance()
    {
        static MemoryIntegrity instance;
        return instance;
    }

    void Start();
    void Stop();

    bool CheckDebugger();
    bool CheckSendInputHook();
    bool CheckMemoryHooks();

private:
    MemoryIntegrity() : m_running(false) {}
    void MonitorLoop();

    std::atomic<bool> m_running;
    std::thread m_monitorThread;
};

