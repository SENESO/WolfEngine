#pragma once
#include "framework.h"

// Forward declarations for graphic.dll structs
struct CMySize
{
    int cx;
    int cy;
};

struct CMyPos
{
    int x;
    int y;
};

struct C3_SIZE
{
    int cx;
    int cy;
};

struct C3_POS
{
    int x;
    int y;
};

enum RENDER_TEXT_STYLE
{
    TEXT_STYLE_NORMAL = 0,
    TEXT_STYLE_SHADOW = 1,
    TEXT_STYLE_BORDER = 2,
    TEXT_STYLE_GLOW   = 3
};

class GraphicHook
{
public:
    static GraphicHook& Instance()
    {
        static GraphicHook instance;
        return instance;
    }

    bool Initialize(bool isDX9);

    // Original function pointers
    typedef CMySize (*FnShowStringW_DX8)(int, int, unsigned long, const wchar_t*, const char*, int, int, RENDER_TEXT_STYLE, unsigned long, CMyPos&);
    typedef C3_SIZE (*FnShowStringW_DX9)(int, int, unsigned long, const wchar_t*, const char*, int, bool, RENDER_TEXT_STYLE, unsigned long, C3_POS);
    typedef void (*FnShowBlock)(int, int, int, int, unsigned long);

    static FnShowStringW_DX8 s_OrigShowStringW_DX8;
    static FnShowStringW_DX9 s_OrigShowStringW_DX9;
    static FnShowBlock s_OrigShowBlock;

    // Hooks
    static CMySize HookShowStringW_DX8(int x, int y, unsigned long color, const wchar_t* text, const char* font, int size, int style, RENDER_TEXT_STYLE renderStyle, unsigned long param9, CMyPos& pos);
    static C3_SIZE HookShowStringW_DX9(int x, int y, unsigned long color, const wchar_t* text, const char* font, int size, bool flag, RENDER_TEXT_STYLE renderStyle, unsigned long param9, C3_POS pos);

    // In-game HUD Ping/FPS string interceptor
    static void ProcessHudStatusString(wchar_t* buffer, size_t maxLen);

private:
    GraphicHook() : m_hGraphic(NULL), m_isDX9(false) {}
    HMODULE m_hGraphic;
    bool m_isDX9;
};

