#pragma once

#include <Windows.h>

namespace Windows
{
    class WindowClass final
    {
        friend class Runner;

        WindowClass();
        ~WindowClass();

        HWND createNativeWindow() const;

        LPCSTR GetClassname() const { return reinterpret_cast<LPCSTR>(atom); }

        HINSTANCE instance;
        ATOM      atom;
    };
}