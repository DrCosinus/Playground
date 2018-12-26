#include "win_inputs.hpp"

#include <game.hpp>

#define NOMINMAX
#include <Windows.h>
#include <Xinput.h>

#include <algorithm>

static real32 mapStick(int16 _value, int16 _deadzone)
{
    return (_value > _deadzone) ? (_value - _deadzone) / (32767.0f - _deadzone)
                                : (_value < -_deadzone ? ((_value + _deadzone) / (32768.0f - _deadzone)) : 0);
}

static Game::DigitalButtonState ProcessDigitalBtn(DWORD                           XInputButtonState,
                                                  const Game::DigitalButtonState& PrevState,
                                                  DWORD                           ButtonBit)
{
    Game::DigitalButtonState NewState;
    NewState.EndedDown           = ((XInputButtonState & ButtonBit) == ButtonBit);
    NewState.HalfTransitionCount = (PrevState.EndedDown != NewState.EndedDown) ? 1 : 0;
    return NewState;
}

static void ProcessKeyboardMessage(Game::DigitalButtonState& NewState, bool32 IsDown)
{
    NewState.EndedDown = IsDown;
    ++NewState.HalfTransitionCount;
}

namespace Windows
{
    struct Inputs::Driver
    {
        Driver()
        {
            HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
            if (!XInputLibrary)
            {
                XInputLibrary = LoadLibraryA("xinput9_1_0.dll");

                if (!XInputLibrary)
                {
                    XInputLibrary = LoadLibraryA("xinput1_3.dll");
                }
            }

            if (XInputLibrary)
            {
                if (auto XInputGetState = (GetStateCB)GetProcAddress(XInputLibrary, "XInputGetState"))
                {
                    GetState = XInputGetState;
                }

                if (auto XInputSetState = (SetStateCB)GetProcAddress(XInputLibrary, "XInputSetState"))
                {
                    SetState = XInputSetState;
                }
            }
        }
        using GetStateCB = DWORD (*)(DWORD, XINPUT_STATE*);
        using SetStateCB = DWORD (*)(DWORD, XINPUT_VIBRATION*);

        GetStateCB GetState = GetStateStub;
        SetStateCB SetState = SetStateStub;

    private:
        static DWORD GetStateStub(DWORD, XINPUT_STATE*) { return ERROR_DEVICE_NOT_CONNECTED; }
        static DWORD SetStateStub(DWORD, XINPUT_VIBRATION*) { return ERROR_DEVICE_NOT_CONNECTED; }
    };

    Inputs::Inputs()  = default;
    Inputs::~Inputs() = default;

    void Inputs::Init() { driver = std::make_unique<Driver>(); }

    void Inputs::ProcessPendingMessages(Game::GamePad& KeyboardController)
    {
        MSG Message;
        while (!QuitRequested && PeekMessageA(&Message, 0, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
        {
            switch (Message.message)
            {
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                auto VKCode        = static_cast<uint32>(Message.wParam);
                auto ScanCode      = (static_cast<uint32>(Message.lParam) >> 16) & 0xFF;
                auto WasDown       = (Message.lParam & (1 << 30)) != 0;
                auto IsDown        = (Message.lParam & (1 << 31)) == 0;
                auto AltKeyWasDown = (Message.lParam & (1 << 29)) != 0;
                if (WasDown != IsDown)
                {
                    char BufferT[24];
                    sprintf_s(BufferT, "ScanCode: %d\n", ScanCode);
                    OutputDebugStringA(BufferT);

                    switch (VKCode)
                    {
                    case 'Z': // W ScanCode 17
                        ProcessKeyboardMessage(KeyboardController.MoveUp, IsDown);
                        break;
                    case 'Q': // A ScanCode 30
                        ProcessKeyboardMessage(KeyboardController.MoveLeft, IsDown);
                        break;
                    case 'S': // ScanCode 31
                        ProcessKeyboardMessage(KeyboardController.MoveDown, IsDown);
                        break;
                    case 'D': // ScanCode 32
                        ProcessKeyboardMessage(KeyboardController.MoveRight, IsDown);
                        break;
                    case 'A': // Q ScanCode 16
                        ProcessKeyboardMessage(KeyboardController.LeftShoulder, IsDown);
                        break;
                    case 'E': // ScanCode 18
                        ProcessKeyboardMessage(KeyboardController.RightShoulder, IsDown);
                        break;
                    case VK_UP: // ScanCode 72
                        ProcessKeyboardMessage(KeyboardController.ActionUp, IsDown);
                        break;
                    case VK_LEFT: // ScanCode 75
                        ProcessKeyboardMessage(KeyboardController.ActionLeft, IsDown);
                        break;
                    case VK_DOWN: // ScanCode 80
                        ProcessKeyboardMessage(KeyboardController.ActionDown, IsDown);
                        break;
                    case VK_RIGHT: // ScanCode 77
                        ProcessKeyboardMessage(KeyboardController.ActionRight, IsDown);
                        break;
                    case VK_ESCAPE: // ScanCode 1
                        ProcessKeyboardMessage(KeyboardController.Start, IsDown);
                        break;
                    case VK_BACK: // ScanCode 14
                        ProcessKeyboardMessage(KeyboardController.Back, IsDown);
                        break;
                    }
                }

                if ((VKCode == VK_F4) && AltKeyWasDown)
                {
                    QuitRequested = true;
                    break;
                }
            }
            break;

            default:
                // TranslateMessage(&Message); // Useless?
                DispatchMessageA(&Message);
                break;
            }
        }
    }

    void Inputs::Update()
    {
        auto& prevInput = PIInputs[CurrentInput];
        CurrentInput    = 1 - CurrentInput;
        auto& curInput  = PIInputs[CurrentInput];

        auto& curKeyboardCtrl  = curInput.Keyboard;
        auto& prevKeyboardCtrl = prevInput.Keyboard;
        curKeyboardCtrl        = {};

        for (size_t ButtonIndex = 0; ButtonIndex < ArrayCount(curKeyboardCtrl.Buttons); ++ButtonIndex)
        {
            curKeyboardCtrl.Buttons[ButtonIndex].EndedDown = prevKeyboardCtrl.Buttons[ButtonIndex].EndedDown;
        }

        ProcessPendingMessages(curKeyboardCtrl);

        constexpr DWORD MaxControllerCount =
            std::min(static_cast<const uint32>(XUSER_MAX_COUNT), curInput.GamePadCount);

        for (DWORD ControllerIndex = 0; ControllerIndex < MaxControllerCount; ++ControllerIndex)
        {
            auto& prevCtrl = prevInput.GamePads[ControllerIndex];
            auto& curCtrl  = curInput.GamePads[ControllerIndex];

            XINPUT_STATE ControllerState;
            if (driver->GetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
            {
                curCtrl.IsConnected = true;

                auto& Pad = ControllerState.Gamepad;

                curCtrl.LeftStickX = mapStick(Pad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                curCtrl.LeftStickY = mapStick(Pad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

                if ((curCtrl.LeftStickX != 0.0f) || (curCtrl.LeftStickY != 0.0f))
                {
                    curCtrl.IsAnalog = true;
                }

                if (Pad.wButtons & XINPUT_GAMEPAD_DPAD_UP)
                {
                    curCtrl.LeftStickY = 1.0f;
                    curCtrl.IsAnalog   = false;
                }
                if (Pad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
                {
                    curCtrl.LeftStickY = -1.0f;
                    curCtrl.IsAnalog   = false;
                }
                if (Pad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
                {
                    curCtrl.LeftStickX = -1.0f;
                    curCtrl.IsAnalog   = false;
                }
                if (Pad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
                {
                    curCtrl.LeftStickX = 1.0f;
                    curCtrl.IsAnalog   = false;
                }

                // TODO: Hysteresis
                constexpr real32 Threshold = 0.5f;
                curCtrl.MoveLeft  = ProcessDigitalBtn(curCtrl.LeftStickX < -Threshold ? 1 : 0, prevCtrl.MoveLeft, 1);
                curCtrl.MoveRight = ProcessDigitalBtn(curCtrl.LeftStickX > Threshold ? 1 : 0, prevCtrl.MoveRight, 1);
                curCtrl.MoveDown  = ProcessDigitalBtn(curCtrl.LeftStickY < -Threshold ? 1 : 0, prevCtrl.MoveDown, 1);
                curCtrl.MoveUp    = ProcessDigitalBtn(curCtrl.LeftStickY > Threshold ? 1 : 0, prevCtrl.MoveUp, 1);

                curCtrl.ActionDown  = ProcessDigitalBtn(Pad.wButtons, prevCtrl.ActionDown, XINPUT_GAMEPAD_A);
                curCtrl.ActionRight = ProcessDigitalBtn(Pad.wButtons, prevCtrl.ActionRight, XINPUT_GAMEPAD_B);
                curCtrl.ActionLeft  = ProcessDigitalBtn(Pad.wButtons, prevCtrl.ActionLeft, XINPUT_GAMEPAD_X);
                curCtrl.ActionUp    = ProcessDigitalBtn(Pad.wButtons, prevCtrl.ActionUp, XINPUT_GAMEPAD_Y);
                curCtrl.LeftShoulder =
                    ProcessDigitalBtn(Pad.wButtons, prevCtrl.LeftShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER);
                curCtrl.RightShoulder =
                    ProcessDigitalBtn(Pad.wButtons, prevCtrl.RightShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER);

                curCtrl.Start = ProcessDigitalBtn(Pad.wButtons, prevCtrl.Start, XINPUT_GAMEPAD_START);
                curCtrl.Back  = ProcessDigitalBtn(Pad.wButtons, prevCtrl.Back, XINPUT_GAMEPAD_BACK);
            }
            else
            {
                curCtrl.IsConnected = false;
            }
        }
    }
} // namespace Windows