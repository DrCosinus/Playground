#pragma once

#include <memory>

namespace cuppa
{
    class app;
}

extern std::unique_ptr<cuppa::app> createProto();