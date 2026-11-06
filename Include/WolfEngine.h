#pragma once
#include "framework.h"

class WolfEngine
{
public:
    static WolfEngine& Instance()
    {
        static WolfEngine instance;
        return instance;
    }

    bool Initialize(HMODULE hModule);
    void Shutdown();

    HMODULE GetModuleHandle() const { return m_hModule; }

private:
    WolfEngine() : m_hModule(NULL), m_initialized(false) {}
    HMODULE m_hModule;
    bool m_initialized;
};

