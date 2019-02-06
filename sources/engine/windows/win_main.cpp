#include <windows.h>

#include "cpu.hpp"
#include "hdtimer.hpp"
#include "scopedTimerResolution.hpp"
#include "win_backbuffer.hpp"
#include "win_inputs.hpp"
#include "win_sound.hpp"
#include "window.hpp"

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
    class WindowClass final
    {
        friend class Runner;

        static ATOM registerClass(HINSTANCE instance)
        {
            constexpr const char* const windowClassName = "EngineWindowClass";

            WNDCLASSA wndClass     = {};
            wndClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
            wndClass.lpfnWndProc   = MainWindowCB;
            wndClass.hInstance     = instance;
            wndClass.lpszClassName = windowClassName;
            return RegisterClassA(&wndClass);
        }

        static LRESULT MainWindowCB(HWND hwnd, UINT Message, WPARAM WParam, LPARAM LParam)
        {
            LRESULT Result = 0;

            switch (Message)
            {
            case WM_CLOSE:
                // if (MessageBoxA(Window, "Are you sure?", "Quit Application", MB_YESNO | MB_ICONQUESTION) == IDYES)
                {
                    DestroyWindow(hwnd);
                }
                break;

            case WM_DESTROY:
                PostQuitMessage(0);
                break;

            case WM_PAINT:
            {
                auto& window = *reinterpret_cast<Window*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
                window.draw();
            }
            break;

            default:
                Result = DefWindowProcA(hwnd, Message, WParam, LParam);
                break;
            }

            return Result;
        }

        WindowClass()
            : instance{ static_cast<HINSTANCE>(GetModuleHandleA(NULL)) }
            , atom{ registerClass(instance) }
        {
            if (!atom)
            {
                throw std::domain_error{ "Fail to register window class" };
            }
        }
        ~WindowClass() { UnregisterClassA(GetClassname(), instance); }

        HWND createNativeWindow() const
        {
            constexpr const char* const windowName = "Game";

            return CreateWindowExA(0,
                                   GetClassname(),
                                   windowName,
                                   WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                   CW_USEDEFAULT,
                                   CW_USEDEFAULT,
                                   CW_USEDEFAULT,
                                   CW_USEDEFAULT,
                                   0,
                                   0,
                                   instance,
                                   0);
        }

        LPCSTR GetClassname() const { return reinterpret_cast<LPCSTR>(atom); }

        HINSTANCE instance;
        ATOM      atom;
    };

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

        struct config
        {
            WindowClass wndClass;
        };

        // order matters
        WindowClass           wndClass; // no dependies, can throw
        BackBuffer            backbuffer; // no dependies
        Window                window; // depends on wndClass, and backbuffer, can throw
        ScopedTimerResolution timerResolution; // no dependencies
        Inputs                inputs; // no dependies, can throw
        SoundEngine           sndEngine; // depends on window
        Memory                memory;
        win32_state           win32State;
        GameCode              gameDLL; // depends on win32State
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
                (void)Thread;
                (void)Buffer;
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