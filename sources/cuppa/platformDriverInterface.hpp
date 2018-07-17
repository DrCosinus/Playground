#pragma once

#include "unit.hpp"

namespace cuppa
{
    class app;

    struct gamepadInterface
    {
        virtual bool connected() const = 0;
        virtual Direction leftStick() const = 0;
        virtual Direction rightStick() const = 0;
        virtual bool buttonA() const = 0;
        virtual bool buttonB() const = 0;
        virtual bool buttonX() const = 0;
        virtual bool buttonY() const = 0;
        // Direction LeftPad;
        // Direction RightPad;
        // bool connected : 1;
        // bool buttonA : 1;
        // bool buttonB : 1;
        // bool buttonX : 1;
        // bool buttonY : 1;
        // bool buttonStart : 1;
        // bool buttonBack : 1;
        // bool buttonLeftShoulderButton : 1;
        // bool buttonRightShoulderButton : 1;
        // //bool buttonLeftTriggerButton : 1;
        // //bool buttonRightTriggerButton : 1;
        // bool buttonLeftThumbButton : 1;
        // bool buttonRightThumbButton : 1;
        // bool buttonDigitalUp : 1;
        // bool buttonDigitalLeft : 1;
        // bool buttonDigitalDown : 1;
        // bool buttonDigitalRight : 1;
    };

    struct platformDriverInterface
    {
        virtual ~platformDriverInterface() = default;

        virtual void setup(app& _app) = 0;
        virtual void run() = 0;

        virtual void size(Pixel _width, Pixel _height) = 0;
        virtual const gamepadInterface& gamepad(std::size_t padIndex) = 0;
    };
} // namespace cuppa