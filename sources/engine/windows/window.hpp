#pragma once

#include <Windows.h>

namespace Windows
{
    class BackBuffer;

    class Window final
    {
        friend class Runner;

    public:
        Window(const Window&) = delete; // non copiable
        Window(Window&&)      = delete;
        ~Window();

        void blitBackBuffer() const { blitBackBuffer(hdc); }

        void draw() const;

        HWND getHandle() const { return hwnd; }
        HDC  getDeviceContext() const { return hdc; }

    private:
        Window(HWND hwnd, BackBuffer& backbuffer);

        void blitBackBuffer(HDC _hdc) const;

        // since we specified CS_OWNDC, we can just get one device context and use it forever

        // order matters
        const HWND        hwnd;
        const HDC         hdc; // depends on hwnd
        const BackBuffer& backbuffer;
    };

} // namespace Windows