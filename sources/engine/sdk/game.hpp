#pragma once

#include "types.hpp"

#define ArrayCount(_ARRAY_) (sizeof(_ARRAY_) / sizeof(*_ARRAY_))

inline constexpr int64 Kilobytes(int64 Value)
{
    return Value * 1024LL;
}

inline constexpr int64 Megabytes(int64 Value)
{
    return Kilobytes(Value) * 1024LL;
}

inline constexpr int64 Gigabytes(int64 Value)
{
    return Megabytes(Value) * 1024LL;
}

inline constexpr int64 Terabytes(int64 Value)
{
    return Gigabytes(Value) * 1024LL;
}

#if __clang__
#define IS_CLANG 1
#define IS_MSVC 0
#elif _MSC_VER
#define IS_CLANG 0
#define IS_MSVC 1
inline auto __builtin_trap()
{
    return __debugbreak();
}
#endif

inline void PIDebugBreak()
{
    if constexpr (IS_CLANG)
    {
        __builtin_trap();
    }
    else if constexpr (IS_MSVC)
    {
        __debugbreak();
    }
}

inline void Check(bool _condition)
{
#if ENABLE_ASSERT
    if (!_condition)
    {
        PIDebugBreak();
    }
#endif
}

inline void Assert([[maybe_unused]] bool _condition)
{
    if constexpr (ENABLE_ASSERT)
    {
        if (!(_condition))
        {
            PIDebugBreak();
        }
    }
}

struct Dimension
{
    int32 Width;
    int32 Height;
};

// Platform independent backbuffer
struct PIBackBuffer
{
    void* Memory;
    int32 Width;
    int32 Height;
    int32 BytesPerPixel;
    int32 Pitch;
};

namespace Game
{
    struct Memory
    {
        Memory(const Memory&) = delete; // non copyable
        virtual ~Memory() {}
        bool32 IsInitialized = false;

        const uint64 PermanentStorageSize;
        // REQUIRED to be cleared to zero at startup
        void* PermanentStorage = nullptr;

        const uint64 TransientStorageSize;
        // REQUIRED to be cleared to zero at startup
        void* TransientStorage = nullptr;

    protected:
        Memory(uint64 PermanentStorageSize, uint64 TransientStorageSize)
            : PermanentStorageSize{ PermanentStorageSize }
            , TransientStorageSize{ TransientStorageSize }
        {}
    };

    struct SoundOutputBuffer
    {
        uint32 SamplesPerSecond;
        uint32 SampleCount;
        int16* Samples;
        bool32 IsValid; // private
    };

    struct Inputs;

    // void UpdateAndRender(Memory&             Memory,
    //                      const Inputs&       Inputs,
    //                      const PIBackBuffer& Buffer,
    //                      SoundOutputBuffer&  SoundBuffer);
} // namespace Game