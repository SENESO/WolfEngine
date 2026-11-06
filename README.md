# WolfEngine — Full Source Recovery

This repository contains the completely reverse-engineered and reconstructed C++ source code for **WolfEngine.dll** (from the `ESOWolf` / `Open Source [WolfEngine]` suite for Conquer Online).

## Architecture Overview

```
WolfEngine/
├── WolfEngine.sln                    // Visual Studio Solution
├── WolfEngine.vcxproj                // Visual Studio C++ Project (x86 Dynamic Library)
├── WolfEngine.def                    // Exports _DllMain@12
├── README.md                         // Full subsystem documentation
├── Include/
│   ├── framework.h                   // Win32 / Winsock / Crypt / Toolhelp headers
│   ├── pch.h                         // Precompiled header
│   ├── WolfEngine.h                  // Core engine singleton orchestrator
│   ├── Environment.h                 // Process validation (Conquer.exe) & DirectX mode (DX8 vs DX9)
│   ├── HookManager.h                 // Memory scanning & inline/detour hooking helpers
│   ├── GraphicHook.h                 // graphic.dll ShowStringW / ShowBlock & Ping/FPS HUD modification
│   ├── ShellHook.h                   // Shell32.dll ShellExecuteA hook to suppress browser popups
│   ├── TQDatHook.h                   // TQDat.dll TQDatInit hook with key seed parameters
│   ├── NdacHook.h                    // ndac.dll & socket closesocket hook
│   ├── FileIntegrity.h               // MD5 hash calculator for client files (welcome hash)
│   ├── NetworkManager.h              // Client/server auth handshake & cheat packet transmission
│   ├── AntiCheat.h                   // Anti-cheat coordinator
│   ├── ProcessScanner.h              // Process monitor blacklist (>100 debuggers, trainers, macro tools)
│   ├── MacroDetector.h               // Low-level mouse hook (WH_MOUSE_LL) detecting click macros & injected events
│   ├── MemoryIntegrity.h             // Anti-debug, SendInput hook detection, and memory scanner
│   ├── DiscordRPC.h                  // Discord IPC client (\\?\pipe\discord-ipc-0..9, App ID 1230904445949186220)
│   ├── SteamProtocol.h               // SteamExe registry reader & custom URL protocol handler
│   └── LuaManager.h                  // Embedded Lua script executor & GUI.ini patcher
├── Source/
│   ├── DllMain.cpp                   // DLL entry point (_DllMain@12)
│   ├── WolfEngine.cpp                // Engine lifecycle management
│   ├── Environment.cpp               // Conquer.exe & env_dx9 path detection
│   ├── HookManager.cpp               // Pattern scanning and byte patching
│   ├── GraphicHook.cpp               // CMyBitmap font and HUD status string interceptor
│   ├── ShellHook.cpp                 // ShellExecuteA blocker
│   ├── TQDatHook.cpp                 // TQDat hook implementation
│   ├── NdacHook.cpp                  // Ndac hook implementation
│   ├── FileIntegrity.cpp             // MD5 computation for Conquer client files
│   ├── NetworkManager.cpp            // Welcome hash (packet 1) & cheat alert (packet 2)
│   ├── ProcessScanner.cpp            // WTS process enumerator & blacklist matcher
│   ├── MacroDetector.cpp             // Mouse timing and LLMHF_INJECTED filter
│   ├── MemoryIntegrity.cpp           // CheckRemoteDebuggerPresent & SendInput prologue checks
│   ├── DiscordRPC.cpp                // Named pipe client & JSON RPC presence sender
│   ├── SteamProtocol.cpp             // Software\Classes protocol registration
│   ├── LuaManager.cpp                // GUI.ini / Spirit.lua loader
│   └── pch.cpp                       // Precompiled header generator
└── Lua/
    └── Spirit.lua                    // Verbatim recovered Dragon Soul / Spirit system Lua script
```

## Subsystem Details

### 1. Environment & Process Validation
- **Binary Check:** Asserts that the loading process is `Conquer.exe`. Any foreign injector or test executable is terminated.
- **DirectX Detection:** Checks if the path includes `env_dx9` to toggle between DirectX 8 and DirectX 9 function signatures.

### 2. Client File Integrity & Handshake
- Computes MD5 checksums across:
  - `Env_DX8/Conquer.exe` and `Env_DX9/Conquer.exe`
  - `ini/MagicType.dat`
  - `WolfEngine.dat`
  - `ini/MagicEffect.ini`
  - `ini/Server_Cn_Res.ini`
  - `WolfEngine.dll`
- Packs these into a 400-byte welcome payload (Opcode `1`) sent on initial connection.
- Server validates and replies:
  - `VALID` -> continues loading.
  - `BANNED` -> shows ban notice and terminates.
  - Mismatch -> shows client modification alert and exits.

### 3. Graphics & In-Game Overlay
- Intercepts `graphic.dll` `CMyBitmap::ShowStringW` (both DX8 and DX9 exports).
- Formats in-game status bar: `[%s] (%03d,%03d) Ping: %s; Fps: %02d; FpsAver : %02d  %02d/%02d/%d %s %02d:%02d`.
- Injects custom color coding for low vs high ping thresholds.

### 4. Anti-Cheat & Process Monitor
- Scans running processes via `WTSEnumerateProcessesW` against known hacking tools (`CheatEngine.exe`, `x64dbg.exe`, `IDAPro.exe`, `ProcessHacker.exe`, `ArtMoney.exe`, `AutoClicker.exe`, `TinyTask.exe`, etc.).
- Whitelists critical OS processes (`csrss.exe`, `services.exe`, `ekrn.exe`, `lsass.exe`, `smss.exe`).
- Monitors low-level mouse input via `WH_MOUSE_LL` to detect rapid clicks (<15ms), locked pushes, and synthetic injected events (`LLMHF_INJECTED`).
- Validates that `SendInput` and network `send` have not been hooked by third-party memory manipulation tools.

### 5. Discord Rich Presence
- Connects to local named pipes `\\?\pipe\discord-ipc-0` through `discord-ipc-9`.
- Publishes presence for Application ID `1230904445949186220` with state `Idle` / `Loading..`, details `Best Conquer Private Server`, and asset `logo512`.

### 6. Embedded Lua Layer
- Embedded `Spirit.lua` handles the Dragon Soul system (`SpiritTypeID`, `SpiritTipsInfo`, `DlgQLUpLev`, `GetEffectByQLTotalLevel`, etc.).
- Automatically patches `../ini/GUI.ini` and `../ini/GUI800X600.ini` to link new UI buttons and animation sections.

## Building with Visual Studio

1. Open `WolfEngine.sln` in Visual Studio 2022 or 2017.
2. Select **Release** or **Debug** with the **Win32 (x86)** platform.
3. Build the solution. The compiled binary will be generated at `bin/Release/WolfEngine.dll`.

