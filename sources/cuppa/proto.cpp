#include "proto.hpp"

namespace
{
    struct proto : cuppa::app
    {
        Font font;
        void setup() override
        {
            size( 640_px, 480_px );
            font = loadFont("Consolas", 14);
            textFont(font);
        }
        void draw() override
        {
            background(Black);
            fill(Green);
            text("HELLO WORLD!", { 0_px, -3_px});
            text("IWMO....II00", { 0_px, 12_px});
            for(int row = 2; row < 32; ++row)
            {
                text("012345678901234567890123456789012345678901234567890123456789", { 0_px, 15_px*row-3_px});
            }
            // noFill();
            // stroke(0.5_px, White);
            // rect({320_px, 240_px}, {640_px, 480_px});

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