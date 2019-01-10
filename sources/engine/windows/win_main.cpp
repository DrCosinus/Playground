#include <windows.h>

#include "cpu.hpp"
#include "hdtimer.hpp"
#include "win_inputs.hpp"
#include "win_sound.hpp"

#include <game.hpp>
#include <types.hpp>

#include <cstdio>

namespace Windows
{
    static Dimension GetWindowDimension(HWND Window)
    {
        RECT ClientRect;
        GetClientRect(Window, &ClientRect);

        return { ClientRect.right - ClientRect.left, ClientRect.bottom - ClientRect.top };
    }

    struct WinBackBuffer : PIBackBuffer
    {
        static constexpr int BytesPerPixel = 4;

        void Resize(int _Width, int _Height)
        {
            if (Memory)
            {
                VirtualFree(Memory, 0, MEM_RELEASE);
            }
            Width  = _Width;
            Height = _Height;

            Info.bmiHeader.biSize        = sizeof(Info.bmiHeader);
            Info.bmiHeader.biWidth       = Width;
            Info.bmiHeader.biHeight      = -Height;
            Info.bmiHeader.biPlanes      = 1;
            Info.bmiHeader.biBitCount    = 32;
            Info.bmiHeader.biCompression = BI_RGB;

            int BitmapMemorySize = Width * Height * BytesPerPixel;
            Memory               = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            Pitch                = Width * BytesPerPixel;

            // Clear this to black: Memory allocated by VirtualAlloc is automatically initialized to zero.
        }

        void DisplayBufferInWindow(HDC _DeviceContext, HWND _Window)
        {
            auto Dimension = GetWindowDimension(_Window);
            // TODO: Aspect ratio correction
            // TODO: Play with stretch modes
            StretchDIBits(_DeviceContext,
                          0,
                          0,
                          Dimension.Width,
                          Dimension.Height,
                          0,
                          0,
                          Width,
                          Height,
                          Memory,
                          &Info,
                          DIB_RGB_COLORS,
                          SRCCOPY);
        }

        const PIBackBuffer& PrepareUpdate() const { return *this; }

    private:
        // Pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
        BITMAPINFO Info;
    };

    struct DebugBackBuffer : WinBackBuffer
    {
        void DebugDrawVertical(int32 X, int32 top, int32 bottom, uint32 color) const
        {
            if (top > Height || bottom < 0)
            {
                return;
            }
            if (top <= 0)
            {
                top = 0;
            }

            if (bottom > Height)
            {
                bottom = Height;
            }

            if ((X >= 0) && (X < Width))
            {
                auto pixel = static_cast<uint8*>(Memory) + X * BytesPerPixel + top * Pitch;
                for (int Y = top; Y < bottom; ++Y)
                {
                    *reinterpret_cast<uint32*>(pixel) = color;
                    pixel += Pitch;
                }
            }
        }
    };

    static LRESULT MainWindowCB(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
    {
        LRESULT Result = 0;

        switch (Message)
        {
        case WM_CLOSE:
            // if (MessageBoxA(Window, "Are you sure?", "Quit Application", MB_YESNO | MB_ICONQUESTION) == IDYES)
            {
                DestroyWindow(Window);
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC         DeviceContext = BeginPaint(Window, &Paint);
            auto&       backbuffer    = *reinterpret_cast<WinBackBuffer*>(GetWindowLongPtrA(Window, GWLP_USERDATA));
            backbuffer.DisplayBufferInWindow(DeviceContext, Window);
            EndPaint(Window, &Paint);
        }
        break;

        default:
            Result = DefWindowProcA(Window, Message, WParam, LParam);
            break;
        }

        return Result;
    }

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

    static void DebugDrawSoundBufferMarker(const DebugBackBuffer& backbuffer,
                                           real32                 pixelPerBytes,
                                           int32                  top,
                                           DWORD                  value,
                                           uint32                 color)
    {
        auto x = padX + static_cast<int32>(pixelPerBytes * value);
        backbuffer.DebugDrawVertical(x, top, top + lineHeight, color);
    }

    static void DebugDisplaySoundSync(const DebugBackBuffer& backbuffer, const SoundEngine& sndEngine)
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

    // Set the Windows scheduler granularity to 1ms so that out Sleep() can be more granular.
    struct ScopedTimerResolution
    {
        ScopedTimerResolution()
            : timeCaps{ GetDevCaps() }
        {
            Check(timeBeginPeriod(timeCaps.wPeriodMin) == TIMERR_NOERROR);
        }
        ~ScopedTimerResolution() { Check(timeEndPeriod(timeCaps.wPeriodMin) == TIMERR_NOERROR); }
        // Sleep slightly less than the desired amount of milliseconds (rounded to granularity)
        void Sleep(uint32 Milliseconds) const
        {
            ::Sleep(Milliseconds - (Milliseconds % timeCaps.wPeriodMin) - timeCaps.wPeriodMin);
        }

    private:
        static TIMECAPS GetDevCaps()
        {
            TIMECAPS timeCaps;
            timeGetDevCaps(&timeCaps, sizeof(timeCaps));
            return timeCaps;
        }

        const TIMECAPS timeCaps;
    };

    int Main(HINSTANCE Instance)
    {
        WNDCLASSA WindowClass   = {};
        WindowClass.style       = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        WindowClass.lpfnWndProc = MainWindowCB;
        WindowClass.hInstance   = Instance;
        //    WindowClass.hIcon;
        WindowClass.lpszClassName = "EngineWindowClass";

        ScopedTimerResolution timerResolution;

        // TODO: How do we reliably query on this on Windows? (no more constexpr)
        constexpr uint32 MonitorRefreshHz           = 60;
        constexpr uint32 GameUpdateHz               = MonitorRefreshHz / 2;
        constexpr uint32 TargetMicrosecondsPerFrame = 1'000'000 / GameUpdateHz;

        if (RegisterClassA(&WindowClass))
        {
            HWND Window = CreateWindowExA(0,
                                          WindowClass.lpszClassName,
                                          "Engine",
                                          WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                          CW_USEDEFAULT,
                                          CW_USEDEFAULT,
                                          CW_USEDEFAULT,
                                          CW_USEDEFAULT,
                                          0,
                                          0,
                                          Instance,
                                          0);
            if (Window)
            {
                // Windows specific stuffs
                HiDefTimer    hdTimer;
                Inputs        inputs;
                WinBackBuffer backbuffer;
                SoundEngine   sndEngine;

                backbuffer.Resize(1280, 720);

                // NOTE: Store backbuffer structure pointer in the user date of the window
                SetWindowLongPtrA(Window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&backbuffer));

                // Since we specified CS_OWNDC, we can just get one device context and use it forever because we are not
                // sharing it with anyone.
                HDC DeviceContext = GetDC(Window);

                inputs.Init();
                sndEngine.Init(Window);
                sndEngine.ClearBuffer();
                sndEngine.Play();

                bool Pause = false;

                Game::Memory GameMemory;
                GameMemory.PermanentStorageSize = Megabytes(64);
                GameMemory.TransientStorageSize = Gigabytes(1);

                LPVOID BaseAddress = reinterpret_cast<LPVOID>(Terabytes(2));
                uint64 TotalSize   = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
                GameMemory.PermanentStorage =
                    VirtualAlloc(BaseAddress, (size_t)TotalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
                GameMemory.TransientStorage =
                    (static_cast<uint8*>(GameMemory.PermanentStorage) + GameMemory.PermanentStorageSize);
                // FIXME: need to be free on exit

                if (/*Samples &&*/ GameMemory.PermanentStorage && GameMemory.TransientStorage)
                {
                    auto LastCounter = hdTimer.GetWallClock();

                    bool   GlobalRunning  = true;
                    uint64 LastCycleCount = __rdtsc();
                    while (GlobalRunning)
                    {
                        inputs.Update();
                        GlobalRunning &= !inputs.IsQuitRequested();
                        GlobalRunning &= ProcessPendingMessages();

                        if (inputs.GetCurrent().Keyboard.Back.EndedDown)
                        {
                            GlobalRunning = false;
                        }
                        if (inputs.GetCurrent().GamePads[0].Start.EndedDown &&
                            inputs.GetCurrent().GamePads[0].Start.HalfTransitionCount == 1)
                        {
                            Pause = !Pause;
                        }

                        if (!Pause)
                        {
                            auto SoundBuffer = sndEngine.PrepareUpdate();
#if DEBUG_SOUND
                            markers[currentMarkerIndex].PlayCursor   = sndEngine.lastPlayCursor;
                            markers[currentMarkerIndex].WriteCursor  = sndEngine.lastWriteCursor;
                            markers[currentMarkerIndex].ByteToLock   = sndEngine.lastByteToLock;
                            markers[currentMarkerIndex].BytesToWrite = sndEngine.lastBytesToWrite;
#endif // DEBUG_SOUND
                            auto& Buffer = backbuffer.PrepareUpdate();

                            Game::UpdateAndRender(GameMemory, inputs.GetCurrent(), Buffer, SoundBuffer);

#if DEBUG_SOUND
                            auto sndCursors                             = sndEngine.GetCursors();
                            markers[currentMarkerIndex].FlipPlayCursor  = sndCursors.PlayCursor;
                            markers[currentMarkerIndex].FlipWriteCursor = sndCursors.WriteCursor;
#endif // DEBUG_SOUND
                            if (SoundBuffer.IsValid)
                            {
                                sndEngine.FillSoundBuffer(SoundBuffer);
                            }

                            auto MicrosecondsElapsedForFrame = LastCounter.GetElapsedMicroseconds();
                            if (MicrosecondsElapsedForFrame < TargetMicrosecondsPerFrame)
                            {
                                auto SleepMicroseconds = TargetMicrosecondsPerFrame - MicrosecondsElapsedForFrame;
                                if (SleepMicroseconds > 0)
                                {
                                    timerResolution.Sleep(static_cast<DWORD>(SleepMicroseconds / 1'000));
                                }
                                do
                                {
                                    MicrosecondsElapsedForFrame = LastCounter.GetElapsedMicroseconds();
                                } while (MicrosecondsElapsedForFrame < TargetMicrosecondsPerFrame);
                            }
                            else
                            {
                                // TODO: MISSED FRAME RATE!
                                // TODO: Logging
                            }

                            auto EndCounter = hdTimer.GetWallClock();
                            auto MSPerFrame = LastCounter.GetElapsedMicroseconds() * 0.001f;
                            LastCounter     = EndCounter;

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
                            DebugDisplaySoundSync(DebugBackBuffer{ backbuffer }, sndEngine);
                            currentMarkerIndex++;
                            if (currentMarkerIndex >= markerCount)
                            {
                                currentMarkerIndex = 0;
                            }
#endif // DEBUG_SOUND
                        }
                        backbuffer.DisplayBufferInWindow(DeviceContext, Window);
                    }
                }
                else
                {
                    // TODO: Logging
                }
            }
            else
            {
                // TODO: Logging
            }
        }
        else
        {
            // TODO: Logging
        }

        return 0;
    }

} // namespace Windows

void check_floatingpoint32_precision()
{
    auto magic = [](float f, float g) { printf("%f + %f = %f\n", f, g, f + g); };

    for (int i = 8; i < 16; ++i)
    {
        magic(powf(2, 14) - 1, powf(2, (float)-i));
    }
}

int WinMain(HINSTANCE Instance, HINSTANCE /*PrevInstance*/, LPSTR /*CommandLine*/, int /*ShowCode*/)
{
    check_floatingpoint32_precision();
    cpu_info();
    return Windows::Main(Instance);
}
