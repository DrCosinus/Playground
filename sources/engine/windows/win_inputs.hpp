#pragma once

#include <game_inputs.hpp>

#include <memory>

namespace Windows
{

    struct Inputs
    {
        // NOTE: even if they are trivial, the ctor & dtor definitions must be in the source file, the only place where
        // the complete definition of Driver is known
        Inputs();
        ~Inputs();

        void Init();
        void Update();

        bool32 IsQuitRequested() const { return QuitRequested; }
        auto   GetCurrent() const { return PIInputs[CurrentInput]; }

    private:
        struct Driver;
        std::unique_ptr<Driver> driver;

        void ProcessPendingMessages(Game::GamePad& KeyboardController);

        // Platform Independent Inputs (Double buffer for transition detection)
        Game::Inputs PIInputs[2]   = {};
        uint32       CurrentInput  = 0;
        bool32       QuitRequested = false;
    };

} // namespace Windows