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
            background(CornflowerBlue);
            fill(White);
            text("HELLO WORLD!", {});
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
        }

        Point pos = { 400_px, 300_px };
    };
} // anonymous namespace

std::unique_ptr<cuppa::app> createProto()
{
    return std::make_unique<proto>();
}