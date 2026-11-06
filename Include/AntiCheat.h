#pragma once
#include "framework.h"
#include "ProcessScanner.h"
#include "MacroDetector.h"
#include "MemoryIntegrity.h"

class AntiCheat
{
public:
    static AntiCheat& Instance()
    {
        static AntiCheat instance;
        return instance;
    }

    bool Initialize(HINSTANCE hInst)
    {
        ProcessScanner::Instance().Start();
        MacroDetector::Instance().Initialize(hInst);
        MemoryIntegrity::Instance().Start();
        return true;
    }

    void Shutdown()
    {
        ProcessScanner::Instance().Stop();
        MacroDetector::Instance().Uninitialize();
        MemoryIntegrity::Instance().Stop();
    }
};

