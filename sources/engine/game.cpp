#include "game.hpp"
#include "game_inputs.hpp"

#include <cmath>

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
        int16  SampleValue = (int16)(SineValue * ToneVolume);
        *SampleOut++       = SampleValue;
        *SampleOut++       = SampleValue;

        tSine += 2.0f * Pi32 * 1.0f / (real32)WavePeriod;
    }
}

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

    void ProcessGamepad(State& GameState, const GamePad& gamepad)
    {
        if (gamepad.IsAnalog)
        {
            GameState.BlueOffset += (int)(4.0f * gamepad.LeftStickX);
            GameState.ToneHz = 256 + (int)(128.0f * gamepad.LeftStickY);
        }
        else
        {
            if (gamepad.MoveLeft.EndedDown)
            {
                GameState.BlueOffset -= 1;
            }
            if (gamepad.MoveRight.EndedDown)
            {
                GameState.BlueOffset += 1;
            }
        }
        if (gamepad.ActionDown.EndedDown)
        {
            GameState.GreenOffset += 1;
        }
    }

    void UpdateAndRender(Memory& Memory, const Input& Input, const PIBackBuffer& Buffer, SoundOutputBuffer& SoundBuffer)
    {
        auto& GameState = *static_cast<State*>(Memory.PermanentStorage);
        if (!Memory.IsInitialized)
        {
            // const char* Filename = __FILE__;

            // debug_read_file_result File = DEBUGPlatformReadEntireFile(Filename);
            // if (File.Contents)
            // {
            //     DEBUGPlatformWriteEntireFile("test.out", File.ContentsSize, File.Contents);
            //     DEBUGPlatformFreeFileMemory(File.Contents);
            // }

            GameState.ToneHz = 256;

            // TODO: This may be more appropriate to do in the platform layer
            Memory.IsInitialized = true;
        }

        ProcessGamepad(GameState, Input.Keyboard);
        for (auto& gamepad : Input.GamePads)
        {
            ProcessGamepad(GameState, gamepad);
        }

        // TODO: Allow sample offsets here for more robust platform options
        GameOutputSound(SoundBuffer, GameState.ToneHz);
        RenderWeirdGradient(Buffer, GameState.BlueOffset, GameState.GreenOffset);
    }
} // namespace Game