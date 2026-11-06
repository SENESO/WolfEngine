#pragma once
#include "framework.h"

class NdacHook
{
public:
    static NdacHook& Instance()
    {
        static NdacHook instance;
        return instance;
    }

    bool Initialize();

    typedef int (WSAAPI *FnCloseSocket)(SOCKET s);
    static FnCloseSocket s_OrigCloseSocket;
    static int WSAAPI HookCloseSocket(SOCKET s);

private:
    NdacHook() : m_hNdac(NULL) {}
    HMODULE m_hNdac;
};

