#pragma once

#include "unit.hpp"

namespace cuppa
{
    class app;

    struct platformDriverInterface
    {
        virtual ~platformDriverInterface() = default;

        virtual void setup(app& _app) = 0;
        virtual void run() = 0;

        virtual void size(Pixel _width, Pixel _height) = 0;
        virtual Pixel getWidth() const = 0;
        virtual Pixel getHeight() const = 0;
    };
} // namespace cuppa