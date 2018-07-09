#include "platformDriverInterface.hpp"

#include "cuppa.hpp" // for cuppa::app

#include <windows.h>

namespace cuppa
{
    static constexpr TCHAR windowClassName[] = TEXT("cuppaWindowClass");

    void assertionError(const char* _message)
    {
        MessageBoxA(NULL, _message, "Assertion error!", MB_ICONEXCLAMATION | MB_OK);
    }

    struct platformDriverMSWindows : platformDriverInterface
    {
        static auto getAppPtr(HWND hWnd) { return reinterpret_cast<app*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)); }

        void setup(app& _app) override
        {
            auto hInstance = GetModuleHandle(NULL);
            registerWindowClass(hInstance);
            createAppWindow(hInstance, _app);
        }

        void run() override
        {
            MSG Msg;
            while(GetMessage(&Msg, NULL, 0, 0) > 0)
            {
                getAppPtr(hWnd_)->update();
                TranslateMessage(&Msg);
                DispatchMessage(&Msg);
            }
        }

        void size(Pixel _width, Pixel _height) override
        {
            // for now, we only handle initial width and height
            // TODO: handle SetWindowSize after the creation of the windows
            width = _width;
            height = _height;
        }
        Pixel getWidth() const override { return width; }
        Pixel getHeight() const override { return height; }

private:
        void draw()
        {
            PAINTSTRUCT ps;
            auto hdc = BeginPaint(hWnd_, &ps);
            auto appPtr = getAppPtr(hWnd_);
            auto gfxDvrItf = appPtr->graphicsDriver.get();
            gfxDvrItf->draw(*appPtr, DeviceContext{hdc});
            EndPaint(hWnd_, &ps);
        }

        void refreshWindow(HWND _hWnd)
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
                        auto appPtr = getAppPtr(hwnd);
                        if (appPtr)
                            static_cast<platformDriverMSWindows*>(appPtr->platformDriver.get())->refreshWindow(hwnd);
                    }
                    break;
                case WM_PAINT:
                    {
                        auto appPtr = getAppPtr(hwnd);
                        if (appPtr)
                            static_cast<platformDriverMSWindows*>(appPtr->platformDriver.get())->draw();
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

        void registerWindowClass(HINSTANCE hInstance)
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

        void createAppWindow(HINSTANCE hInstance, app& _app)
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
            auto p = getAppPtr(hWnd_);
            Assert(p==&_app);
            ShowWindow(hWnd_, SW_NORMAL);
            UpdateWindow(hWnd_);
        }

        HWND hWnd_ { 0 };
        Pixel width = 240_px, height = 120_px;
    };

    std::unique_ptr<platformDriverInterface> createPlatformDriverMSWindows() { return std::make_unique<platformDriverMSWindows>(); }
} // namespace cuppa