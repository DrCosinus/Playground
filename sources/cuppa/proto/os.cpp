#include "os.hpp"

#include "console.hpp"

namespace proto
{
    namespace
    {
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
            "/E100:"
        );
    }

    void OS::interpret(std::string_view command)
    {
        if (command == "HELP")
        {
            console->writeScript("popo/N:");
        }
        else if (command == "CLS")
        {
            console->clearScreen();
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