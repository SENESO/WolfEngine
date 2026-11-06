#pragma once
#include "framework.h"

class DiscordRPC
{
public:
    static DiscordRPC& Instance()
    {
        static DiscordRPC instance;
        return instance;
    }

    bool Initialize(const std::string& clientId = "1230904445949186220");
    void Shutdown();

    void UpdatePresence(const std::string& state, const std::string& details);

private:
    DiscordRPC() : m_hPipe(INVALID_HANDLE_VALUE), m_running(false) {}
    bool ConnectPipe();
    bool SendHandshake();
    bool SendFrame(int opcode, const std::string& payload);

    std::string m_clientId;
    HANDLE m_hPipe;
    std::atomic<bool> m_running;
    std::thread m_updateThread;
    int64_t m_startTime;
};

