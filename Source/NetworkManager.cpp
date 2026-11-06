#include "pch.h"
#include "NetworkManager.h"

bool NetworkManager::Initialize(SOCKET s)
{
    m_socket = s;
    return true;
}

bool NetworkManager::PerformAuthHandshake()
{
    if (m_socket == INVALID_SOCKET) return false;

    // Build and send the 400-byte welcome hash packet
    ClientWelcomePacket packet;
    FileIntegrity::Instance().BuildWelcomePacket(packet);

    int sent = send(m_socket, (const char*)&packet, sizeof(packet), 0);
    if (sent != sizeof(packet))
    {
        closesocket(m_socket);
        WSACleanup();
        shutdown(m_socket, SD_BOTH);
        ExitProcess(0);
        return false;
    }

    // Await server verification response
    char response[10000] = { 0 };
    int recvd = recv(m_socket, response, sizeof(response) - 1, 0);
    if (recvd <= 0)
    {
        ExitProcess(0);
        return false;
    }

    if (strcmp(response, "BANNED") == 0)
    {
        MessageBoxA(NULL, "Your account has been banned. Please contact support.", "Banned", MB_OK | MB_ICONERROR);
        shutdown(m_socket, SD_BOTH);
        ExitProcess(0);
        return false;
    }
    else if (strcmp(response, "VALID") == 0)
    {
        m_isAuthenticated = true;
        // Receive additional server configuration
        recv(m_socket, response, sizeof(response) - 1, 0);
        Sleep(100);
        recv(m_socket, response, sizeof(response) - 1, 0);
        StartListener();
        return true;
    }
    else
    {
        MessageBoxA(NULL, "Please~Reinstall~your~client~there~are~some~unauthorized~changes~made~to~some~critical~files~from~our~client.", "Error", MB_OK | MB_ICONERROR);
        shutdown(m_socket, SD_BOTH);
        ExitProcess(0);
        return false;
    }
}

bool NetworkManager::SendCheatReport(const char* reason)
{
    if (m_socket == INVALID_SOCKET) return false;

    char packet[512] = { 0 };
    *(uint32_t*)packet = 0x00000002; // Cheat Report Packet Opcode
    strncpy_s(packet + 4, sizeof(packet) - 4, reason, _TRUNCATE);

    int sent = send(m_socket, packet, (int)strlen(packet + 4) + 5, 0);
    return (sent > 0);
}


void NetworkManager::StartListener()
{
    if (m_listening) return;
    m_listening = true;
    m_listenerThread = std::thread(&NetworkManager::CommandListenerLoop, this);
}

void NetworkManager::StopListener()
{
    m_listening = false;
    if (m_listenerThread.joinable())
        m_listenerThread.join();
}

void NetworkManager::CommandListenerLoop()
{
    char buffer[10008] = { 0 };
    while (m_listening && m_socket != INVALID_SOCKET)
    {
        int bytes = recv(m_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) break;

        buffer[bytes] = '\0';
        if (strcmp(buffer, "KILL") == 0)
        {
            shutdown(m_socket, SD_BOTH);
            closesocket(m_socket);
            ExitProcess(0);
        }
    }
}
