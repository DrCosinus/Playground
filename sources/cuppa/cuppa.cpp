#include "cuppa.hpp"

#include <cstdlib>
#include <windows.h>

#include <memory>

#include <random>

namespace cuppa
{
    void Assert(bool _condition, const char* _message)
    {
        if(!_condition)
        {
            MessageBoxA(NULL, _message, "Assertion error!", MB_ICONEXCLAMATION | MB_OK);
            exit(1);
            return;
        }
    }
} // namespace cuppa

using namespace cuppa;

static constexpr TCHAR windowClassName[] = TEXT("cuppaWindowClass");

struct MSWindowsDriver
{
    void Setup(app& _app)
    {
        auto hInstance = GetModuleHandle(NULL);
        RegisterWindowClass(hInstance);
        CreateAppWindow(hInstance, _app);
    }

    void SetWindowSize(Pixel _width, Pixel _height)
    {
        // for now, we only handle initial width and height
        // TODO: handle SetWindowSize after the creation of the windows
        width = _width;
        height = _height;
    }
    Pixel GetWindowWidth() const { return width; }
    Pixel GetWindowHeight() const { return height; }

private:
    static auto GetAppPtr(HWND hWnd) { return reinterpret_cast<app*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)); }

    void Draw()
    {
        PAINTSTRUCT ps;
        auto hdc = BeginPaint(hWnd_, &ps);
        auto appPtr = GetAppPtr(hWnd_);
        auto gfxDvrItf = appPtr->graphicsDriver.get();
        gfxDvrItf->draw(*appPtr, DeviceContext{hdc});
        EndPaint(hWnd_, &ps);
    }

    void RefreshWindow(HWND _hWnd)
    {
        InvalidateRect(_hWnd, nullptr, FALSE);
        UpdateWindow(_hWnd);
    }

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch(msg)
        {
            case WM_CREATE:
                SetTimer(hwnd, 1234, 16, (TIMERPROC)0);
                break;
            case WM_TIMER:
            // TODO: check the TIMER_ID
                {
                    auto appPtr = GetAppPtr(hwnd);
                    if (appPtr)
                        std::any_cast<MSWindowsDriver*>(appPtr->SystemDriver)->RefreshWindow(hwnd);
                }
                break;
            case WM_PAINT:
                {
                    auto appPtr = GetAppPtr(hwnd);
                    if (appPtr)
                        std::any_cast<MSWindowsDriver*>(appPtr->SystemDriver)->Draw();
                }
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
            MessageBox(NULL, TEXT("Window Registration Failed!"), TEXT("Error!"), MB_ICONEXCLAMATION | MB_OK);
            return;
        }
    }

    void CreateAppWindow(HINSTANCE hInstance, app& _app)
    {
        RECT rc{0,0,width.getAs<int>(), height.getAs<int>()};
        AdjustWindowRectEx(&rc,WS_OVERLAPPEDWINDOW,FALSE, WS_EX_CLIENTEDGE);
        hWnd_ = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            windowClassName,
            TEXT("Cuppa"),
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, rc.right-rc.left, rc.bottom-rc.top,
            NULL, NULL, hInstance, NULL);
        if (hWnd_ == NULL)
        {
            MessageBox(NULL, TEXT("Window Creation Failed!"), TEXT("Error!"), MB_ICONEXCLAMATION | MB_OK);
            return;
        }
        SetLastError(0);
        if (SetWindowLongPtr(hWnd_, GWLP_USERDATA, (LONG_PTR)&_app)==0 && GetLastError()!=0)
        {
            MessageBox(NULL, TEXT("SetWindowLongPtr 1 Failed!"), TEXT("Error!"), MB_ICONEXCLAMATION | MB_OK);
            return;
        }
        auto p = GetAppPtr(hWnd_);
        Assert(p==&_app);
        ShowWindow(hWnd_, SW_NORMAL);
        UpdateWindow(hWnd_);
    }

    HWND hWnd_ { 0 };
    Pixel width = 240_px, height = 120_px;
};

namespace cuppa
{
    extern std::unique_ptr<graphicsDriverInterface> createGraphicsDriverGdiplus();

    void app::run()
    {
        // Commandline: __argc, __argv
        SystemDriver = new MSWindowsDriver(); // TODO: it will leak!!

        graphicsDriver = createGraphicsDriverGdiplus();
        graphicsDriver->setup();
        setup();
        std::any_cast<MSWindowsDriver*>(SystemDriver)->Setup(*this);
        MSG Msg;
        while(GetMessage(&Msg, NULL, 0, 0) > 0)
        {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
            update();
        }
    }

    void app::size(Pixel _width, Pixel _height)   {   std::any_cast<MSWindowsDriver*>(SystemDriver)->SetWindowSize(_width, _height);    }
    Pixel app::getWidth() const { return std::any_cast<MSWindowsDriver*>(SystemDriver)->GetWindowWidth(); }
    Pixel app::getHeight() const { return std::any_cast<MSWindowsDriver*>(SystemDriver)->GetWindowHeight(); }

    std::default_random_engine engine{ std::random_device()() };
    std::uniform_real_distribution<float> uniform_distribution{ 0.0f, 1.0f };
    std::normal_distribution<float> normal_distribution{ };
    float app::random() { return uniform_distribution(engine); }

    float app::randomGaussian() { return normal_distribution(engine); }
} // namespace cuppa