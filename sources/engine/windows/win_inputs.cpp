#include "win_inputs.hpp"

#include <game.hpp>

#define NOMINMAX
#include <Windows.h>
#include <Xinput.h>

#include <algorithm>

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
        auto& OldInput = Input[CurrentInput];
        CurrentInput   = 1 - CurrentInput;
        auto& NewInput = Input[CurrentInput];

        auto& NewKeyboardController = NewInput.Keyboard;
        auto& OldKeyboardController = OldInput.Keyboard;
        NewKeyboardController       = {};

        for (size_t ButtonIndex = 0; ButtonIndex < ArrayCount(NewKeyboardController.Buttons); ++ButtonIndex)
        {
            NewKeyboardController.Buttons[ButtonIndex].EndedDown = OldKeyboardController.Buttons[ButtonIndex].EndedDown;
        }

        ProcessPendingMessages(NewKeyboardController);

        constexpr DWORD MaxControllerCount =
            std::min(static_cast<const uint32>(XUSER_MAX_COUNT), NewInput.GamePadCount);

        for (DWORD ControllerIndex = 0; ControllerIndex < MaxControllerCount; ++ControllerIndex)
        {
            auto& OldCtrl = OldInput.GamePads[ControllerIndex];
            auto& NewCtrl = NewInput.GamePads[ControllerIndex];

            XINPUT_STATE ControllerState;
            if (driver->GetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
            {
                NewCtrl.IsConnected = true;

                auto& Pad = ControllerState.Gamepad;

                NewCtrl.LeftStickX = mapStick(Pad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                NewCtrl.LeftStickY = mapStick(Pad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

                if ((NewCtrl.LeftStickX != 0.0f) || (NewCtrl.LeftStickY != 0.0f))
                {
                    NewCtrl.IsAnalog = true;
                }

                if (Pad.wButtons & XINPUT_GAMEPAD_DPAD_UP)
                {
                    NewCtrl.LeftStickY = 1.0f;
                    NewCtrl.IsAnalog   = false;
                }
                if (Pad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
                {
                    NewCtrl.LeftStickY = -1.0f;
                    NewCtrl.IsAnalog   = false;
                }
                if (Pad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
                {
                    NewCtrl.LeftStickX = -1.0f;
                    NewCtrl.IsAnalog   = false;
                }
                if (Pad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
                {
                    NewCtrl.LeftStickX = 1.0f;
                    NewCtrl.IsAnalog   = false;
                }

                // TODO: Hysteresis
                constexpr real32 Threshold = 0.5f;
                NewCtrl.MoveLeft  = ProcessDigitalBtn(NewCtrl.LeftStickX < -Threshold ? 1 : 0, OldCtrl.MoveLeft, 1);
                NewCtrl.MoveRight = ProcessDigitalBtn(NewCtrl.LeftStickX > Threshold ? 1 : 0, OldCtrl.MoveRight, 1);
                NewCtrl.MoveDown  = ProcessDigitalBtn(NewCtrl.LeftStickY < -Threshold ? 1 : 0, OldCtrl.MoveDown, 1);
                NewCtrl.MoveUp    = ProcessDigitalBtn(NewCtrl.LeftStickY > Threshold ? 1 : 0, OldCtrl.MoveUp, 1);

                NewCtrl.ActionDown  = ProcessDigitalBtn(Pad.wButtons, OldCtrl.ActionDown, XINPUT_GAMEPAD_A);
                NewCtrl.ActionRight = ProcessDigitalBtn(Pad.wButtons, OldCtrl.ActionRight, XINPUT_GAMEPAD_B);
                NewCtrl.ActionLeft  = ProcessDigitalBtn(Pad.wButtons, OldCtrl.ActionLeft, XINPUT_GAMEPAD_X);
                NewCtrl.ActionUp    = ProcessDigitalBtn(Pad.wButtons, OldCtrl.ActionUp, XINPUT_GAMEPAD_Y);
                NewCtrl.LeftShoulder =
                    ProcessDigitalBtn(Pad.wButtons, OldCtrl.LeftShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER);
                NewCtrl.RightShoulder =
                    ProcessDigitalBtn(Pad.wButtons, OldCtrl.RightShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER);

                NewCtrl.Start = ProcessDigitalBtn(Pad.wButtons, OldCtrl.Start, XINPUT_GAMEPAD_START);
                NewCtrl.Back  = ProcessDigitalBtn(Pad.wButtons, OldCtrl.Back, XINPUT_GAMEPAD_BACK);
            }
            else
            {
                NewCtrl.IsConnected = false;
            }
        }
    }
} // namespace Windows