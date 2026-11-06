// WolfImportPatcher - Static PE Import Injection (IAT patching)
// Adds a new ".newimp" section and forces the target PE to load a DLL
// via the Windows PE Loader (ntdll!LdrLoadDll) before the original entry point.
//
// Usage:
//   WolfImportPatcher <target.exe> [options]
//
// Options:
//   --dll <name>           DLL to inject (default: WolfEngine.dll)
//   --import <name>        Exported function name to import (default: WolfEntry)
//   --ordinal <n>          Import by ordinal n instead of by name
//   --out <file>           Output file (default: overwrite target in-place)
//   --section <name>       New section name (default: .newimp, max 8 chars)
//   --backup               Create <target>.bak before modifying
//   --help                 Show this help
//
// Notes:
// - The imported function MUST be exported by the DLL, otherwise the PE loader
//   will fail to start the process ("entry point not found" / ordinal missing).
// - WolfEngine.def currently exports only _DllMain@12; add/export a plain
//   function (e.g. WolfEntry) or use --import _DllMain@12 / --ordinal 1 as
//   appropriate for your build.

#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

struct Options {
    std::string target;
    std::string dllName = "WolfEngine.dll";
    std::string importName = "WolfEntry";
    uint16_t    importOrdinal = 0;   // 0 = use name
    bool        useOrdinal = false;
    std::string outPath;
    std::string sectionName = ".newimp";
    bool        backup = false;
};

void PrintUsage(const char* argv0) {
    std::cout
        << "WolfImportPatcher - Static PE Import Injection (IAT / .newimp)\n\n"
        << "Usage:\n  " << (argv0 ? argv0 : "WolfImportPatcher")
        << " <target.exe> [options]\n\n"
        << "Options:\n"
        << "  --dll <name>        DLL to force-load (default: WolfEngine.dll)\n"
        << "  --import <name>     Export function name to reference (default: WolfEntry)\n"
        << "  --ordinal <n>       Import by ordinal instead of by name\n"
        << "  --out <file>        Write result to <file> (default: modify target in-place)\n"
        << "  --section <name>    New section name, max 8 chars (default: .newimp)\n"
        << "  --backup            Save a copy as <target>.bak before modifying\n"
        << "  --help              Show help\n\n"
        << "The referenced export must exist in the DLL or the target will fail to start.\n";
}

bool ParseArgs(int argc, char** argv, Options& opt, bool& showHelp) {
    showHelp = false;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--help" || a == "-h" || a == "/?") { PrintUsage(argv[0]); showHelp = true; return false; }
        else if (a == "--backup") { opt.backup = true; }
        else if ((a == "--dll" || a == "--import" || a == "--ordinal" || a == "--out" || a == "--section") && i + 1 < argc) {
            std::string v = argv[++i];
            if (a == "--dll") opt.dllName = v;
            else if (a == "--import") { opt.importName = v; opt.useOrdinal = false; }
            else if (a == "--ordinal") { opt.importOrdinal = static_cast<uint16_t>(std::stoul(v)); opt.useOrdinal = true; }
            else if (a == "--out") opt.outPath = v;
            else if (a == "--section") opt.sectionName = v.substr(0, 8);
        } else if (!a.empty() && a[0] == '-') {
            std::cerr << "Unknown option: " << a << "\n";
            return false;
        } else if (opt.target.empty()) {
            opt.target = a;
        } else {
            std::cerr << "Unexpected argument: " << a << "\n";
            return false;
        }
    }
    if (opt.target.empty()) { PrintUsage(argv[0]); return false; }
    if (opt.outPath.empty()) opt.outPath = opt.target;
    return true;
}

uint32_t AlignUp(uint32_t value, uint32_t align) {
    if (align == 0) return value;
    return (value + align - 1) / align * align;
}

bool ReadFile(const std::string& path, std::vector<uint8_t>& out) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) return false;
    std::streamsize sz = f.tellg();
    if (sz <= 0) return false;
    f.seekg(0, std::ios::beg);
    out.resize(static_cast<size_t>(sz));
    return static_cast<bool>(f.read(reinterpret_cast<char*>(out.data()), sz));
}

bool WriteFile(const std::string& path, const std::vector<uint8_t>& data) {
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    if (!f) return false;
    f.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    return static_cast<bool>(f);
}

template <typename T>
T ReadAt(const std::vector<uint8_t>& b, size_t off) {
    T v{};
    if (off + sizeof(T) > b.size()) return v;
    std::memcpy(&v, b.data() + off, sizeof(T));
    return v;
}

template <typename T>
void WriteAt(std::vector<uint8_t>& b, size_t off, T v) {
    if (off + sizeof(T) > b.size()) b.resize(off + sizeof(T));
    std::memcpy(b.data() + off, &v, sizeof(T));
}

void AppendBytes(std::vector<uint8_t>& b, const void* data, size_t n) {
    const auto* p = static_cast<const uint8_t*>(data);
    b.insert(b.end(), p, p + n);
}

void AppendCString(std::vector<uint8_t>& b, const std::string& s) {
    AppendBytes(b, s.c_str(), s.size() + 1);
}

void PadTo(std::vector<uint8_t>& b, size_t n) {
    if (b.size() < n) b.resize(n, 0);
}

struct PeView {
    bool     is64 = false;
    size_t   ntOff = 0;
    uint16_t numSections = 0;
    uint16_t optSize = 0;
    uint32_t sectionAlign = 0;
    uint32_t fileAlign = 0;
    size_t   dataDirOff = 0;   // offset of first IMAGE_DATA_DIRECTORY in file buffer
    size_t   secTableOff = 0;
};

bool ParsePE(const std::vector<uint8_t>& buf, PeView& pe, std::string& err) {
    if (buf.size() < sizeof(IMAGE_DOS_HEADER)) { err = "File too small for DOS header."; return false; }
    auto dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(buf.data());
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) { err = "Not a valid PE file (missing MZ)."; return false; }
    pe.ntOff = static_cast<size_t>(dos->e_lfanew);
    if (pe.ntOff + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) > buf.size()) { err = "Truncated NT headers."; return false; }
    if (ReadAt<DWORD>(buf, pe.ntOff) != IMAGE_NT_SIGNATURE) { err = "Missing PE signature."; return false; }
    auto& fh = *reinterpret_cast<const IMAGE_FILE_HEADER*>(buf.data() + pe.ntOff + 4);
    pe.numSections = fh.NumberOfSections;
    pe.optSize = fh.SizeOfOptionalHeader;
    size_t optOff = pe.ntOff + 4 + sizeof(IMAGE_FILE_HEADER);
    if (optOff + pe.optSize > buf.size()) { err = "Truncated optional header."; return false; }
    uint16_t magic = ReadAt<uint16_t>(buf, optOff);
    if (magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        pe.is64 = true;
        auto& oh = *reinterpret_cast<const IMAGE_OPTIONAL_HEADER64*>(buf.data() + optOff);
        pe.sectionAlign = oh.SectionAlignment;
        pe.fileAlign = oh.FileAlignment;
        pe.dataDirOff = optOff + offsetof(IMAGE_OPTIONAL_HEADER64, DataDirectory);
    } else if (magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
        pe.is64 = false;
        auto& oh = *reinterpret_cast<const IMAGE_OPTIONAL_HEADER32*>(buf.data() + optOff);
        pe.sectionAlign = oh.SectionAlignment;
        pe.fileAlign = oh.FileAlignment;
        pe.dataDirOff = optOff + offsetof(IMAGE_OPTIONAL_HEADER32, DataDirectory);
    } else {
        err = "Unknown optional header magic (not PE32/PE32+).";
        return false;
    }
    pe.secTableOff = optOff + pe.optSize;
    if (pe.secTableOff + static_cast<size_t>(pe.numSections) * sizeof(IMAGE_SECTION_HEADER) > buf.size()) {
        err = "Truncated section table.";
        return false;
    }
    if (pe.sectionAlign == 0 || pe.fileAlign == 0) { err = "Invalid PE alignment values."; return false; }
    return true;
}

// RVAToFileOffset: convert RVA to raw file offset using section table.
size_t RVAToFileOffset(const std::vector<uint8_t>& buf, const PeView& pe, uint32_t rva) {
    // Check headers region (RVA < first section VA / SizeOfHeaders conceptually)
    // Try sections first.
    for (uint16_t i = 0; i < pe.numSections; ++i) {
        const auto* sh = reinterpret_cast<const IMAGE_SECTION_HEADER*>(
            buf.data() + pe.secTableOff + i * sizeof(IMAGE_SECTION_HEADER));
        uint32_t va = sh->VirtualAddress;
        uint32_t vsize = sh->Misc.VirtualSize ? sh->Misc.VirtualSize : sh->SizeOfRawData;
        if (rva >= va && rva < va + vsize) {
            return static_cast<size_t>(sh->PointerToRawData) + (rva - va);
        }
    }
    // Fallback: if within headers
    if (rva < buf.size()) return rva;
    return static_cast<size_t>(-1);
}

bool Patch(const Options& opt) {
    std::vector<uint8_t> buf;
    if (!ReadFile(opt.target, buf)) {
        std::cerr << "Error: cannot read target file: " << opt.target << "\n";
        return false;
    }

    PeView pe{};
    std::string err;
    if (!ParsePE(buf, pe, err)) {
        std::cerr << "Error: " << err << "\n";
        return false;
    }

    if (opt.backup) {
        if (!WriteFile(opt.target + ".bak", buf)) {
            std::cerr << "Warning: failed to create backup.\n";
        } else {
            std::cout << "Backup: " << opt.target << ".bak\n";
        }
    }

    // Check for existing section name collision (informational; we still proceed with a unique-ish approach? no - just warn)
    for (uint16_t i = 0; i < pe.numSections; ++i) {
        const auto* sh = reinterpret_cast<const IMAGE_SECTION_HEADER*>(
            buf.data() + pe.secTableOff + i * sizeof(IMAGE_SECTION_HEADER));
        char name[9] = {};
        std::memcpy(name, sh->Name, 8);
        if (opt.sectionName == name) {
            std::cerr << "Error: section \"" << name << "\" already exists. Aborting to avoid corrupting the file.\n";
            return false;
        }
    }

    // Compute placement for the new section.
    const auto* lastSh = reinterpret_cast<const IMAGE_SECTION_HEADER*>(
        buf.data() + pe.secTableOff + (pe.numSections - 1) * sizeof(IMAGE_SECTION_HEADER));
    uint32_t lastVirtualEnd = lastSh->VirtualAddress + AlignUp(
        lastSh->Misc.VirtualSize ? lastSh->Misc.VirtualSize : lastSh->SizeOfRawData, pe.sectionAlign);
    uint32_t lastRawEnd = lastSh->PointerToRawData + AlignUp(lastSh->SizeOfRawData, pe.fileAlign);

    uint32_t newSectionRVA = AlignUp(lastVirtualEnd, pe.sectionAlign);
    uint32_t newSectionRaw = AlignUp(lastRawEnd, pe.fileAlign);
    if (newSectionRaw < buf.size()) newSectionRaw = AlignUp(static_cast<uint32_t>(buf.size()), pe.fileAlign);

    // Read existing import directory.
    const size_t importDirIdx = IMAGE_DIRECTORY_ENTRY_IMPORT; // 1
    uint32_t oldImportRVA = ReadAt<uint32_t>(buf, pe.dataDirOff + importDirIdx * sizeof(IMAGE_DATA_DIRECTORY));
    uint32_t oldImportSize = ReadAt<uint32_t>(buf, pe.dataDirOff + importDirIdx * sizeof(IMAGE_DATA_DIRECTORY) + 4);

    std::vector<IMAGE_IMPORT_DESCRIPTOR> oldDescs;
    if (oldImportRVA != 0) {
        size_t off = RVAToFileOffset(buf, pe, oldImportRVA);
        if (off == static_cast<size_t>(-1) || off + sizeof(IMAGE_IMPORT_DESCRIPTOR) > buf.size()) {
            std::cerr << "Error: cannot locate existing import directory in file.\n";
            return false;
        }
        while (off + sizeof(IMAGE_IMPORT_DESCRIPTOR) <= buf.size()) {
            IMAGE_IMPORT_DESCRIPTOR d{};
            std::memcpy(&d, buf.data() + off, sizeof(d));
            off += sizeof(IMAGE_IMPORT_DESCRIPTOR);
            bool allZero = (d.OriginalFirstThunk == 0 && d.TimeDateStamp == 0 &&
                            d.ForwarderChain == 0 && d.Name == 0 && d.FirstThunk == 0);
            if (allZero) break;
            // Check if this DLL is already present (case-insensitive) to avoid duplicates.
            size_t nameOff = RVAToFileOffset(buf, pe, d.Name);
            if (nameOff != static_cast<size_t>(-1) && nameOff < buf.size()) {
                std::string existing(reinterpret_cast<const char*>(buf.data() + nameOff));
                std::string exLower = existing, wantLower = opt.dllName;
                for (auto& c : exLower) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
                for (auto& c : wantLower) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
                if (exLower == wantLower) {
                    std::cerr << "Error: import for \"" << existing << "\" already exists in the target. Aborting.\n";
                    return false;
                }
            }
            oldDescs.push_back(d);
        }
    } else {
        std::cout << "Note: target has no existing import directory; creating a new one.\n";
    }

    // Build .newimp section content.
    // Layout:
    //   [IMAGE_IMPORT_DESCRIPTOR ... old entries ...] [new descriptor] [zero descriptor]
    //   [INT thunk(s) + null terminator thunk]
    //   [IAT thunk(s) + null terminator thunk]
    //   [IMAGE_IMPORT_BY_NAME for name import, if used]
    //   [DLL name string]
    //   [padding to FileAlignment]
    std::vector<uint8_t> sec;
    sec.clear();

    const size_t descArrayOff = 0;
    const size_t descCount = oldDescs.size() + 1; // +1 for our new descriptor (+1 zero terminator written after)
    const size_t descBytes = (descCount + 1) * sizeof(IMAGE_IMPORT_DESCRIPTOR);

    // Reserve space for descriptors
    sec.resize(descBytes, 0);

    // INT
    size_t intOff = sec.size();
    if (pe.is64) {
        uint64_t thunk = 0;
        if (opt.useOrdinal) thunk = IMAGE_ORDINAL_FLAG64 | static_cast<uint64_t>(opt.importOrdinal);
        else thunk = 0; // filled with RVA later
        AppendBytes(sec, &thunk, 8);
        uint64_t zero = 0; AppendBytes(sec, &zero, 8);
    } else {
        uint32_t thunk = 0;
        if (opt.useOrdinal) thunk = IMAGE_ORDINAL_FLAG32 | static_cast<uint32_t>(opt.importOrdinal);
        else thunk = 0;
        AppendBytes(sec, &thunk, 4);
        uint32_t zero = 0; AppendBytes(sec, &zero, 4);
    }

    // IAT (separate copy; loader may write resolved addresses into IAT)
    size_t iatOff = sec.size();
    if (pe.is64) {
        uint64_t thunk = 0;
        if (opt.useOrdinal) thunk = IMAGE_ORDINAL_FLAG64 | static_cast<uint64_t>(opt.importOrdinal);
        else thunk = 0;
        AppendBytes(sec, &thunk, 8);
        uint64_t zero = 0; AppendBytes(sec, &zero, 8);
    } else {
        uint32_t thunk = 0;
        if (opt.useOrdinal) thunk = IMAGE_ORDINAL_FLAG32 | static_cast<uint32_t>(opt.importOrdinal);
        else thunk = 0;
        AppendBytes(sec, &thunk, 4);
        uint32_t zero = 0; AppendBytes(sec, &zero, 4);
    }

    // Hint/Name
    size_t hintNameOff = 0;
    if (!opt.useOrdinal) {
        // Align to 2 for Hint
        if (sec.size() % 2) sec.push_back(0);
        hintNameOff = sec.size();
        uint16_t hint = 0;
        AppendBytes(sec, &hint, 2);
        AppendCString(sec, opt.importName);
        // Patch INT and IAT thunk RVAs to point to Hint/Name
        uint32_t hnRVA = newSectionRVA + static_cast<uint32_t>(hintNameOff);
        if (pe.is64) {
            uint64_t v = static_cast<uint64_t>(hnRVA);
            std::memcpy(sec.data() + intOff, &v, 8);
            std::memcpy(sec.data() + iatOff, &v, 8);
        } else {
            uint32_t v = hnRVA;
            std::memcpy(sec.data() + intOff, &v, 4);
            std::memcpy(sec.data() + iatOff, &v, 4);
        }
    }

    // DLL name
    size_t dllNameOff = sec.size();
    AppendCString(sec, opt.dllName);

    // Write descriptor array
    size_t cursor = descArrayOff;
    for (const auto& d : oldDescs) {
        std::memcpy(sec.data() + cursor, &d, sizeof(d));
        cursor += sizeof(IMAGE_IMPORT_DESCRIPTOR);
    }
    IMAGE_IMPORT_DESCRIPTOR nd{};
    nd.OriginalFirstThunk = newSectionRVA + static_cast<uint32_t>(intOff);
    nd.FirstThunk         = newSectionRVA + static_cast<uint32_t>(iatOff);
    nd.Name               = newSectionRVA + static_cast<uint32_t>(dllNameOff);
    std::memcpy(sec.data() + cursor, &nd, sizeof(nd));
    cursor += sizeof(IMAGE_IMPORT_DESCRIPTOR);
    // zero terminator already present (sec was zeroed/resized)

    uint32_t newImportSize = static_cast<uint32_t>(descBytes);
    uint32_t virtualSize = static_cast<uint32_t>(sec.size());
    uint32_t rawSize = AlignUp(virtualSize, pe.fileAlign);
    PadTo(sec, rawSize);

    // Build new file: original bytes padded to newSectionRaw + new section + appended section header
    std::vector<uint8_t> out = buf;
    PadTo(out, newSectionRaw);
    AppendBytes(out, sec.data(), sec.size());

    // Re-parse offsets in 'out' context? Offsets we need: data dirs, section table, file header, optional header.
    // They are in the header area which we haven't moved; safe to patch 'out' directly using pe offsets.
    // 1) Update Import Directory data dir -> new section
    WriteAt<uint32_t>(out, pe.dataDirOff + importDirIdx * sizeof(IMAGE_DATA_DIRECTORY),
                      newSectionRVA);
    WriteAt<uint32_t>(out, pe.dataDirOff + importDirIdx * sizeof(IMAGE_DATA_DIRECTORY) + 4,
                      newImportSize);

    // 2) Clear Bound Import directory (index 11) to avoid stale bound-import issues
    WriteAt<uint32_t>(out, pe.dataDirOff + IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT * sizeof(IMAGE_DATA_DIRECTORY), 0);
    WriteAt<uint32_t>(out, pe.dataDirOff + IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT * sizeof(IMAGE_DATA_DIRECTORY) + 4, 0);

    // 3) Append new section header at end of section table
    IMAGE_SECTION_HEADER nsh{};
    std::memset(&nsh, 0, sizeof(nsh));
    std::memcpy(nsh.Name, opt.sectionName.c_str(), std::min<size_t>(opt.sectionName.size(), 8));
    nsh.Misc.VirtualSize = virtualSize;
    nsh.VirtualAddress = newSectionRVA;
    nsh.SizeOfRawData = rawSize;
    nsh.PointerToRawData = newSectionRaw;
    nsh.Characteristics = IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;

    size_t newShOff = pe.secTableOff + static_cast<size_t>(pe.numSections) * sizeof(IMAGE_SECTION_HEADER);
    // Ensure there's room for the new header (must be within SizeOfHeaders)
    uint32_t sizeOfHeaders = 0;
    if (pe.is64) sizeOfHeaders = ReadAt<uint32_t>(out, pe.ntOff + 24 + offsetof(IMAGE_OPTIONAL_HEADER64, SizeOfHeaders));
    else         sizeOfHeaders = ReadAt<uint32_t>(out, pe.ntOff + 24 + offsetof(IMAGE_OPTIONAL_HEADER32, SizeOfHeaders));

    if (newShOff + sizeof(IMAGE_SECTION_HEADER) > sizeOfHeaders) {
        std::cerr << "Error: no room in PE headers for a new section header (SizeOfHeaders="
                  << sizeOfHeaders << ", needed offset " << (newShOff + sizeof(IMAGE_SECTION_HEADER)) << ").\n"
                  << "This file cannot be patched safely without relocating headers. Aborting.\n";
        return false;
    }
    WriteAt(out, newShOff, nsh);

    // 4) Increment NumberOfSections
    uint16_t nsec = ReadAt<uint16_t>(out, pe.ntOff + 4 + offsetof(IMAGE_FILE_HEADER, NumberOfSections));
    WriteAt<uint16_t>(out, pe.ntOff + 4 + offsetof(IMAGE_FILE_HEADER, NumberOfSections), static_cast<uint16_t>(nsec + 1));

    // 5) Update SizeOfImage
    uint32_t newSizeOfImage = AlignUp(newSectionRVA + virtualSize, pe.sectionAlign);
    if (pe.is64)
        WriteAt<uint32_t>(out, pe.ntOff + 24 + offsetof(IMAGE_OPTIONAL_HEADER64, SizeOfImage), newSizeOfImage);
    else
        WriteAt<uint32_t>(out, pe.ntOff + 24 + offsetof(IMAGE_OPTIONAL_HEADER32, SizeOfImage), newSizeOfImage);

    // 6) Clear IMAGE_DIRECTORY_ENTRY_IAT bound? Leave as-is is fine; loader rebuilds.
    // 7) If PE checksum exists (CheckSum field), zero it (non-driver PEs don't require it).
    if (pe.is64)
        WriteAt<uint32_t>(out, pe.ntOff + 24 + offsetof(IMAGE_OPTIONAL_HEADER64, CheckSum), 0);
    else
        WriteAt<uint32_t>(out, pe.ntOff + 24 + offsetof(IMAGE_OPTIONAL_HEADER32, CheckSum), 0);

    if (!WriteFile(opt.outPath, out)) {
        std::cerr << "Error: failed to write output: " << opt.outPath << "\n";
        return false;
    }

    std::cout << "OK: static import injected.\n"
              << "  Target:  " << opt.target << "\n"
              << "  Output:  " << opt.outPath << "\n"
              << "  DLL:     " << opt.dllName << "\n"
              << "  Import:  " << (opt.useOrdinal ? ("ordinal " + std::to_string(opt.importOrdinal)) : opt.importName) << "\n"
              << "  Section: " << opt.sectionName << " (RVA 0x" << std::hex << newSectionRVA
              << ", raw 0x" << newSectionRaw << std::dec << ", size " << rawSize << ")\n"
              << "  Import dir -> RVA 0x" << std::hex << newSectionRVA << std::dec << "\n"
              << "\nEnsure the DLL exports the referenced symbol, or the target will fail to load.\n";
    return true;
}

} // namespace

int main(int argc, char** argv) {
    Options opt;
    bool showHelp = false;
    if (!ParseArgs(argc, argv, opt, showHelp)) {
        return showHelp ? 0 : 1;
    }
    return Patch(opt) ? 0 : 1;
}
