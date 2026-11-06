#include "pch.h"
#include "SteamProtocol.h"

std::string SteamProtocol::GetSteamExecutablePath()
{
    HKEY hKey = NULL;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        wchar_t steamExe[MAX_PATH] = { 0 };
        DWORD size = sizeof(steamExe);
        if (RegQueryValueExW(hKey, L"SteamExe", NULL, NULL, (LPBYTE)steamExe, &size) == ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            // Convert slashes
            for (size_t i = 0; steamExe[i]; ++i)
            {
                if (steamExe[i] == L'/') steamExe[i] = L'\\';
            }
            char pathA[MAX_PATH] = { 0 };
            WideCharToMultiByte(CP_ACP, 0, steamExe, -1, pathA, MAX_PATH, NULL, NULL);
            return pathA;
        }
        RegCloseKey(hKey);
    }
    return "";
}

bool SteamProtocol::RegisterProtocol(const std::string& protocolName, const std::string& gameAppId)
{
    std::string steamExe = GetSteamExecutablePath();
    if (steamExe.empty()) return false;

    std::string keyPath = "Software\\Classes\\" + protocolName;
    HKEY hKey = NULL;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, keyPath.c_str(), 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
        return false;

    std::string desc = "URL:Run game " + protocolName + " protocol";
    RegSetValueExA(hKey, NULL, 0, REG_SZ, (const BYTE*)desc.c_str(), (DWORD)desc.length() + 1);
    RegSetValueExA(hKey, "URL Protocol", 0, REG_SZ, (const BYTE*)"", 1);

    // Command
    std::string cmdKeyPath = keyPath + "\\shell\\open\\command";
    HKEY hCmdKey = NULL;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, cmdKeyPath.c_str(), 0, NULL, 0, KEY_WRITE, NULL, &hCmdKey, NULL) == ERROR_SUCCESS)
    {
        char command[MAX_PATH * 2];
        sprintf_s(command, "\"%s\" steam://rungameid/%s", steamExe.c_str(), gameAppId.c_str());
        RegSetValueExA(hCmdKey, NULL, 0, REG_SZ, (const BYTE*)command, (DWORD)strlen(command) + 1);
        RegCloseKey(hCmdKey);
    }

    RegCloseKey(hKey);
    return true;
}

