#pragma once

#include "types.hpp"

#define ArrayCount(_ARRAY_) (sizeof(_ARRAY_) / sizeof(*_ARRAY_))

inline real32 mapStick(int16 _value, int16 _deadzone)
{
    return (_value > _deadzone) ? (_value - _deadzone) / (32767.0f - _deadzone)
                                : (_value < -_deadzone ? ((_value + _deadzone) / (32768.0f - _deadzone)) : 0);
}

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

#if DEBUG
#define Assert(condition)                \
    if (!(condition))                    \
    {                                    \
        *static_cast<int*>(nullptr) = 0; \
    }
#else
#define Assert(condition)
#endif

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
    int32 Pitch;
};

namespace Game
{
    struct Memory
    {
        bool32 IsInitialized = false;

        uint64 PermanentStorageSize = 0;
        // REQUIRED to be cleared to zero at startup
        void* PermanentStorage = nullptr;

        uint64 TransientStorageSize = 0;
        // REQUIRED to be cleared to zero at startup
        void* TransientStorage = nullptr;
    };

    struct SoundOutputBuffer
    {
        uint32 SamplesPerSecond;
        uint32 SampleCount;
        int16* Samples;
        bool32 IsValid; // private
        uint32 ByteToLock; // private
        uint32 BytesToWrite; // private
    };

    struct Input;

    void UpdateAndRender(Memory&             Memory,
                         const Input&        Input,
                         const PIBackBuffer& Buffer,
                         SoundOutputBuffer&  SoundBuffer);
} // namespace Game