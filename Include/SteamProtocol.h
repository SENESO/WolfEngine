#pragma once
#include "framework.h"

class SteamProtocol
{
public:
    static SteamProtocol& Instance()
    {
        static SteamProtocol instance;
        return instance;
    }

    bool RegisterProtocol(const std::string& protocolName, const std::string& gameAppId);
    std::string GetSteamExecutablePath();
};

