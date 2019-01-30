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

    class game_sound_output_buffer;
    using game_update_and_render = void(thread_context&, Game::Memory&, const Game::Inputs&, const PIBackBuffer&);
    using game_get_sound_samples = void(thread_context&, Game::Memory&, Game::SoundOutputBuffer&);

    static void CatStrings(size_t      SourceACount,
                           const char* SourceA,
                           size_t      SourceBCount,
                           const char* SourceB,
                           size_t /*DestCount*/,
                           char* Dest)
    {
        // TODO: Dest bounds checking!

        for (int Index = 0; Index < SourceACount; ++Index)
        {
            *Dest++ = *SourceA++;
        }

        for (int Index = 0; Index < SourceBCount; ++Index)
        {
            *Dest++ = *SourceB++;
        }

        *Dest++ = 0;
    }

    static size_t StringLength(const char* String)
    {
        size_t Count = 0;
        while (*String++)
        {
            ++Count;
        }
        return Count;
    }

    static constexpr size_t WIN32_STATE_FILE_NAME_COUNT = MAX_PATH;
    struct win32_state
    {
        win32_state() { Win32GetEXEFileName(); }

        char        EXEFileName[WIN32_STATE_FILE_NAME_COUNT];
        const char* OnePastLastEXEFileNameSlash;

    private:
        void Win32GetEXEFileName()
        {
            /*DWORD SizeOfFilename        =*/GetModuleFileNameA(0, EXEFileName, sizeof(EXEFileName));
            OnePastLastEXEFileNameSlash = EXEFileName;
            // TODO: should consider begin from the end
            for (char* Scan = EXEFileName; *Scan; ++Scan)
            {
                if (*Scan == '\\' || *Scan == '/')
                {
                    OnePastLastEXEFileNameSlash = Scan + 1;
                }
            }
        }
    };

    struct GameCode final
    {
        const win32_state& Win32State_;
        GameCode(const win32_state& Win32State, const char* sourceDLLName, const char* TempDLLName)
            : Win32State_{ Win32State }
        {
            BuildPath(Win32State, sourceDLLName, sizeof(SourceGameCodeDLLFullPath), SourceGameCodeDLLFullPath);
            BuildPath(Win32State_, TempDLLName, sizeof(TempGameCodeDLLFullPath), TempGameCodeDLLFullPath);

            Load();
        }

        ~GameCode() { Unload(); }

        void Load()
        {
            // for now, during development we use a copy of the DLL so we can rebuild the DLL when the copy is used
            // maybe a better way to handle this is to track new DLL in another folder and copy it if needed
            if (!CopyFileA(SourceGameCodeDLLFullPath, TempGameCodeDLLFullPath, FALSE))
            {
                char buff[64];
                sprintf_s(buff, "DLL copy failed. Error #%d.\n", GetLastError());
                OutputDebugStringA(buff);
                return;
            }

            OutputDebugStringA("Loading ");
            OutputDebugStringA(SourceGameCodeDLLFullPath);
            OutputDebugStringA("\n");
            dll_last_write_time_ = Win32GetLastWriteTime(SourceGameCodeDLLFullPath);

            dll_handle_ = LoadLibraryA(TempGameCodeDLLFullPath);
            if (dll_handle_)
            {
                UpdateAndRender =
                    reinterpret_cast<game_update_and_render*>(GetProcAddress(dll_handle_, "GameUpdateAndRender"));
                GetSoundSamples =
                    reinterpret_cast<game_get_sound_samples*>(GetProcAddress(dll_handle_, "GameGetSoundSamples"));
                is_valid_ = UpdateAndRender && GetSoundSamples;
            }
            else
            {
                is_valid_ = false;
            }
            if (!is_valid_)
            {
                UpdateAndRender = nullptr;
                GetSoundSamples = nullptr;
            }
        }

        void LookForUpdate()
        {
            auto NewDLLWriteTime = Win32GetLastWriteTime(SourceGameCodeDLLFullPath);

            if (CompareFileTime(&NewDLLWriteTime, &dll_last_write_time_) != 0)
            {
                // auto handle = CreateFileA(SourceGameCodeDLLFullPath,
                //                           GENERIC_READ | GENERIC_WRITE,
                //                           0,
                //                           NULL,
                //                           OPEN_EXISTING,
                //                           FILE_ATTRIBUTE_NORMAL,
                //                           NULL);

                // FIXME: yuck... horrible...
                char xxx[WIN32_STATE_FILE_NAME_COUNT];

                BuildPath(Win32State_, "game.lld", sizeof(xxx), xxx);

                if (CopyFileA(SourceGameCodeDLLFullPath, xxx, FALSE))
                {
                    DeleteFileA(xxx);
                    // CloseHandle(handle);
                    Unload();
                    Load();
                }
                // Sleep(1000); // we detect the change to early
                // Unload();
            }
        }

        void Unload()
        {
            if (dll_handle_)
            {
                OutputDebugStringA("Unloading Game DLL.\n");
                FreeLibrary(dll_handle_);
                if (!FreeLibrary(dll_handle_))
                {
                    // if (!DeleteFileA(TempGameCodeDLLFullPath))
                    // {
                    //     char buff[64];
                    //     sprintf_s(buff, "Delete Error #%d", GetLastError());
                    //     OutputDebugStringA(buff);
                    // }
                }
                // CoFreeUnusedLibraries();
                dll_handle_ = 0;
            }
            is_valid_       = false;
            UpdateAndRender = nullptr;
            GetSoundSamples = nullptr;
        }

        bool IsValid() const { return is_valid_; }

        game_update_and_render* UpdateAndRender = nullptr;
        game_get_sound_samples* GetSoundSamples = nullptr;

    private:
        // TODO: should return the path (no out parameter... yuck!)
        static void BuildPath(const win32_state& State, const char* FileName, size_t DestCount, char* Dest)
        {
            CatStrings(State.OnePastLastEXEFileNameSlash - State.EXEFileName,
                       State.EXEFileName,
                       StringLength(FileName),
                       FileName,
                       DestCount,
                       Dest);
        }

        static FILETIME Win32GetLastWriteTime(const char* Filename)
        {
            if (WIN32_FILE_ATTRIBUTE_DATA Data; GetFileAttributesExA(Filename, GetFileExInfoStandard, &Data))
            {
                return Data.ftLastWriteTime;
            }
            return { 0, 0 };
        }

        char     SourceGameCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
        char     TempGameCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
        HMODULE  dll_handle_;
        FILETIME dll_last_write_time_;
        bool     is_valid_;
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

                    win32_state Win32State;
                    GameCode    Game{ Win32State, "game_msvc_r.dll", "game.dll" };
                    bool        GlobalRunning  = true;
                    uint64      LastCycleCount = __rdtsc();
                    while (GlobalRunning)
                    {
                        Game.LookForUpdate();
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
                            // Pause = !Pause;
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

                            thread_context Thread = {};
                            (void)Thread;
                            (void)Buffer;
                            if (Game.UpdateAndRender)
                            {
                                Game.UpdateAndRender(Thread, GameMemory, inputs.GetCurrent(), Buffer);
                            }
                            if (Game.GetSoundSamples)
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

                            auto MicrosecondsElapsedForFrame = LastCounter.GetElapsedMicroseconds();
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
       // DRAW GREEN VERTICAL LINE IF GAME DLL IS VALID
                            DebugBackBuffer{ backbuffer }.DebugDrawVertical(0,
                                                                            0,
                                                                            100,
                                                                            Game.UpdateAndRender ? 0x00FF00 : 0xFF0000);
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

void check_real64_precision()
{
    auto magic = [](int i, real64 f, real64 g) { printf("%02d: %f + %f = %f\n", i, f, g, f + g); };

    for (int i = 1; i < 9; ++i)
    {
        magic(i, 8710.0 * 365.0 * 24.0 * 3600.0, pow(0.1, (real64)i));
    }
}

int WinMain(HINSTANCE Instance, HINSTANCE /*PrevInstance*/, LPSTR /*CommandLine*/, int /*ShowCode*/)
{
    check_real64_precision();
    // cpu_info();
    return Windows::Main(Instance);
}
