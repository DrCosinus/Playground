#include "hdtimer.hpp"

#include <windows.h>

namespace Windows
{
    WallClock::HiDefTimer::HiDefTimer()
    {
        LARGE_INTEGER result;
        QueryPerformanceFrequency(&result);
        Frequency = result.QuadPart;
    }

    WallClock WallClock::create()
    {
        LARGE_INTEGER result;
        QueryPerformanceCounter(&result);
        return { result.QuadPart };
    }

    int64 WallClock::GetElapsedMilliseconds() const
    {
        auto end = WallClock::create();
        return 1000LL * (end.data - data) / HDTimer.Frequency;
    }

    int64 WallClock::GetElapsedMicroseconds() const
    {
        auto end = WallClock::create();
        return 1'000'000LL * (end.data - data) / HDTimer.Frequency;
    }
} // namespace Windows