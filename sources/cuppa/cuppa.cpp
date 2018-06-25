#include "cuppa.hpp"

#include <windows.h>
// http://www.winprog.org/tutorial/simple_window.html

// ugly global for test purpose
unsigned int width = 240, height = 120;

namespace detail
{
    static constexpr char windowClassName[] = "cuppaWindowClass";

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch(msg)
        {
            case WM_CLOSE:
                DestroyWindow(hwnd);
            break;
            case WM_DESTROY:
                PostQuitMessage(0);
            break;
            default:
                return DefWindowProc(hwnd, msg, wParam, lParam);
        }
        return 0;
    }

    static void RegisterWindowClass(HINSTANCE hInstance)
    {
        WNDCLASSEX wc;
        wc.cbSize        = sizeof(WNDCLASSEX);
        wc.style         = 0;
        wc.lpfnWndProc   = WndProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = 0;
        wc.hInstance     = hInstance;
        wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = windowClassName;
        wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

        if(!RegisterClassEx(&wc))
        {
            MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
            return;
        }
    }

    static void CreateAppWindow(HINSTANCE hInstance)
    {
        HWND hwnd = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            windowClassName,
            "Cuppa",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, width, height,
            NULL, NULL, hInstance, NULL);
        if (hwnd == NULL)
        {
            MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
            return;
        }
        ShowWindow(hwnd, SW_NORMAL);
        UpdateWindow(hwnd);
    }
} // namespace detail

namespace cuppa
{
    void app::run()
    {
        // __argc, __argv
        setup();
        auto hInstance = GetModuleHandle(NULL);
        detail::RegisterWindowClass(hInstance);
        detail::CreateAppWindow(hInstance);

        MSG Msg;
        while(GetMessage(&Msg, NULL, 0, 0) > 0)
        {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
            update();
            draw();
        }
    }

    void app::size(unsigned int _width, unsigned int _height)
    {
        width = _width;
        height = _height;
    }

} // namespace cuppa