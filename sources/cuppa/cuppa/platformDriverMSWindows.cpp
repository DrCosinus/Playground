#include "platformDriverInterface.hpp"

#include "cuppa.hpp" // for cuppa::app

#include <windows.h>

#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

#include <Xinput.h>
#include <iostream>
#include <thread>

namespace cuppa
{
    struct xgamepad : gamepadInterface
    {
        XINPUT_STATE state;
        static constexpr DWORD invalidIndex = 0xFFFFFFFF;
        DWORD userIndex = invalidIndex;
        bool isConnected = false;

        void update()
        {
            Assert(userIndex!=invalidIndex);

            //ZeroMemory( &state, sizeof(XINPUT_STATE));
            isConnected = (XInputGetState( userIndex, &state ) == ERROR_SUCCESS);
        }

        bool connected() const override
        {
            return isConnected;
        }

        bool buttonA() const override   {   return isConnected && (state.Gamepad.wButtons & XINPUT_GAMEPAD_A);  }
        bool buttonB() const override   {   return isConnected && (state.Gamepad.wButtons & XINPUT_GAMEPAD_B);  }
        bool buttonX() const override   {   return isConnected && (state.Gamepad.wButtons & XINPUT_GAMEPAD_X);  }
        bool buttonY() const override   {   return isConnected && (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y);  }
        bool buttonDigitalUp() const override   {   return isConnected && (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP);  }
        bool buttonDigitalLeft() const override   {   return isConnected && (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT);  }
        bool buttonDigitalDown() const override   {   return isConnected && (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN);  }
        bool buttonDigitalRight() const override   {   return isConnected && (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);  }
        bool buttonStart() const override   {   return isConnected && (state.Gamepad.wButtons & XINPUT_GAMEPAD_START);  }
        bool buttonBack() const override   {   return isConnected && (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK);  }
        bool buttonLeftShoulder() const override   {   return isConnected && (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);  }
        bool buttonRightShoulder() const override   {   return isConnected && (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);  }
        bool buttonLeftThumb() const override   {   return isConnected && (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB);  }
        bool buttonRightThumb() const override   {   return isConnected && (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);  }

        float leftTrigger() const override { return isConnected ? state.Gamepad.bLeftTrigger / 255.f : 0.f; }
        float rightTrigger() const override { return isConnected ? state.Gamepad.bRightTrigger / 255.f : 0.f; }

        static float remapStick(SHORT stick, SHORT threshold)
        {
            auto absVal = static_cast<float>(abs(stick) - threshold);
            absVal = absVal > 0 ? ( absVal / (32767 - threshold) ) : 0;
            return stick >= 0 ? absVal : -absVal;
        }

        Direction leftStick() const override
        {
            //Assert(isConnected);
            return { Pixel{ remapStick(state.Gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) },
                     Pixel{ remapStick(state.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) } };
        }

        Direction rightStick() const override
        {
            //Assert(isConnected);
            return { Pixel{ remapStick(state.Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) },
                     Pixel{ remapStick(state.Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) } };
        }
    };

    xgamepad gamepads[XUSER_MAX_COUNT];

    static constexpr TCHAR windowClassName[] = TEXT("cuppaWindowClass");

    void assertionError(const char* _message)
    {
        MessageBoxA(NULL, _message, "Assertion error!", MB_ICONEXCLAMATION | MB_OK);
    }

    struct platformDriverMSWindows : platformDriverInterface
    {
        //platformDriverMSWindows() = default;
        ~platformDriverMSWindows() override = default;
        //explicit platformDriverMSWindows(HWND _hWnd) : hWnd_{_hWnd} {}

        static auto getAppPtr(HWND hWnd) { return reinterpret_cast<app*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)); }

        void setup(app& _app) override
        {
            auto hInstance = GetModuleHandle(NULL);
            registerWindowClass(hInstance);
            createAppWindow(hInstance, _app);
            {
                DWORD i = 0;
                for (auto& gamepad: gamepads)
                {
                    gamepad.userIndex = i++;
                }
            }
        }

        DeviceContext getDeviceContext() const override
        {
            return DeviceContext{ GetDC(hWnd_) };
        }

        void quit() override
        {
            DestroyWindow(hWnd_);
        }

        void run() override
        {
            auto appPtr = getAppPtr(hWnd_);
            MSG Msg;
            while(GetMessage(&Msg, NULL, 0, 0) > 0)
            {
                appPtr->onBeginFrame();
                appPtr->update();
                appPtr->onEndFrame();
                TranslateMessage(&Msg);
                DispatchMessage(&Msg);
            }
        }

        void size(Pixel /*_width*/, Pixel /*_height*/) override
        {
            // for now, we only handle initial width and height
            // TODO: handle SetWindowSize after the creation of the windows
        }

        const gamepadInterface& gamepad(std::size_t padIndex) override
        {
            return gamepads[padIndex];
        }

        void beep(int frequency, int duration) override
        {
            std::thread([frequency, duration]{ Beep( frequency, duration ); }).detach();
        }

        void systemBeep(int type) override
        {
            MessageBeep(type);
        }
private:
        void draw()
        {
            auto appPtr = getAppPtr(hWnd_);
            POINT p;
            if (GetCursorPos(&p))
            {
                if (ScreenToClient(hWnd_, &p))
                {
                    appPtr->setMousePosition({Pixel{p.x}, Pixel{p.y}});
                }
            }
            PAINTSTRUCT ps;
            auto hdc = BeginPaint(hWnd_, &ps);
            graphicsDriver->draw(DeviceContext{hdc});
            EndPaint(hWnd_, &ps);
        }

        void refreshWindow()
        {
            InvalidateRect(hWnd_, nullptr, FALSE);
            UpdateWindow(hWnd_);
            for(auto& gamepad: gamepads)
            {
                gamepad.update();
            }
        }

        static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
        {
            //auto platformDriver = platformDriverMSWindows{ hwnd };
            auto platformDriverMS = static_cast<platformDriverMSWindows*>(platformDriver.get());
            switch(msg)
            {
                case WM_CREATE:     platformDriverMS->onCreate(hwnd);                                         break;
                case WM_TIMER:      platformDriverMS->onTimer(wParam);                                        break;
                case WM_PAINT:      platformDriverMS->onPaint();                                              break;
                case WM_CLOSE:      platformDriverMS->onClose();                                              break;
                case WM_DESTROY:    platformDriverMS->onDestroy();                                            break;
                case WM_SIZE:       platformDriverMS->onSize({Pixel(LOWORD(lParam)), Pixel(HIWORD(lParam))}); break;
                case WM_LBUTTONDOWN:platformDriverMS->onLeftMouseButtonDown({Pixel((lParam)), Pixel(GET_Y_LPARAM(lParam))});    break;
                case WM_LBUTTONUP:  platformDriverMS->onLeftMouseButtonUp({Pixel((lParam)), Pixel(GET_Y_LPARAM(lParam))});      break;
                case WM_KEYDOWN:    platformDriverMS->onKeyDown(static_cast<short>(wParam));    break;
                case WM_KEYUP:      platformDriverMS->onKeyUp(static_cast<short>(wParam));      break;
                case WM_CHAR:       platformDriverMS->onChar(static_cast<char>(wParam));        break;
                //case WM_MOUSEMOVE:  platformDriverMS->onMouseMove({Pixel(LOWORD(lParam)), Pixel(HIWORD(lParam))}); break;
                default:            return DefWindowProc(hwnd, msg, wParam, lParam);
            }
            return 0;
        }

        void onCreate(HWND _hWnd)
        {
            SetTimer(_hWnd, 1234, 16, (TIMERPROC)0);
        }

        void onClose()
        {
            DestroyWindow(hWnd_);
        }

        void onDestroy()
        {
            PostQuitMessage(0);
        }

        void onTimer(UINT_PTR timerID)
        {
            // TODO: check the TIMER_ID
            if (timerID==1234)
            {
                refreshWindow();
            }
        }

        void onPaint()
        {
            draw();
        }

        void onSize(Direction newSize)
        {
            auto appPtr = getAppPtr(hWnd_);
            appPtr->setSize(newSize);
        }

        //Point leftMouseButtonDownPosition;
        void onLeftMouseButtonDown(Point /*position*/)
        {
            //leftMouseButtonDownPosition = position;
        }

        void onLeftMouseButtonUp(Point position)
        {
            //if (leftMouseButtonDownPosition == position)
            {
                getAppPtr(hWnd_)->mouseClick(position);
            }
        }

        // void onMouseMove(Point position)
        // {
        //     getAppPtr(hWnd_)->mouseMove(position);
        // }

        void onKeyDown(short virtualKeyCode)
        {
            getAppPtr(hWnd_)->keyDown(virtualKeyCode);
        }

        void onKeyUp(short virtualKeyCode)
        {
            getAppPtr(hWnd_)->keyUp(virtualKeyCode);

        }

        void onChar(char character)
        {
            getAppPtr(hWnd_)->keyChar(character);
        }

        void registerWindowClass(HINSTANCE hInstance)
        {
            WNDCLASSEX wc;
            wc.cbSize        = sizeof(WNDCLASSEX);
            wc.style         = CS_CLASSDC;
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
            RECT rc{0,0, _app.getWidth().getAs<int>(), _app.getHeight().getAs<int>()};
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
    };

    std::unique_ptr<platformDriverInterface> createPlatformDriverMSWindows() { return std::make_unique<platformDriverMSWindows>(); }
} // namespace cuppa