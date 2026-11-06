#include "pch.h"
#include "FileIntegrity.h"

std::string FileIntegrity::CalculateMD5(const std::string& filePath)
{
    HANDLE hFile = CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return "-1";

    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    if (!CryptAcquireContextW(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
    {
        CloseHandle(hFile);
        return "-1";
    }

    if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
    {
        CryptReleaseContext(hProv, 0);
        CloseHandle(hFile);
        return "-1";
    }

    BYTE buffer[4096];
    DWORD bytesRead = 0;
    while (ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0)
    {
        CryptHashData(hHash, buffer, bytesRead, 0);
    }

    BYTE hashBytes[16];
    DWORD hashLen = sizeof(hashBytes);
    std::string result = "-1";

    if (CryptGetHashParam(hHash, HP_HASHVAL, hashBytes, &hashLen, 0))
    {
        char hex[33] = { 0 };
        for (DWORD i = 0; i < hashLen; ++i)
            sprintf_s(&hex[i * 2], 3, "%02x", hashBytes[i]);
        result = hex;
    }

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    CloseHandle(hFile);

    return result;
}

bool FileIntegrity::BuildWelcomePacket(ClientWelcomePacket& packet)
{
    memset(&packet, 0, sizeof(packet));
    packet.packetId = 1;

    auto copyHash = [](char* dest, const std::string& hash) {
        strncpy_s(dest, 33, hash.c_str(), _TRUNCATE);
    };

    // Calculate MD5 of primary game assets
    copyHash(packet.magicTypeHash, CalculateMD5("ini/MagicType.dat"));
    copyHash(packet.conquerDX8Hash, CalculateMD5("Env_DX8/Conquer.exe"));
    copyHash(packet.magicEffectHash, CalculateMD5("ini/MagicEffect.ini"));
    copyHash(packet.wolfEngineDatHash, CalculateMD5("WolfEngine.dat"));
    copyHash(packet.serverCnResHash, CalculateMD5("ini/Server_Cn_Res.ini"));
    copyHash(packet.wolfEngineDllHash, CalculateMD5("WolfEngine.dll"));
    copyHash(packet.conquerDX9Hash, CalculateMD5("Env_DX9/Conquer.exe"));

    return true;
}

