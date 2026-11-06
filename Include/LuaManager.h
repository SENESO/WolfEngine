#pragma once
#include "framework.h"

class LuaManager
{
public:
    static LuaManager& Instance()
    {
        static LuaManager instance;
        return instance;
    }

    bool Initialize();
    bool LoadWolfEngineDat(const std::string& path = "WolfEngine.dat");
    bool InitializeSpiritSystem();
    bool PatchGuiIni();

    const std::string& GetConfigData() const { return m_configData; }

private:
    LuaManager() : m_initialized(false) {}
    bool m_initialized;
    std::string m_configData;
};

