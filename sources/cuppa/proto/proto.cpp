#include "proto.hpp"

#include "console.hpp"
#include "os.hpp"

#include <cuppa/cuppa.hpp>

#include <string>
#include <iostream>
#include <queue>

using namespace cuppa;

namespace proto
{
    struct proto : app
    {
        Console console;
        OS fos;

        void setup() override
        {
            size( 800_px, 600_px );
            console.setup(fos);
            fos.setup(console);
        }

        void draw() override
        {
            stroke(Appliance::DISABLED);
            scale( 780 / 640.f, 580 / 480.f );
            console.ref( 72, 0) = gamepad(0).connected() ? '0' : '-';
            console.ref( 73, 0) = gamepad(1).connected() ? '1' : '-';
            console.ref( 74, 0) = gamepad(2).connected() ? '2' : '-';
            console.ref( 75, 0) = gamepad(3).connected() ? '3' : '-';
            translate({ 10_px, 10_px });
            background(Black);
            console.draw();

            rect(pos, { 10_px, 10_px });

            auto& pad = gamepad(0);
            if (pad.connected())
            {
                pos += pad.leftStick().flipY() * 10;
                if (pos.x<-5_px) pos.x += 810_px;
                if (pos.x>805_px) pos.x -= 810_px;
                if (pos.y<-5_px) pos.y += 610_px;
                if (pos.y>605_px) pos.y -= 610_px;
            }
            if (pad.buttonBack())
            {
                quit();
            }
            {
                auto x = 0_px;
                auto y = noise(x / getWidth()) * getHeight();
                stroke(Red, 3_px);
                for (auto x2 = 1_px; x2 < getWidth(); ++x2)
                {
                    auto y2 = noise(x2 / getWidth()) * getHeight();
                    line({x, y}, Point{x2, y2}); // to do: Point should accept (int, float) arguments std::common_type
                    x = x2;
                    y = y2;
                }
            }
        }

        void keyDown(short keyCode) override
        {
            console.pushKeyCode(keyCode);
        }

        void keyChar(char c) override
        {
            console.pushChar(c);
        }

        Point pos = { 400_px, 300_px };
    };
} // anonymous namespace

std::unique_ptr<app> createProto()
{
    return std::make_unique<proto::proto>();
}