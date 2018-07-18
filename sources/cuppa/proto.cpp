#include "proto.hpp"

namespace
{
    struct proto : cuppa::app
    {
        void setup() override
        {
            size( 800_px, 600_px );
        }
        void draw() override
        {
            background(Color{100,149,237});
            fill(White);
            text("HELLO WORLD!", {});
            rect(pos, { 10_px, 10_px });

            auto& pad = gamepad(0);
            if (pad.connected())
            {
                pos += pad.leftStick().flipY() * 10;
            }
            if (pad.buttonBack())
            {
                quit();
            }
        }

        Point pos = { 400_px, 300_px };
    };
} // anonymous namespace

std::unique_ptr<cuppa::app> createProto()
{
    return std::make_unique<proto>();
}