#include "cuppa.hpp"

#include <windows.h>

#include <gdiplus.h>
// sample GDI+: https://codes-sources.commentcamarche.net/source/view/29875/966776#browser

template<auto N>
void Assert(bool _condition, const char (&_message)[N])
{
    if(!_condition)
    {
        MessageBox(NULL, _message, "Assertion error!", MB_ICONEXCLAMATION | MB_OK);
        exit(1);
        return;
    }
}

void Assert(bool _condition)
{
    Assert(_condition, "No description");
}

static constexpr char windowClassName[] = "cuppaWindowClass";

template<typename GRAPHICS_DRIVER_TYPE>
struct MSWindowsDriver
{
    void Setup(cuppa::app& _app)
    {
        app_ = &_app;
        instance = this;
        auto hInstance = GetModuleHandle(NULL);
        RegisterWindowClass(hInstance);
        CreateAppWindow(hInstance);
    }

    void SetWindowSize(unsigned int _width, unsigned int _height)
    {
        // for now, we only handle initial width and height
        // TODO: handle SetWindowSize after the creation of the windows
        width = _width;
        height = _height;
    }

private:
    void Draw()
    {
        PAINTSTRUCT ps;
        auto hdc = BeginPaint(hWnd_, &ps);
        GraphicsDriver.Draw(*app_, hdc);
        EndPaint(hWnd_, &ps);
    }

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch(msg)
        {
            case WM_CREATE:
                if (instance)
                {
                    instance->GraphicsDriver.Init();
                }
                break;
            case WM_PAINT:
                instance->Draw();
                break;
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

    void RegisterWindowClass(HINSTANCE hInstance)
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

    void CreateAppWindow(HINSTANCE hInstance)
    {
        hWnd_ = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            windowClassName,
            "Cuppa",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, width, height,
            NULL, NULL, hInstance, NULL);
        if (hWnd_ == NULL)
        {
            MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
            return;
        }
        ShowWindow(hWnd_, SW_NORMAL);
        UpdateWindow(hWnd_);
    }

    cuppa::app* app_;

    inline static MSWindowsDriver* instance = nullptr;
    GRAPHICS_DRIVER_TYPE GraphicsDriver;
    HWND hWnd_ { 0 };
    unsigned int width = 240, height = 120;
};

struct GdiplusDriver
{
    using Graphics = Gdiplus::Graphics;
    void Init()
    {
        Gdiplus::GdiplusStartup(&token, &startupInput, NULL);
    }

    void Draw(cuppa::app& _app, HDC _hdc)
    {
        Graphics graphics{ _hdc };

        auto fillBrush = Gdiplus::SolidBrush( Gdiplus::Color(255, 0, 0));
        auto rect = Gdiplus::RectF(Gdiplus::PointF{100,1000}, Gdiplus::SizeF{50,50});
        graphics.FillRectangle( &fillBrush, rect);
        _app.draw();
    }

private:
    ULONG_PTR                       token;
    Gdiplus::GdiplusStartupInput    startupInput;
};

static MSWindowsDriver<GdiplusDriver> SystemDriver;

namespace cuppa
{
    void app::run()
    {
        // __argc, __argv
        setup();
        SystemDriver.Setup(*this);
        MSG Msg;
        while(GetMessage(&Msg, NULL, 0, 0) > 0)
        {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
            update();
        }
    }

    void app::size(unsigned int _width, unsigned int _height)
    {
        SystemDriver.SetWindowSize(_width, _height);
    }

} // namespace cuppa