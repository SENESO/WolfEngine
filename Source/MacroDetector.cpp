#include "pch.h"
#include "MacroDetector.h"
#include "NetworkManager.h"

DWORD MacroDetector::s_lastClickTime = 0;
int MacroDetector::s_rapidClickCount = 0;

bool MacroDetector::Initialize(HINSTANCE hInstance)
{
    m_hInstance = hInstance;
    m_hook = SetWindowsHookExW(WH_MOUSE_LL, LowLevelMouseProc, hInstance, 0);
    return (m_hook != NULL);
}

void MacroDetector::Uninitialize()
{
    if (m_hook)
    {
        UnhookWindowsHookEx(m_hook);
        m_hook = NULL;
    }
}

LRESULT CALLBACK MacroDetector::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        HWND foreground = GetForegroundWindow();
        DWORD foregroundPid = 0;
        GetWindowThreadProcessId(foreground, &foregroundPid);

        if (foregroundPid == GetCurrentProcessId())
        {
            MSLLHOOKSTRUCT* pMouse = (MSLLHOOKSTRUCT*)lParam;

            // 1. Detect injected mouse events (software macros / auto-clickers)
            if (pMouse->flags & (LLMHF_INJECTED | LLMHF_LOWER_IL_INJECTED))
            {
                NetworkManager::Instance().SendCheatReport("Detect script on mouse (Injected)");
                MessageBoxA(NULL, "Detect script on mouse.", "Macro Detected", MB_OK | MB_ICONERROR);
                ExitProcess(0);
            }

            // 2. Validate allowed buttons
            if (wParam != WM_MOUSEMOVE &&
                wParam != WM_LBUTTONDOWN && wParam != WM_LBUTTONUP &&
                wParam != WM_RBUTTONDOWN && wParam != WM_RBUTTONUP &&
                wParam != WM_MBUTTONDOWN && wParam != WM_MBUTTONUP &&
                wParam != WM_MOUSEWHEEL)
            {
                NetworkManager::Instance().SendCheatReport("Detect invalid button on mouse");
                MessageBoxA(NULL, "Detect invalid button on mouse.", "Macro Detected", MB_OK | MB_ICONERROR);
                ExitProcess(0);
            }

            // 3. Fast auto-clicker detection on right click
            if (wParam == WM_RBUTTONDOWN)
            {
                DWORD now = GetTickCount();
                DWORD diff = now - s_lastClickTime;
                s_lastClickTime = now;

                if (diff < 15) // Impossible human click rate (< 15ms)
                {
                    s_rapidClickCount++;
                    if (s_rapidClickCount > 5)
                    {
                        NetworkManager::Instance().SendCheatReport("Detect Macro (locked push)");
                        MessageBoxA(NULL, "Detect Macro (locked push).", "Macro Detected", MB_OK | MB_ICONERROR);
                        ExitProcess(0);
                    }
                }
                else
                {
                    s_rapidClickCount = 0;
                }
            }
        }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

