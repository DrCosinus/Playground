#pragma once

#include "unit.hpp"

namespace cuppa
{
    class app;

    struct gamepad
    {
        bool connected;
        Direction LeftPad;
        Direction RightPad;
    };

    struct platformDriverInterface
    {
        virtual ~platformDriverInterface() = default;

        virtual void setup(app& _app) = 0;
        virtual void run() = 0;

        virtual void size(Pixel _width, Pixel _height) = 0;
        virtual const gamepad& getGamepad(std::size_t padIndex) = 0;
    };
} // namespace cuppa