#pragma once
#include "framework.h"

class TQDatHook
{
public:
    static TQDatHook& Instance()
    {
        static TQDatHook instance;
        return instance;
    }

    bool Initialize();

    typedef void (*FnTQDatInit)(void* param1, int param2, int param3, ...);
    static FnTQDatInit s_OrigTQDatInit;

private:
    TQDatHook() : m_hTQDat(NULL) {}
    HMODULE m_hTQDat;
};

