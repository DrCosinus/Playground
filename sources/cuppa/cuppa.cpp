#include "cuppa.hpp"

namespace cuppa
{
    void app::run()
    {
        setup();
        while(true)
        {
            update();
            draw();
        }
    }
} // namespace cuppa