#include "os.hpp"

#include "console.hpp"
#include <cuppa/cuppa.hpp>

#include <iostream>

using namespace cuppa;

namespace proto
{
    namespace
    {
        [[maybe_unused]]
        std::string_view note =
            "Ce week-end, j'aurai participé à un Game Jam organisé par un You-Tuber fan de truc chelou, ukulélé et de fromage."
            "Il est bien gentil, mais quand le thème est tombé ça a été la douche froide. Franchement il est sérieux ?"
            "Faire du neuf avec du vieux.";
    }

    void OS::boot()
    {
        console->writeScript(
            "/P500:PotAuJeu early prototype- v12.23b,/N:/P300:"
            "Copyright (C) 1972-2018, DrCosinus Systems Corporation, INC./N:/P300:"
            "/N:"
#if 0
            "DRCSNS PRTBLPRO2.0 Deluxe ACPI BIOS Rev 1008/N:/P300:"
            "/N:"
            "Main Processor:/P200: DRC At0tron(tm) /C4:23Mhz/C1:/N:/P500:"
            "/N:"
            "Memory Frequency is at /C4:16MHz/C1:/N:/P500:"
            "  Primary Master Disk: /C4:DRCSNS DR1000-23/C1:/N:"
            "   Primary Slave Disk: /C4:Empty/C1:/N:"
            "Secondary Master Disk: /C4:DRCSNS DSK-100 Floppy Disk Drive/C1:/N:"
            " Secondart Slave Disk: /C4:Empty/C1:/N:"
            "/N:"
            "Connecting to store/D75:......../D1:............/D: /C3:OK/C1:/N:"
            "Downloading loot boxes/D75:............./D1:..../D: /C3:OK/C1:/N:"
            "Checking micro transactions/D75:..../D1:......../D: /C3:OK/C1:/N:"
            "Procedural world generation/D75:.........../D1:./D: /C3:OK/C1:/N:"
            "Systemic gameplay init/D75:............../D1:.../D: /C3:OK/C1:/N:"
            "Retrieving Blockchains list/D75:......./D1:...../D: /C3:OK/C1:/N:"
            "Sound/D75:....../D: /C2:FAILURE/C1:/N:"
            "3D HD Graphics/D75:..../D: /C2:FAILURE/C1:/N:"
            "Inovating Gameplay/D75:......./D: /C2:FAILURE/C1:/N:"
#endif
            "/E100:"
        );
    }

    int getInteger(std::string_view& view, int defaultValue)
    {
        if (!view.empty() && isDigit(view[0]))
        {
            int value = 0;
            std::size_t i = 0;
            while(i < view.size() && isDigit(view[i]))
            {
                value = value * 10 + (view[i++] - '0');
            }
            if (i==view.size() || view[i]==' ')
            {
                view.remove_prefix(i);
                if (i<view.size())
                {
                    view.remove_prefix(view.find_first_not_of(' '));
                }
                return value;
            }
        }
        return defaultValue;
    }

    void OS::interpret(std::string_view commandline)
    {
        auto pos = commandline.find_first_of(' ');
        std::string_view command;
        if (pos == std::string_view::npos)
        {
            command = commandline;
            commandline = std::string_view{};
        }
        else
        {
            command = commandline.substr(0, pos);
            commandline.remove_prefix(pos);
            commandline.remove_prefix(commandline.find_first_not_of(' '));
        }

        if (command == "help")
        {
            console->writeScript("popo/N:");
        }
        else if (command == "cls")
        {
            console->clearScreen();
        }
        else  if (command == "beep")
        {
            auto frequency = getInteger(commandline, 523);
            auto duration = getInteger(commandline, 500);
            beep(frequency, duration);
        }
        else  if (command == "mb")
        {
            auto type = getInteger(commandline, 0);
            systemBeep(type);
        }
        else if ( command == "quit")
        {
            quit();
        }
        else if ( command == "color")
        {
            auto color = getInteger(commandline, 1);
            console->caretColor( static_cast<Console::color_code_type>(color) );
        }
        else if ( command == "perlin")
        {
            //auto color = getInteger(commandline, 1);
            std::cout << "perlin noise:" << std::endl;
            float pmin = 1000, pmax = -1000, sum = 0;
            auto count = 0u;
            for (auto f = 0.0f; f <= 256.0f; f += 0.1f)
            {
                auto p = noise(f);
                sum += p;
                count++;
                if (p > pmax)
                    pmax = p;
                if (p < pmin)
                    pmin = p;
            }
            std::cout << "min: " << pmin << std::endl;
            std::cout << "max: " << pmax << std::endl;
            std::cout << "mean: " << (sum/count) << std::endl;

            //console-> caretColor( static_cast<Console::color_code_type>(color) );
        }
    }

    void OS::onConsoleEvent(std::size_t eventID)
    {
        switch(eventID)
        {
            case 100:
            {
                console->writeLine(console->rows - 2, "Press DEL to enter SETUP; press Alt+F2 to enter utility");
                console->writeLine(console->rows - 1, "21/07/2014-Drcsns-PPP-2.0");
            }
            break;
        }
    }

    void OS::setup(Console& _console)
    {
        console = &_console;
        boot();
    }

    void OS::update()
    {

    }
} // namespace proto