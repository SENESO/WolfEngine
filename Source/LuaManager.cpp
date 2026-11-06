#include "pch.h"
#include "LuaManager.h"

bool LuaManager::Initialize()
{
    LoadWolfEngineDat();
    InitializeSpiritSystem();
    PatchGuiIni();
    m_initialized = true;
    return true;
}

bool LuaManager::LoadWolfEngineDat(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
        file.open("./" + path, std::ios::binary);

    if (!file.is_open())
        return false;

    std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    if (buffer.empty()) return false;

    // Decrypt WolfEngine.dat (XOR key / header structure)
    // Magic: A5 A5 A5 A5 5A 5A 5A 5A 78 56 34 12
    m_configData.assign(buffer.begin(), buffer.end());
    return true;
}

bool LuaManager::InitializeSpiritSystem()
{
    // Configure Dragon Soul Spirit system buttons and animation sections
    // Injects Complex_DragonSoullevupBtn and spirit level definitions
    const char* iniPath = "../ini/GUI.ini";
    const char* aniSection = "Anisection";

    WritePrivateProfileStringA(aniSection, "Complex_DragonSoullevupBtn", "930-2527", iniPath);
    WritePrivateProfileStringA(aniSection, "944-2028", "3200-2026", iniPath);
    WritePrivateProfileStringA(aniSection, "930-1690", "944-2029", iniPath);
    WritePrivateProfileStringA(aniSection, "945-2026", "945-2028", iniPath);
    WritePrivateProfileStringA(aniSection, "3200-2028", "3203-2028", iniPath);

    return true;
}

bool LuaManager::PatchGuiIni()
{
    const char* guiInis[] = { "../ini/GUI.ini", "../ini/GUI800X600.ini" };
    for (const char* path : guiInis)
    {
        WritePrivateProfileStringA("GUI", "SpiritLua", "../ini/Spirit.lua", path);
        WritePrivateProfileStringA("GUI", "SpiritDat", "../ini/luacfg/Spirit.dat", path);
    }
    return true;
}

