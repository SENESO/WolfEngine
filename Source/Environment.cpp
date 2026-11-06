#include "pch.h"
#include "Environment.h"

bool Environment::Initialize()
{
    char modulePath[MAX_PATH] = { 0 };
    if (!GetModuleFileNameA(NULL, modulePath, MAX_PATH))
    {
        ExitProcess(0);
        return false;
    }

    m_processPath = modulePath;

    // Find executable filename
    size_t lastSlash = m_processPath.find_last_of("\\/");
    std::string fileName = (lastSlash != std::string::npos) ? m_processPath.substr(lastSlash + 1) : m_processPath;

    // Enforce that target process MUST be Conquer.exe
    if (_stricmp(fileName.c_str(), "Conquer.exe") != 0)
    {
        MessageBoxA(NULL, "This engine can only be loaded by Conquer.exe", "WolfEngine Error", MB_ICONERROR);
        ExitProcess(0);
        return false;
    }

    // Check parent folder for DirectX mode (env_dx9 vs env_dx8)
    if (lastSlash != std::string::npos)
    {
        size_t prevSlash = m_processPath.find_last_of("\\/", lastSlash - 1);
        if (prevSlash != std::string::npos)
        {
            m_envFolder = m_processPath.substr(prevSlash + 1, lastSlash - prevSlash - 1);
            std::string lowerEnv = m_envFolder;
            std::transform(lowerEnv.begin(), lowerEnv.end(), lowerEnv.begin(), ::tolower);
            if (lowerEnv.find("env_dx9") != std::string::npos)
            {
                m_isDX9 = true;
            }
        }
    }

    return true;
}

