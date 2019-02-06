#include <windows.h>

#include "cpu.hpp"
#include "gameDLL.hpp"
#include "hdtimer.hpp"
#include "scopedTimerResolution.hpp"
#include "win_backbuffer.hpp"
#include "win_inputs.hpp"
#include "win_sound.hpp"
#include "window.hpp"
#include "windowsClass.hpp"

#include <game.hpp>
#include <types.hpp>

#include <cstdio>

namespace Windows
{

    static bool ProcessPendingMessages()
    {
        bool result = true;
        MSG  Message;
        while (result && PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
        {
            switch (Message.message)
            {
            case WM_QUIT:
                result = false;
                break;

            default:
                // TranslateMessage(&Message); // Useless?
                DispatchMessageA(&Message);
                break;
            }
        }
        return result;
    }
#if DEBUG_SOUND
    struct Marker
    {
        DWORD PlayCursor;
        DWORD WriteCursor;
        DWORD ByteToLock;
        DWORD BytesToWrite;

        DWORD FlipPlayCursor;
        DWORD FlipWriteCursor;
    };
    constexpr size_t markerCount = 15;
    Marker           markers[markerCount];
    size_t           currentMarkerIndex = 0;

    constexpr int32 padX       = 16;
    constexpr int32 padY       = 16;
    constexpr int32 lineHeight = 64;
    constexpr DWORD PlayColor  = 0xFFFFFFFF; // White
    constexpr DWORD WriteColor = 0xFFFF0000; // Red
    // constexpr DWORD ExpectedFlipColor = 0xFFFFFF00; // Yellow
    // constexpr DWORD PlayWindowColor   = 0xFFFF00FF; // Purple

    static void DebugDrawSoundBufferMarker(const BackBuffer& backbuffer,
                                           real32            pixelPerBytes,
                                           int32             top,
                                           DWORD             value,
                                           uint32            color)
    {
        auto x = padX + static_cast<int32>(pixelPerBytes * value);
        backbuffer.DebugDrawVertical(x, top, top + lineHeight, color);
    }

    static void DebugDisplaySoundSync(const BackBuffer& backbuffer, const SoundEngine& sndEngine)
    {
        auto pixelPerBytes = static_cast<real32>(backbuffer.Width - 2 * padX) / sndEngine.WorkBufferSize;

        for (size_t markerIndex = 0; markerIndex < markerCount; ++markerIndex)
        {
            auto& marker = markers[markerIndex];

            auto top = padY;
            // auto bottom = top + lineHeight;

            // if (markerIndex == currentMarkerIndex)
            // {
            //     top += lineHeight + padY;
            //     bottom += lineHeight + padY;

            //     auto firstTop = top;
            // }
            DebugDrawSoundBufferMarker(backbuffer, pixelPerBytes, top, marker.PlayCursor, PlayColor);
            DebugDrawSoundBufferMarker(backbuffer, pixelPerBytes, top + 16, marker.WriteCursor, WriteColor);
            DebugDrawSoundBufferMarker(backbuffer,
                                       pixelPerBytes,
                                       top + padY + lineHeight,
                                       marker.FlipPlayCursor,
                                       PlayColor);
            DebugDrawSoundBufferMarker(backbuffer,
                                       pixelPerBytes,
                                       top + padY + lineHeight,
                                       marker.FlipWriteCursor,
                                       WriteColor);

            // if (markerIndex == currentMarkerIndex)
            {
                auto x    = marker.ByteToLock;
                auto xEnd = marker.ByteToLock + marker.BytesToWrite;
                while (x < xEnd)
                {
                    DebugDrawSoundBufferMarker(backbuffer,
                                               pixelPerBytes,
                                               top + (x / sndEngine.WorkBufferSize + 2) * (lineHeight + padY),
                                               x % sndEngine.WorkBufferSize,
                                               (markerIndex % 2) ? 0xFFFFAA55 : 0xFFCC8844);
                    x += DWORD(1 / pixelPerBytes);
                }
            }
        }
    }
#endif // DEBUG_SOUND

    class game_sound_output_buffer;

} // namespace Windows

void check_real64_precision()
{
    auto magic = [](int i, real64 f, real64 g) { printf("%02d: %f + %f = %f\n", i, f, g, f + g); };

    for (int i = 1; i < 9; ++i)
    {
        magic(i, 8710.0 * 365.0 * 24.0 * 3600.0, pow(0.1, (real64)i));
    }
}

// #############################################################################################################
// #############################################################################################################
// #############################################################################################################
// #############################################################################################################
// #############################################################################################################

#include <stdexcept>

namespace Windows
{

    class Memory final : public Game::Memory
    {
    public:
        Memory()
            : Game::Memory{ Megabytes(64), Gigabytes(1) }
            , baseAddress{ reinterpret_cast<LPVOID>(Terabytes(2)) }
        {
            uint64 TotalSize = PermanentStorageSize + TransientStorageSize;
            PermanentStorage = VirtualAlloc(baseAddress, (SIZE_T)TotalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if (!PermanentStorage)
            {
                throw std::domain_error{ "Fail to allocate virtual memory!" };
            }
            TransientStorage = (static_cast<uint8*>(PermanentStorage) + PermanentStorageSize);
        }
        ~Memory() override { VirtualFree(baseAddress, 0, MEM_RELEASE); }

    private:
        void* const baseAddress;
    };

    class Runner final
    {
        // TODO: How do we reliably query on this on Windows? (no more constexpr)
        inline static constexpr uint32 MonitorRefreshHz           = 60;
        inline static constexpr uint32 GameUpdateHz               = MonitorRefreshHz / 2;
        inline static constexpr uint32 TargetMicrosecondsPerFrame = 1'000'000 / GameUpdateHz;

        // order matters
        WindowClass           wndClass; // no dependies, can throw
        BackBuffer            backbuffer; // no dependies
        Window                window; // depends on wndClass, and backbuffer, can throw
        ScopedTimerResolution timerResolution; // no dependencies
        Inputs                inputs; // no dependies, can throw
        SoundEngine           sndEngine; // depends on window
        Memory                memory;
        win32_state           win32State;
        GameDLL               gameDLL; // depends on win32State
        WallClock             lastCounter;

        bool   isRunning      = true; // no dependencies
        uint64 LastCycleCount = __rdtsc(); // no dependencies
        bool   isPaused       = false; // no dependencies

        Runner()
            : backbuffer{ 1280, 720 }
            , window{ wndClass.createNativeWindow(), backbuffer }
            , gameDLL{ win32State, "game_msvc_r.dll", "game.dll" }
            , lastCounter{ WallClock::create() }
        {
            sndEngine.Init(window.getHandle());
            sndEngine.ClearBuffer();
            sndEngine.Play();

            // FIXME: need to be free on exit

            // if (/*Samples &&*/)
            // {
            // }
        }

        void update()
        {
            gameDLL.LookForUpdate();
            inputs.Update();
            isRunning &= !inputs.IsQuitRequested();
            isRunning &= ProcessPendingMessages();

            if (inputs.GetCurrent().Keyboard.Back.EndedDown)
            {
                isRunning = false;
            }
            if (inputs.GetCurrent().GamePads[0].Start.EndedDown &&
                inputs.GetCurrent().GamePads[0].Start.HalfTransitionCount == 1)
            {
                // Pause = !Pause;
            }

            if (!isPaused)
            {
                auto SoundBuffer = sndEngine.PrepareUpdate();
#if DEBUG_SOUND
                markers[currentMarkerIndex].PlayCursor   = sndEngine.lastPlayCursor;
                markers[currentMarkerIndex].WriteCursor  = sndEngine.lastWriteCursor;
                markers[currentMarkerIndex].ByteToLock   = sndEngine.lastByteToLock;
                markers[currentMarkerIndex].BytesToWrite = sndEngine.lastBytesToWrite;
#endif // DEBUG_SOUND
                auto& Buffer = backbuffer.PrepareUpdate();

                thread_context Thread = {};

                if (gameDLL.UpdateAndRender)
                {
                    gameDLL.UpdateAndRender(Thread, memory, inputs.GetCurrent(), Buffer);
                }
                if (gameDLL.GetSoundSamples)
                {
                    // Game.GetSoundSamples(Thread, GameMemory, SoundBuffer);
                }

#if DEBUG_SOUND
                auto sndCursors                             = sndEngine.GetCursors();
                markers[currentMarkerIndex].FlipPlayCursor  = sndCursors.PlayCursor;
                markers[currentMarkerIndex].FlipWriteCursor = sndCursors.WriteCursor;
#endif // DEBUG_SOUND
                if (SoundBuffer.IsValid)
                {
                    sndEngine.FillSoundBuffer(SoundBuffer);
                }

                auto MicrosecondsElapsedForFrame = lastCounter.GetElapsedMicroseconds();
                if (MicrosecondsElapsedForFrame < TargetMicrosecondsPerFrame)
                {
                    auto SleepMicroseconds = TargetMicrosecondsPerFrame - MicrosecondsElapsedForFrame;
                    if (SleepMicroseconds > 0)
                    {
                        // timerResolution.Sleep(static_cast<DWORD>(SleepMicroseconds / 1'000));
                    }
                    // do
                    // {
                    //     MicrosecondsElapsedForFrame = LastCounter.GetElapsedMicroseconds();
                    // } while (MicrosecondsElapsedForFrame < TargetMicrosecondsPerFrame);
                }
                else
                {
                    // TODO: MISSED FRAME RATE!
                    // TODO: Logging
                }

                auto EndCounter{ WallClock::create() };
                auto MSPerFrame = lastCounter.GetElapsedMicroseconds() * 0.001f;
                lastCounter     = EndCounter;

                uint64 EndCycleCount = __rdtsc();
                uint64 CyclesElapsed = EndCycleCount - LastCycleCount;
                LastCycleCount       = EndCycleCount;

                auto FPS  = 1000.0f / (real32)MSPerFrame;
                auto MCPF = (real32)CyclesElapsed / 1'000'000.0f;

                char FPSBuffer[256];
                sprintf_s(FPSBuffer,
                          sizeof(FPSBuffer),
                          "%.02fms/f,  %.02ffps,  %.02fMCycles/Frame\n",
                          MSPerFrame,
                          FPS,
                          MCPF);
                // OutputDebugStringA(FPSBuffer);
#if DEBUG_SOUND
                DebugDisplaySoundSync(backbuffer, sndEngine);
                currentMarkerIndex++;
                if (currentMarkerIndex >= markerCount)
                {
                    currentMarkerIndex = 0;
                }
#endif // DEBUG_SOUND
       // DRAW GREEN VERTICAL LINE IF GAME DLL IS VALID
                constexpr uint32 green = 0x00FF00;
                constexpr uint32 red   = 0xFF0000;
                backbuffer.DebugDrawVertical(0, 0, 100, gameDLL.UpdateAndRender ? green : red);
            }

            window.blitBackBuffer();
        }

        bool is_running() const { return isRunning; }

        ~Runner() = default;

    public:
        static void run()
        {
            Runner runner;
            while (runner.is_running())
            {
                runner.update();
            }
        }
    };
} // namespace Windows

int WinMain(HINSTANCE /*instance*/, HINSTANCE /*PrevInstance*/, LPSTR /*CommandLine*/, int /*ShowCode*/)
{
    try
    {
        Windows::Runner::run();
    } catch (const std::domain_error& e)
    {
        OutputDebugStringA(e.what());
        OutputDebugStringA("\n");
        // std::cerr << e.what() << '\n';
    }
    OutputDebugStringA("Finished\n");
    return 0;
}