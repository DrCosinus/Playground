#pragma once

#include "types.hpp"

namespace Game
{
    // Inputs

    struct DigitalButtonState
    {
        uint32 HalfTransitionCount;
        bool32 EndedDown;
    };

    struct GamePad
    {
        bool32 IsConnected;
        bool32 IsAnalog;

        real32 LeftStickX, LeftStickY;
        real32 RightStickX, RightStickY;
        real32 LeftTrigger, RightTrigger;

        union
        {
            DigitalButtonState Buttons[16];
#pragma warning(push)
#pragma warning(disable : 4201)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#pragma clang diagnostic ignored "-Wnested-anon-types"
            struct
            {
                DigitalButtonState MoveUp;
                DigitalButtonState MoveDown;
                DigitalButtonState MoveLeft;
                DigitalButtonState MoveRight;
                DigitalButtonState ActionUp;
                DigitalButtonState ActionDown;
                DigitalButtonState ActionLeft;
                DigitalButtonState ActionRight;
                DigitalButtonState LeftShoulder;
                DigitalButtonState RightShoulder;
                DigitalButtonState LeftThumb;
                DigitalButtonState RightThumb;
                DigitalButtonState LeftTriggerButton;
                DigitalButtonState RightTriggerButton;
                DigitalButtonState Back;
                DigitalButtonState Start;
            };
#pragma warning(pop)
#pragma clang diagnostic pop
        };
    };

    struct Inputs
    {
        static constexpr uint32 GamePadCount = 4;
        GamePad                 GamePads[GamePadCount];
        GamePad                 Keyboard;
    };
}