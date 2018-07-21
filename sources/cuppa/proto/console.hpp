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
        cuppa::Font font;
        std::size_t idleTime = 0;
        std::size_t characterTime = 0;
        std::size_t lineTime = 0;
        unsigned char caret_color = 1;

        std::size_t caretBlinkCounter = 0;
        bool interactive = false;
        std::queue<std::string_view> scripts;

        OS* os = nullptr;

        void setup(OS& _os);
        void draw();
        bool ready();
        void writeLine(std::size_t row, std::string_view line);
        void writeScript(std::string_view line);
        void scrollup();    // scroll up all rows
        void scrolldown();  // scroll down rows below caret row
        void clearScreen();

        void triggerEvent(std::size_t eventID);

        void updateScripts();
        void pushChar(char);
        void pushKeyCode(short keyCode);
    };
} // namespace proto