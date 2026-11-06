#pragma once
#include "framework.h"
#include "FileIntegrity.h"

class NetworkManager
{
public:
    static NetworkManager& Instance()
    {
        static NetworkManager instance;
        return instance;
    }

    bool Initialize(SOCKET s);
    bool PerformAuthHandshake();
    bool SendCheatReport(const char* reason);
    void StartListener();
    void StopListener();
    void SetSocket(SOCKET s) { m_socket = s; }
    SOCKET GetSocket() const { return m_socket; }

private:
    NetworkManager() : m_socket(INVALID_SOCKET), m_isAuthenticated(false), m_listening(false) {}
    void CommandListenerLoop();

    SOCKET m_socket;
    bool m_isAuthenticated;
    std::atomic<bool> m_listening;
    std::thread m_listenerThread;
};

