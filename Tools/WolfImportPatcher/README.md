# WolfImportPatcher

Static PE import injection (IAT patching) for `WolfEngine.dll` or any DLL.

This tool rewrites a target PE so the Windows PE Loader (`ntdll!LdrLoadDll`) loads the DLL **before** the target's `AddressOfEntryPoint`, with no runtime injector.

## What it does (matching the `.newimp` method)

1. Appends a new section (default `.newimp`) aligned to the PE's `SectionAlignment`/`FileAlignment`.
2. Copies all existing `IMAGE_IMPORT_DESCRIPTOR` entries into `.newimp` and appends a new descriptor for the target DLL.
3. Builds the new descriptor's `INT` (`OriginalFirstThunk`), `IAT` (`FirstThunk`), hint/name (or ordinal thunk), and DLL name inside `.newimp`.
4. Redirects `IMAGE_DIRECTORY_ENTRY_IMPORT` to the new descriptor array and clears the Bound Import directory.
5. Adds the new section header (requires free space in `SizeOfHeaders`), increments `NumberOfSections`, updates `SizeOfImage`, and zeros the PE checksum.

Supports PE32 (x86) and PE32+ (x64).

## Build (Visual Studio, Win32/x64 console)

```bat
cl /EHsc /std:c++17 /O2 /W3 WolfImportPatcher.cpp /Fe:WolfImportPatcher.exe
```

Or add `WolfImportPatcher.cpp` as a Console Application in the solution (v145, C++17), matching the other tools.

## Usage

```text
WolfImportPatcher <target.exe> [options]

  --dll <name>        DLL to force-load (default: WolfEngine.dll)
  --import <name>     Export function name to reference (default: WolfEntry)
  --ordinal <n>       Import by ordinal instead of by name
  --out <file>        Write result to <file> (default: modify target in-place)
  --section <name>    New section name, max 8 chars (default: .newimp)
  --backup            Save a copy as <target>.bak before modifying
```

### Examples

```bat
REM Inject WolfEngine.dll by exported name (recommended: export WolfEntry from the DLL)
WolfImportPatcher Conquer.exe --backup --dll WolfEngine.dll --import WolfEntry

REM Import by ordinal 1
WolfImportPatcher Conquer.exe --backup --dll WolfEngine.dll --ordinal 1

REM Write to a new file instead of modifying in place
WolfImportPatcher Conquer.exe --out Conquer_patched.exe --dll WolfEngine.dll --import WolfEntry
```

## Important: the import symbol must exist

The PE loader resolves the referenced import. If `WolfEngine.dll` does not export the chosen name/ordinal, the target process will fail to start (missing import/entry point).

`WolfEngine.def` currently exports only `_DllMain@12`. To make static injection reliable, export a plain anchor function from `WolfEngine.dll`, for example:

```c
// in a .cpp
extern "C" __declspec(dllexport) void WolfEntry() {}
```

and/or add `WolfEntry` to `WolfEngine.def`, then use `--import WolfEntry`. (`DllMain` runs on load regardless; the imported symbol just satisfies the loader dependency.)

## Limitations / safety

- Refuses to run if the section name already exists, if the DLL is already imported, or if there is no room in `SizeOfHeaders` for another section header (to avoid corrupting packed/protected files).
- Files protected/virtualized (e.g. a `.vlizer`-packed image) may have constrained headers; patching them still works in many cases but depends on available header space and loader compatibility.
- Always keep a backup (`--backup`) and test the patched binary in a safe environment.
