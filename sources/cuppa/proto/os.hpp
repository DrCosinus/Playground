#pragma once

#include <string_view>

namespace proto
{
    struct Console;

    struct OS
    {
        Console* console;

        void boot();
        void setup(Console& _console);
        void update();
        void interpret(std::string_view command);
        void onConsoleEvent(std::size_t eventID);
    };
} // namespace proto