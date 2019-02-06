#pragma once

#include <types.hpp>

namespace Windows
{

    class WallClock final // copyable/movable
    {
    public:
        static WallClock create();

        int64 GetElapsedMilliseconds() const;
        int64 GetElapsedMicroseconds() const;

    private:
        WallClock(int64 data)
            : data{ data }
        {}

        int64            data;
        class HiDefTimer final
        {
        private:
            HiDefTimer();

            friend class WallClock;

            int64 Frequency;
        };
        inline static HiDefTimer HDTimer;
    };

} // namespace Windows