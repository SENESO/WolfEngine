#pragma once

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable: 4996)

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <tlhelp32.h>
#include <wtsapi32.h>
#include <iphlpapi.h>
#include <shellapi.h>
#include <wincrypt.h>
#include <psapi.h>

#include "detours.h"

#include <string>
#include <vector>
#include <map>
#include <set>
#include <thread>
#include <atomic>
#include <chrono>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "wtsapi32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "psapi.lib")
