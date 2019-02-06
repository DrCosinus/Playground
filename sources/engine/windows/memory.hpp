#pragma once

#include "game.hpp"

namespace Windows
{

    class Memory final : public Game::Memory
    {
    public:
        Memory();
        ~Memory() override;
    };

}