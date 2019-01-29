#include "game.hpp"
#include "game_inputs.hpp"

#include "types.hpp"

#include <cmath>
/*
constexpr float Pi32 = 3.14159265359f;

static void GameOutputSound(Game::SoundOutputBuffer& SoundBuffer, int ToneHz)
{
    static real32 tSine;
    int16         ToneVolume = 3000;
    int           WavePeriod = SoundBuffer.SamplesPerSecond / ToneHz;

    int16* SampleOut = SoundBuffer.Samples;
    for (auto SampleIndex = 0U; SampleIndex < SoundBuffer.SampleCount; ++SampleIndex)
    {
        real32 SineValue   = sinf(tSine);
        int16  SampleValue = (int16)(SineValue * ToneVolume * 0); // silence
        *SampleOut++       = SampleValue;
        *SampleOut++       = SampleValue;

        tSine += 2.0f * Pi32 * 1.0f / (real32)WavePeriod;
    }
}*/

static void RenderWeirdGradient(const PIBackBuffer& Buffer, int BlueOffset, int GreenOffset)
{
    auto Row = static_cast<uint8*>(Buffer.Memory);
    for (int Y = 0; Y < Buffer.Height; ++Y)
    {
        auto Pixel = reinterpret_cast<uint32*>(Row);
        for (int X = 0; X < Buffer.Width; ++X)
        {
            uint8 Blue  = (uint8)(X + BlueOffset);
            uint8 Green = (uint8)(Y + GreenOffset);

            *Pixel++ = ((Green << 8) | Blue);
        }

        Row += Buffer.Pitch;
    }
}

namespace Game
{
    struct State
    {
        int ToneHz;
        int GreenOffset;
        int BlueOffset;
    };

    // void ProcessGamepad(State& GameState, const GamePad& gamepad)
    // {
    //     if (gamepad.IsAnalog)
    //     {
    //         GameState.BlueOffset += (int)(4.0f * gamepad.LeftStickX);
    //         GameState.ToneHz = 256 + (int)(128.0f * gamepad.LeftStickY);
    //     }
    //     else
    //     {
    //         if (gamepad.MoveLeft.EndedDown)
    //         {
    //             GameState.BlueOffset -= 1;
    //         }
    //         if (gamepad.MoveRight.EndedDown)
    //         {
    //             GameState.BlueOffset += 1;
    //         }
    //     }
    //     if (gamepad.ActionDown.EndedDown)
    //     {
    //         GameState.GreenOffset += 1;
    //     }
    // }
} // namespace Game

using namespace Game;

extern "C" __declspec(dllexport) void GameUpdateAndRender(thread_context& Thread,
                         Memory& Memory,
                         const Inputs&          Inputs,
                         const PIBackBuffer&    Buffer/*,
                         SoundOutputBuffer&     SoundBuffer*/)
{
    (void)Thread;
    auto& GameState = *static_cast<State*>(Memory.PermanentStorage);
    // if (!Memory.IsInitialized)
    // {
    //     // const char* Filename = __FILE__;

    //     // debug_read_file_result File = DEBUGPlatformReadEntireFile(Filename);
    //     // if (File.Contents)
    //     // {
    //     //     DEBUGPlatformWriteEntireFile("test.out", File.ContentsSize, File.Contents);
    //     //     DEBUGPlatformFreeFileMemory(File.Contents);
    //     // }

    //     // TODO: This may be more appropriate to do in the platform layer
    //     Memory.IsInitialized = true;
    // }
    (void)Inputs;
    // ProcessGamepad(GameState, Inputs.Keyboard);
    // for (auto& gamepad : Inputs.GamePads)
    // {
    //     ProcessGamepad(GameState, gamepad);
    // }

    RenderWeirdGradient(Buffer, GameState.BlueOffset, GameState.GreenOffset);
}

extern "C" __declspec(dllexport) void GameGetSoundSamples(thread_context&    Thread,
                                                          Memory&            Memory,
                                                          SoundOutputBuffer& SoundBuffer)
{
    (void)Thread;
    (void)Memory;
    (void)SoundBuffer;

    auto& GameState = *static_cast<State*>(Memory.PermanentStorage);
    if (!Memory.IsInitialized)
    {
        GameState.ToneHz = 256;

        // TODO: This may be more appropriate to do in the platform layer
        Memory.IsInitialized = true;
    }

    // TODO: Allow sample offsets here for more robust platform options
    // GameOutputSound(SoundBuffer, GameState.ToneHz);
}