#include "proto.hpp"

#include <string>
#include <iostream>

using namespace cuppa;

namespace
{
    namespace fantasy
    {
        struct Console
        {
            static constexpr std::size_t cols = 80;
            static constexpr std::size_t rows = 30;
            std::size_t caret_row = 0;
            char buffer[cols*rows];
            Font font;

            std::size_t count = 0;
            bool interactive = false;
            std::string_view script;

            Console()
            {
            }
            void setup()
            {
                font = loadFont("Consolas", 14);
                auto p = textSize("1");
                std::cout << p.width.getAs<float>() << "x" << p.height.getAs<float>() << std::endl;
                p = textSize("12345678901234567890123456789012345678901234567890123456789012345678901234567890\n0");
                std::cout << p.width.getAs<float>() << "x" << p.height.getAs<float>() << std::endl;
                writeline("DrCosinus -- PotAuJeu - v12.23b, ");
                writeline("Copyright (C) 1972-2018, DrCosinus Systems Corporation, INC.");
                writeline(" ");
                writeline("DRCSNS PRTBLPRO2.0 Deluxe ACPI BIOS Rev 1008");
                writeline(" ");
                writeline("Main Processor : DRC Powertron(tm) 23Mhz");
            }
            void draw()
            {
                textFont(font);
                fill(Lime);
                for(size_t row = 0; row < rows; ++row)
                {
                    text({buffer+row*cols, cols}, { 0_px, 16_px * row});
                }
                count = (count+1)%60;
                if (count < 30)
                {
                    rect({ 5_px, 8_px+16_px * caret_row}, { 10_px, 14_px });
                }
            }
            void writeline(std::string_view line)
            {
                auto subline = line.substr(0, cols);
                std::copy(std::begin(subline), std::end(subline), buffer+caret_row*cols);
                caret_row++;
            }
        };

        struct FantasyOs
        {
            void boot()
            {

            }
            void setup()
            {
                boot();
            }
            void draw()
            {

            }
        };
    } // namespace fantasy

    struct proto : app
    {
        fantasy::Console console;

        void setup() override
        {
            size( 800_px, 600_px );
            console.setup();
        }

        void draw() override
        {
            scale( 780 / 640.f, 580 / 480.f );
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
        }

        Point pos = { 400_px, 300_px };
    };
} // anonymous namespace

std::unique_ptr<app> createProto()
{
    return std::make_unique<proto>();
}