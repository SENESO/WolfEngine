#pragma once
#include "framework.h"

class MacroDetector
{
public:
    static MacroDetector& Instance()
    {
        static MacroDetector instance;
        return instance;
    }

    bool Initialize(HINSTANCE hInstance);
    void Uninitialize();

    static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);

private:
    MacroDetector() : m_hook(NULL), m_hInstance(NULL) {}
    HHOOK m_hook;
    HINSTANCE m_hInstance;

    static DWORD s_lastClickTime;
    static int s_rapidClickCount;
};

