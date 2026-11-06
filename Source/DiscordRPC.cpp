#include "pch.h"
#include "DiscordRPC.h"

bool DiscordRPC::Initialize(const std::string& clientId)
{
    m_clientId = clientId;
    m_startTime = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    if (!ConnectPipe())
        return false;

    if (!SendHandshake())
        return false;

    m_running = true;
    UpdatePresence("Idle", "Best Conquer Private Server");

    return true;
}

void DiscordRPC::Shutdown()
{
    m_running = false;
    if (m_hPipe != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hPipe);
        m_hPipe = INVALID_HANDLE_VALUE;
    }
}

bool DiscordRPC::ConnectPipe()
{
    for (int i = 0; i < 10; ++i)
    {
        wchar_t pipeName[64];
        swprintf_s(pipeName, L"\\\\?\\pipe\\discord-ipc-%d", i);

        m_hPipe = CreateFileW(pipeName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (m_hPipe != INVALID_HANDLE_VALUE)
            return true;

        if (GetLastError() == ERROR_PIPE_BUSY)
        {
            if (WaitNamedPipeW(pipeName, 5000))
            {
                m_hPipe = CreateFileW(pipeName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
                if (m_hPipe != INVALID_HANDLE_VALUE)
                    return true;
            }
        }
    }
    return false;
}

bool DiscordRPC::SendFrame(int opcode, const std::string& payload)
{
    if (m_hPipe == INVALID_HANDLE_VALUE) return false;

    struct FrameHeader
    {
        uint32_t opcode;
        uint32_t length;
    } header;

    header.opcode = opcode;
    header.length = (uint32_t)payload.length();

    DWORD bytesWritten = 0;
    if (!WriteFile(m_hPipe, &header, sizeof(header), &bytesWritten, NULL))
        return false;

    return WriteFile(m_hPipe, payload.data(), (DWORD)payload.length(), &bytesWritten, NULL) != FALSE;
}

bool DiscordRPC::SendHandshake()
{
    // Opcode 0 = Handshake
    std::string json = "{\"v\":1,\"client_id\":\"" + m_clientId + "\"}";
    return SendFrame(0, json);
}

void DiscordRPC::UpdatePresence(const std::string& state, const std::string& details)
{
    std::ostringstream ss;
    ss << "{"
       << "\"cmd\":\"SET_ACTIVITY\","
       << "\"args\":{"
       << "\"pid\":" << GetCurrentProcessId() << ","
       << "\"activity\":{"
       << "\"state\":\"" << state << "\","
       << "\"details\":\"" << details << "\","
       << "\"timestamps\":{\"start\":" << m_startTime << "},"
       << "\"assets\":{"
       << "\"large_image\":\"logo512\","
       << "\"large_text\":\"Best Conquer Private Server\""
       << "}"
       << "}},"
       << "\"nonce\":\"" << GetTickCount() << "\""
       << "}";

    // Opcode 1 = Frame
    SendFrame(1, ss.str());
}

