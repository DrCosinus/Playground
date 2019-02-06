#include <types.hpp>

namespace Windows
{

    // Set the Windows scheduler granularity to 1ms so that out Sleep() can be more granular.
    class ScopedTimerResolution
    {
    public:
        ScopedTimerResolution();
        ScopedTimerResolution(const ScopedTimerResolution&) = delete; // non copyable
        ~ScopedTimerResolution();

        // Sleep slightly less than the desired amount of milliseconds (rounded to granularity)
        void Sleep(uint32 Milliseconds) const;

    private:
        static uint32 GetMinPeriod();

        const uint32 minPeriod;
    };

} // namespace Windows