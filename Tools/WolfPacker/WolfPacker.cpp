#include <windows.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

// Container format reverse-engineered from WolfEngine.dll
// Magic: A5 A5 A5 A5 5A 5A 5A 5A 78 56 34 12 + CAFEBABE
#pragma pack(push, 1)
struct WolfDatHeader
{
    uint8_t  seedKey[16];   // 09 B7 6B 18 C7 64 C1 04 1B 44 EC 50 E6 36 6A 13
    uint32_t magic1;        // 0xA5A5A5A5
    uint32_t magic2;        // 0x5A5A5A5A
    uint32_t magic3;        // 0x12345678
    uint32_t magic4;        // 0xCAFEBABE
    uint32_t fileCount;
    uint32_t totalDataSize;
};

struct WolfDatEntry
{
    char     fileName[64];
    uint32_t fileOffset;
    uint32_t fileSize;
    uint32_t checksum;
};
#pragma pack(pop)

static const uint8_t DEFAULT_SEED[16] = {
    0x09, 0xB7, 0x6B, 0x18, 0xC7, 0x64, 0xC1, 0x04,
    0x1B, 0x44, 0xEC, 0x50, 0xE6, 0x36, 0x6A, 0x13
};

void PrintUsage()
{
    std::cout << "WolfPacker - WolfEngine.dat Archive Utility\n"
              << "Usage:\n"
              << "  WolfPacker -pack <inputFile> <outputDat>\n"
              << "  WolfPacker -unpack <inputDat> <outputDir>\n"
              << "  WolfPacker -info <inputDat>\n";
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        PrintUsage();
        return 1;
    }

    std::string mode = argv[1];

    if (mode == "-info")
    {
        std::ifstream dat(argv[2], std::ios::binary);
        if (!dat)
        {
            std::cerr << "Failed to open " << argv[2] << "\n";
            return 1;
        }

        WolfDatHeader hdr = { 0 };
        dat.read((char*)&hdr, sizeof(hdr));
        std::cout << "WolfEngine.dat Container Header:\n"
                  << "  Magic1: 0x" << std::hex << hdr.magic1 << "\n"
                  << "  Magic2: 0x" << std::hex << hdr.magic2 << "\n"
                  << "  Magic3: 0x" << std::hex << hdr.magic3 << "\n"
                  << "  Magic4: 0x" << std::hex << hdr.magic4 << "\n"
                  << "  File Count: " << std::dec << hdr.fileCount << "\n"
                  << "  Data Size:  " << hdr.totalDataSize << " bytes\n";
        return 0;
    }
    else if (mode == "-pack" && argc >= 4)
    {
        std::string inputFilePath = argv[2];
        std::string outputDatPath = argv[3];

        std::ifstream in(inputFilePath, std::ios::binary | std::ios::ate);
        if (!in)
        {
            std::cerr << "Cannot open input file: " << inputFilePath << "\n";
            return 1;
        }

        size_t fileSize = (size_t)in.tellg();
        in.seekg(0, std::ios::beg);
        std::vector<uint8_t> buffer(fileSize);
        in.read((char*)buffer.data(), fileSize);
        in.close();

        WolfDatHeader hdr = { 0 };
        memcpy(hdr.seedKey, DEFAULT_SEED, 16);
        hdr.magic1 = 0xA5A5A5A5;
        hdr.magic2 = 0x5A5A5A5A;
        hdr.magic3 = 0x12345678;
        hdr.magic4 = 0xCAFEBABE;
        hdr.fileCount = 1;
        hdr.totalDataSize = (uint32_t)fileSize;

        WolfDatEntry entry = { 0 };
        // Extract basename
        size_t lastSlash = inputFilePath.find_last_of("\\/");
        std::string baseName = (lastSlash != std::string::npos) ? inputFilePath.substr(lastSlash + 1) : inputFilePath;
        strncpy_s(entry.fileName, baseName.c_str(), sizeof(entry.fileName) - 1);
        entry.fileOffset = sizeof(hdr) + sizeof(entry);
        entry.fileSize = (uint32_t)fileSize;

        std::ofstream out(outputDatPath, std::ios::binary);
        if (!out)
        {
            std::cerr << "Cannot create output dat: " << outputDatPath << "\n";
            return 1;
        }

        out.write((const char*)&hdr, sizeof(hdr));
        out.write((const char*)&entry, sizeof(entry));
        out.write((const char*)buffer.data(), buffer.size());
        out.close();

        std::cout << "Successfully packed " << baseName << " into " << outputDatPath << "\n";
        return 0;
    }
    else if (mode == "-unpack" && argc >= 4)
    {
        std::string inputDatPath = argv[2];
        std::string outputDir = argv[3];

        std::ifstream in(inputDatPath, std::ios::binary);
        if (!in)
        {
            std::cerr << "Cannot open dat file: " << inputDatPath << "\n";
            return 1;
        }

        WolfDatHeader hdr = { 0 };
        in.read((char*)&hdr, sizeof(hdr));
        if (hdr.magic1 != 0xA5A5A5A5 || hdr.magic4 != 0xCAFEBABE)
        {
            std::cout << "Warning: Header magic does not match WolfEngine container specification.\n";
        }

        CreateDirectoryA(outputDir.c_str(), NULL);

        for (uint32_t i = 0; i < hdr.fileCount; ++i)
        {
            WolfDatEntry entry = { 0 };
            in.read((char*)&entry, sizeof(entry));

            std::string outFilePath = outputDir + "\\" + entry.fileName;
            std::streampos curr = in.tellg();

            in.seekg(entry.fileOffset, std::ios::beg);
            std::vector<uint8_t> fileBuf(entry.fileSize);
            in.read((char*)fileBuf.data(), entry.fileSize);

            std::ofstream out(outFilePath, std::ios::binary);
            if (out)
            {
                out.write((const char*)fileBuf.data(), fileBuf.size());
                out.close();
                std::cout << "Extracted: " << entry.fileName << " (" << entry.fileSize << " bytes)\n";
            }
            in.seekg(curr, std::ios::beg);
        }
        return 0;
    }

    PrintUsage();
    return 1;
}
