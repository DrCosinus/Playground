#pragma once

#include "unit.hpp"
#include "image.hpp" // for DeviceContext... what the shame :(

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
        virtual bool buttonDigitalUp() const = 0;
        virtual bool buttonDigitalLeft() const = 0;
        virtual bool buttonDigitalDown() const = 0;
        virtual bool buttonDigitalRight() const = 0;
        virtual bool buttonStart() const = 0;
        virtual bool buttonBack() const = 0;
        virtual bool buttonLeftShoulder() const = 0;
        virtual bool buttonRightShoulder() const = 0;
        virtual bool buttonLeftThumb() const = 0;
        virtual bool buttonRightThumb() const = 0;
        virtual float leftTrigger() const = 0;
        virtual float rightTrigger() const = 0;

        // //bool buttonLeftTriggerButton : 1;
        // //bool buttonRightTriggerButton : 1;
    };

    struct platformDriverInterface
    {
        virtual ~platformDriverInterface() = default;

        virtual void setup(app& _app) = 0;
        virtual void run() = 0;
        virtual void quit() = 0;

        virtual void size(Pixel _width, Pixel _height) = 0;
        virtual const gamepadInterface& gamepad(std::size_t padIndex) = 0;
        virtual DeviceContext getDeviceContext() const = 0;
    };
} // namespace cuppa