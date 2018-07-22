#pragma once

#include "../font.hpp"

#include <cstddef>
#include <queue>
#include <string_view>

namespace proto
{
    struct OS;

    struct Console
    {
        using coord_type = std::size_t;
        using time_milliseconds_type = std::size_t;
        using color_code_type = unsigned char;
        using frame_count_type = std::size_t;
        using event_type = std::size_t;

        static constexpr coord_type cols = 80;
        static constexpr coord_type rows = 30;

        OS* os = nullptr;

        void setup(OS& _os);
        void draw();
        bool ready();
        void writeLine(coord_type row, std::string_view line);
        void writeScript(std::string_view line);
        void scrollup();    // scroll up all rows
        void scrolldown();  // scroll down rows below caret row
        void clearScreen();

        void triggerEvent(event_type eventID);

        void updateScripts();
        void pushChar(char);
        void pushKeyCode(short keyCode);

        void caretColor(color_code_type colorCode) { caret.colorCode = colorCode; }
    private:
        struct caret_info
        {
            coord_type      col = 0;
            coord_type      row = 0;
            color_code_type colorCode = 1;
            frame_count_type blinkCounter = 0;
        };

        struct cell
        {
            char character = ' ';
            color_code_type color = 1;
        };

        cell buffer[cols*rows];
        cuppa::Font font;
        time_milliseconds_type idleTime = 0;
        time_milliseconds_type characterTime = 0;
        time_milliseconds_type lineTime = 0;

        //bool interactive = false;
        std::queue<std::string_view> scripts;
        caret_info caret;
    };
} // namespace proto