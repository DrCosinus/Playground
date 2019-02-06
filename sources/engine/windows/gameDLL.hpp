#pragma once

#include <Windows.h>

namespace Game
{
    struct Memory;
    struct Inputs;
    struct SoundOutputBuffer;

} // namespace Game

struct PIBackBuffer;
struct thread_context;

namespace Windows
{
    using game_update_and_render = void(thread_context&, Game::Memory&, const Game::Inputs&, const PIBackBuffer&);
    using game_get_sound_samples = void(thread_context&, Game::Memory&, Game::SoundOutputBuffer&);

    static constexpr size_t WIN32_STATE_FILE_NAME_COUNT = MAX_PATH;
    struct win32_state
    {
        win32_state() { Win32GetEXEFileName(); }

        char        EXEFileName[WIN32_STATE_FILE_NAME_COUNT];
        const char* OnePastLastEXEFileNameSlash;

    private:
        void Win32GetEXEFileName(); // RetrieveEXEFileName
    };

    class GameDLL final
    {
    public:
        const win32_state& Win32State_;

        GameDLL(const win32_state& Win32State, const char* sourceDLLName, const char* TempDLLName);

        ~GameDLL() { Unload(); }

        void Load();

        void LookForUpdate();

        void Unload();

        bool IsValid() const { return is_valid_; }

        game_update_and_render* UpdateAndRender = nullptr;
        game_get_sound_samples* GetSoundSamples = nullptr;

    private:
        char     SourceGameCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
        char     TempGameCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
        HMODULE  dll_handle_;
        FILETIME dll_last_write_time_;
        bool     is_valid_;
    };

} // namespace Windows