#include "pch.h"
#include "GraphicHook.h"
#include "HookManager.h"

GraphicHook::FnShowStringW_DX8 GraphicHook::s_OrigShowStringW_DX8 = nullptr;
GraphicHook::FnShowStringW_DX9 GraphicHook::s_OrigShowStringW_DX9 = nullptr;
GraphicHook::FnShowBlock GraphicHook::s_OrigShowBlock = nullptr;

bool GraphicHook::Initialize(bool isDX9)
{
    m_isDX9 = isDX9;
    m_hGraphic = GetModuleHandleA("graphic.dll");
    if (!m_hGraphic)
    {
        m_hGraphic = LoadLibraryA("graphic.dll");
        if (!m_hGraphic) return false;
    }

    s_OrigShowBlock = (FnShowBlock)GetProcAddress(m_hGraphic, "?ShowBlock@CMyBitmap@@SAXHHHHK@Z");

    if (!m_isDX9)
    {
        s_OrigShowStringW_DX8 = (FnShowStringW_DX8)GetProcAddress(m_hGraphic,
            "?ShowStringW@CMyBitmap@@SA?AUCMySize@@HHKPBGPBDHHW4RENDER_TEXT_STYLE@@KAAUCMyPos@@@Z");
        if (s_OrigShowStringW_DX8)
        {
            HookManager::Instance().AttachHook((PVOID*)&s_OrigShowStringW_DX8, (PVOID)HookShowStringW_DX8);
        }
    }
    else
    {
        s_OrigShowStringW_DX9 = (FnShowStringW_DX9)GetProcAddress(m_hGraphic,
            "?ShowStringW@CMyBitmap@@SA?AUC3_SIZE@@HHKPBGPBDH_NW4RENDER_TEXT_STYLE@@KUC3_POS@@@Z");
        if (s_OrigShowStringW_DX9)
        {
            HookManager::Instance().AttachHook((PVOID*)&s_OrigShowStringW_DX9, (PVOID)HookShowStringW_DX9);
        }
    }

    return true;
}

CMySize GraphicHook::HookShowStringW_DX8(int x, int y, unsigned long color, const wchar_t* text,
    const char* font, int size, int style, RENDER_TEXT_STYLE renderStyle, unsigned long param9, CMyPos& pos)
{
    // Filter and enhance special colored strings
    if (color == 0xFFFFCD7 && size == 13)
    {
        color = 0xFFFFFCD7;
    }
    else if (color == 0xFF000000 && size == 17)
    {
        color = 0xFF000000;
    }

    if (s_OrigShowStringW_DX8)
        return s_OrigShowStringW_DX8(x, y, color, text, font, size, style, renderStyle, param9, pos);

    CMySize defaultSize = { 0, 0 };
    return defaultSize;
}

C3_SIZE GraphicHook::HookShowStringW_DX9(int x, int y, unsigned long color, const wchar_t* text,
    const char* font, int size, bool flag, RENDER_TEXT_STYLE renderStyle, unsigned long param9, C3_POS pos)
{
    if (color == 0xFFFFCD7 && size == 13)
    {
        color = 0xFFFFFCD7;
    }
    else if (color == 0xFF000000 && size == 17)
    {
        color = 0xFF000000;
    }

    if (s_OrigShowStringW_DX9)
        return s_OrigShowStringW_DX9(x, y, color, text, font, size, flag, renderStyle, param9, pos);

    C3_SIZE defaultSize = { 0, 0 };
    return defaultSize;
}

void GraphicHook::ProcessHudStatusString(wchar_t* buffer, size_t maxLen)
{
    if (!buffer) return;

    // Check if status string format
    const wchar_t* pPing = wcsstr(buffer, L"Ping:");
    if (pPing)
    {
        int pingVal = 0;
        if (swscanf_s(pPing, L"Ping: %d", &pingVal) == 1)
        {
            wchar_t pingStr[32] = { 0 };
            swprintf_s(pingStr, L"Ping: %02d", pingVal);

            const wchar_t* pSemi = wcschr(pPing, L';');
            if (pSemi)
            {
                std::wstring remainder = pSemi;
                ptrdiff_t offset = pPing - buffer;
                if ((size_t)offset < maxLen)
                {
                    wcscpy_s(buffer + offset, maxLen - offset, pingStr);
                    wcscat_s(buffer + offset, maxLen - offset, remainder.c_str());
                }
            }
        }
    }
}

