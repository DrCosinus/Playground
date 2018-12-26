#include "hdtimer.hpp"

#include <windows.h>

namespace Windows
{
    HiDefTimer::HiDefTimer()
    {
        LARGE_INTEGER result;
        QueryPerformanceFrequency(&result);
        Frequency = result.QuadPart;
    }
    WallClock HiDefTimer::GetWallClock() const
    {
        LARGE_INTEGER result;
        QueryPerformanceCounter(&result);
        return { result.QuadPart, *this };
    }

    int64 WallClock::GetElapsedMilliseconds() const
    {
        auto end = HDTimer.GetWallClock();
        return HDTimer.MillisecondsElapsed(*this, end);
    }
    int64 WallClock::GetElapsedMicroseconds() const
    {
        auto end = HDTimer.GetWallClock();
        return HDTimer.MicrosecondsElapsed(*this, end);
    }
}