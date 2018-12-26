#pragma once

#include <types.hpp>

namespace Windows
{
    struct WallClock
    {
        int64 GetElapsedMilliseconds() const;
        int64 GetElapsedMicroseconds() const;

        WallClock& operator=(const WallClock& _other)
        {
            this->~WallClock();
            new (this) WallClock(_other);
            return *this;
        }

    private:
        friend struct HiDefTimer;

        WallClock(int64 data, const HiDefTimer& HDTimer)
            : data { data }
            , HDTimer { HDTimer }
        {}

        int64             data;
        const HiDefTimer& HDTimer;
    };

    struct HiDefTimer
    {
        HiDefTimer();
        WallClock GetWallClock() const;

    private:
        friend WallClock;
        int64 MillisecondsElapsed(WallClock _start, WallClock _end) const
        {
            return 1000LL * (_end.data - _start.data) / Frequency;
        }
        int64 MicrosecondsElapsed(WallClock _start, WallClock _end) const
        {
            return 1'000'000LL * (_end.data - _start.data) / Frequency;
        }

    private:
        int64 Frequency;
    };
} // namespace Windows