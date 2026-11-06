#pragma once
#include "framework.h"

class Environment
{
public:
    static Environment& Instance()
    {
        static Environment instance;
        return instance;
    }

    bool Initialize();
    bool IsDX9() const { return m_isDX9; }
    const std::string& GetProcessPath() const { return m_processPath; }
    const std::string& GetEnvironmentFolder() const { return m_envFolder; }

private:
    Environment() : m_isDX9(false) {}
    bool m_isDX9;
    std::string m_processPath;
    std::string m_envFolder;
};

