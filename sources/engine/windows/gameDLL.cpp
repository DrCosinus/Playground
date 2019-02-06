#include "gameDLL.hpp"

#include <cstdio>

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

// TODO: should return the path (no out parameter... yuck!)
static void BuildPath(const Windows::win32_state& State, const char* FileName, size_t DestCount, char* Dest)
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

namespace Windows
{

    void win32_state::Win32GetEXEFileName()
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

    GameDLL::GameDLL(const win32_state& Win32State, const char* sourceDLLName, const char* TempDLLName)
        : Win32State_{ Win32State }
    {
        BuildPath(Win32State, sourceDLLName, sizeof(SourceGameCodeDLLFullPath), SourceGameCodeDLLFullPath);
        BuildPath(Win32State_, TempDLLName, sizeof(TempGameCodeDLLFullPath), TempGameCodeDLLFullPath);

        Load();
    }

    void GameDLL::Load()
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

    void GameDLL::LookForUpdate()
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

    void GameDLL::Unload()
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

} // namespace Windows