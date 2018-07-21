#include "proto.hpp"

#include <string>
#include <iostream>
#include <queue>

using namespace cuppa;

namespace
{
    namespace fantasy
    {
        struct Console
        {
            static constexpr std::size_t cols = 80;
            static constexpr std::size_t rows = 30;
            std::size_t caret_col = 0;
            std::size_t caret_row = 0;
            struct cell
            {
                char character = ' ';
                unsigned char color = 1;
            };
            cell buffer[cols*rows];
            Font font;
            std::size_t idleTime = 0;
            std::size_t characterTime = 0;
            std::size_t lineTime = 0;
            unsigned char caret_color = 1;

            std::size_t caretBlinkCounter = 0;
            bool interactive = false;
            std::queue<std::string_view> scripts;

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
            }
            void draw()
            {
                if (idleTime > 0)
                {
                    if (idleTime > 16)
                        idleTime -= 16;
                    else
                        idleTime = 0;
                }
                updateScripts();
                // auto memo = caret_row;
                // caret_row = 10;
                // writeLine("millisecond from start:");
                // writeLine(std::to_string(getMillisFromMark()));
                // caret_row = memo;
                textFont(font);
                fill(Lime);
                int index = 0;
                for(auto& c: buffer)
                {
                    int row = index / cols;
                    int col = index - row * cols;
                    Color color;
                    switch(c.color)
                    {
                        case 1: color = Lime; break;
                        case 2: color = Red; break;
                        case 3: color = Yellow; break;
                        case 4: color = White; break;
                        default: color = Blue; break;
                    }
                    fill(color);
                    text({ &c.character, 1 }, { 8_px * col, 16_px * row});
                    ++index;
                }
                fill(Lime);
                caretBlinkCounter = (caretBlinkCounter+1)%60;
                if (caretBlinkCounter < 30)
                {
                    rect({ 6_px+ 8_px * caret_col, 8_px+16_px * caret_row}, { 8_px, 16_px });
                }
            }

            void writeLine(std::string_view line)
            {
                auto subline = line.substr(0, cols);
                std::transform(std::begin(subline), std::end(subline), buffer+caret_row*cols, [caret_color=caret_color](auto c){ return cell{ c, caret_color }; } );
                caret_row++;
            }

            void writeScript(std::string_view line)
            {
                scripts.push(line);
            }

            void updateScripts()
            {
                if (idleTime == 0 && !scripts.empty())
                {
                    auto& line = scripts.front();
                    //auto subline = line.substr(0, cols);
                    //std::copy(std::begin(subline), std::end(subline), buffer+caret_row*cols);

                    for(int i = 0; !line.empty() && idleTime==0; ++i)
                    {
                        // commands are /Lxyz: where
                        // - L is a single character coding the command
                        // - xyz is an optional positive integral value
                        // - colon character
                        // consequently command are at least 3 characters long
                        if (line[0]=='/' && line.size()>2)
                        {
                            auto j = 2u;
                            std::size_t value = 0;
                            while (line.size()>j && line[j]>='0' && line[j]<='9')
                            {
                                value = 10 * value + (line[j++]-'0');
                            }
                            if (line.size()>j && line[j]==':')
                            {
                                switch(line[1])
                                {
                                    case 'P': // /Pxyz: pause immediately for xyz milliseconds
                                        idleTime = value;
                                        break;
                                    case 'D': // /Dxyz: setup delay time between each characters
                                        characterTime = value;
                                        break;
                                    case 'C': // /Cxyz: set the following characters color
                                        caret_color = static_cast<decltype(caret_color)>(value);
                                        break;
                                    case 'N': // /N: new line
                                        caret_col = 0;
                                        caret_row++;
                                        break;
                                    case 'S': // /S: scroll up
                                        std::copy( buffer + cols, buffer + rows * cols, buffer);
                                        std::fill( buffer + (rows-1)*cols, buffer + rows * cols, cell{ ' ', caret_color });
                                        idleTime = characterTime;
                                        break;
                                }
                                line = line.substr(j+1);
                                continue;
                            }
                        }
                        buffer[caret_col+caret_row*cols] = { line[0], caret_color };
                        caret_col++;
                        if (caret_col==cols)
                        {
                            caret_col = 0;
                            caret_row++;
                        }
                        line = line.substr(1);
                        idleTime = characterTime;
                        caretBlinkCounter = 0;
                    }
                    if (line.empty())
                        scripts.pop();
                }
            }
        };

        struct OS
        {
            Console* console;
            std::string_view header[6] =
            {
                "/P500:PotAuJeu early prototype- v12.23b,/N:/P300:"
                "Copyright (C) 1972-2018, DrCosinus Systems Corporation, INC./N:/P300:"
                "/N:"
                "DRCSNS PRTBLPRO2.0 Deluxe ACPI BIOS Rev 1008/N:/P300:"
                "/N:"
                "Main Processor:/P200: DRC Powertron(tm) /C4:23Mhz/C1:/N:/P500:"
                "/N:"
                "Connecting to store/D100:......../D1:............/D: /C3:OK/C1:/N:"
                "Downloading loot boxes/D100:............./D1:..../D: /C3:OK/C1:/N:"
                "Checking micro transactions/D100:..../D1:......../D: /C3:OK/C1:/N:"
                "Procedural world generation/D100:.........../D1:./D: /C3:OK/C1:/N:"
                "Systemic gameplay init/D100:............../D1:.../D: /C3:OK/C1:/N:"
                "Retrieving Blockchains list/D100:......./D1:...../D: /C3:OK/C1:/N:"
                "Sound/D100:....../D1:............................/D: /C2:FAILURE/C1:/N:"
                "3D HD Graphics/D100:..../D1:...................../D: /C2:FAILURE/C1:/N:"
                "Modern Gameplay/D100:.../D1:...................../D: /C2:FAILURE/C1:/N:"
                "/D80:/S:/S:/S:/S:/S:/S:/S:/S:/S:/S:/S:/S:/S:/S:/S:/S:"
            };
            void boot()
            {
                for(auto& sv: header)
                {
                    console->writeScript(sv);
                }

            }
            void setup(Console& _console)
            {
                console = &_console;
                boot();
            }
            void update()
            {

            }
        };
    } // namespace fantasy

    struct proto : app
    {
        fantasy::Console console;
        fantasy::OS fos;

        void setup() override
        {
            size( 800_px, 600_px );
            console.setup();
            fos.setup(console);
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