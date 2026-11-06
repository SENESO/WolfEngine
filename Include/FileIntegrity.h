#pragma once
#include "framework.h"

struct ClientWelcomePacket
{
    uint32_t packetId;             // offset 0x00: 0x00000001
    char magicTypeHash[33];        // offset 0x04: ini/MagicType.dat
    char conquerDX8Hash[33];       // offset 0x25: Env_DX8/Conquer.exe
    char magicEffectHash[33];      // offset 0x46: ini/MagicEffect.ini
    char wolfEngineDatHash[33];    // offset 0x67: WolfEngine.dat
    char serverCnResHash[33];      // offset 0x88: ini/Server_Cn_Res.ini
    char wolfEngineDllHash[33];    // offset 0xA9: WolfEngine.dll
    char conquerDX9Hash[33];       // offset 0xCA: Env_DX9/Conquer.exe
    char extraHash1[33];           // offset 0xEB
    char extraHash2[33];           // offset 0x10C
    char extraHash3[33];           // offset 0x12D
    char extraHash4[33];           // offset 0x14E
    char extraHash5[33];           // offset 0x16F
    uint8_t padding[13];           // Total size: 400 bytes
};

class FileIntegrity
{
public:
    static FileIntegrity& Instance()
    {
        static FileIntegrity instance;
        return instance;
    }

    std::string CalculateMD5(const std::string& filePath);
    bool BuildWelcomePacket(ClientWelcomePacket& packet);
};

