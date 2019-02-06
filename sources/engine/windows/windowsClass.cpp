#include "windowsClass.hpp"

#include "Window.hpp"

#include <stdexcept>

namespace Windows
{

    static LRESULT MainWindowCB(HWND hwnd, UINT Message, WPARAM WParam, LPARAM LParam)
    {
        LRESULT Result = 0;

        switch (Message)
        {
        case WM_CLOSE:
            // if (MessageBoxA(Window, "Are you sure?", "Quit Application", MB_YESNO | MB_ICONQUESTION) == IDYES)
            {
                DestroyWindow(hwnd);
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_PAINT:
        {
            auto& window = *reinterpret_cast<Window*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
            window.draw();
        }
        break;

        default:
            Result = DefWindowProcA(hwnd, Message, WParam, LParam);
            break;
        }

        return Result;
    }

    static ATOM registerClass(HINSTANCE instance)
    {
        constexpr const char* const windowClassName = "EngineWindowClass";

        WNDCLASSA wndClass     = {};
        wndClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wndClass.lpfnWndProc   = MainWindowCB;
        wndClass.hInstance     = instance;
        wndClass.lpszClassName = windowClassName;
        return RegisterClassA(&wndClass);
    }

    WindowClass::WindowClass()
        : instance{ static_cast<HINSTANCE>(GetModuleHandleA(NULL)) }
        , atom{ registerClass(instance) }
    {
        if (!atom)
        {
            throw std::domain_error{ "Fail to register window class" };
        }
    }
    WindowClass::~WindowClass() { UnregisterClassA(GetClassname(), instance); }

    HWND WindowClass::createNativeWindow() const
    {
        constexpr const char* const windowName = "Game";

        return CreateWindowExA(0,
                               GetClassname(),
                               windowName,
                               WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                               CW_USEDEFAULT,
                               CW_USEDEFAULT,
                               CW_USEDEFAULT,
                               CW_USEDEFAULT,
                               0,
                               0,
                               instance,
                               0);
    }

} // namespace Windows