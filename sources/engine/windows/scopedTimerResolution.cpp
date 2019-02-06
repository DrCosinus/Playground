#include "scopedTimerResolution.hpp"

#include "game.hpp"

#include <windows.h>

namespace Windows
{

    ScopedTimerResolution::ScopedTimerResolution()
        : minPeriod{ GetMinPeriod() }
    {
        Check(timeBeginPeriod(minPeriod) == TIMERR_NOERROR);
    }

    ScopedTimerResolution ::~ScopedTimerResolution() { Check(timeEndPeriod(minPeriod) == TIMERR_NOERROR); }

    // Sleep slightly less than the desired amount of milliseconds (rounded to granularity)
    void ScopedTimerResolution::Sleep(uint32 Milliseconds) const
    {
        ::Sleep(Milliseconds - (Milliseconds % minPeriod) - minPeriod);
    }

    uint32 ScopedTimerResolution::GetMinPeriod()
    {
        TIMECAPS timeCaps;
        timeGetDevCaps(&timeCaps, sizeof(timeCaps));
        return timeCaps.wPeriodMin;
    }

}; // namespace Windows
