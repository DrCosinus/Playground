#include "window.hpp"

#include "win_backbuffer.hpp"

#include <stdexcept>

static Dimension GetWindowDimension(HWND hwnd)
{
    RECT ClientRect;
    GetClientRect(hwnd, &ClientRect);

    return { ClientRect.right - ClientRect.left, ClientRect.bottom - ClientRect.top };
}

namespace Windows
{

    Window::Window(HWND hwnd, BackBuffer& backbuffer)
        : hwnd{ hwnd }
        , hdc{ GetDC(hwnd) }
        , backbuffer{ backbuffer }
    {
        if (!hwnd)
        {
            throw std::domain_error{ "Fail to create window." };
        }
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    }

    Window::~Window()
    {
        ReleaseDC(hwnd, hdc);
        DestroyWindow(hwnd);
    }

    void Window::blitBackBuffer(HDC _hdc) const
    {
        auto [w, h] = GetWindowDimension(hwnd);
        // TODO: Aspect ratio correction
        // TODO: Play with stretch modes
        StretchDIBits(_hdc,
                      0,
                      0,
                      w,
                      h,
                      0,
                      0,
                      backbuffer.Width,
                      backbuffer.Height,
                      backbuffer.Memory,
                      &backbuffer.Info,
                      DIB_RGB_COLORS,
                      SRCCOPY);
    }

    void Window::draw() const
    {
        PAINTSTRUCT Paint;
        HDC         DeviceContext = BeginPaint(hwnd, &Paint);
        blitBackBuffer(DeviceContext);
        EndPaint(hwnd, &Paint);
    }

} // namespace Windows