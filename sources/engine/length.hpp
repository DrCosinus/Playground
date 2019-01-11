#pragma once

#include "types.hpp"

template <int64 LENGTH_DIMENSION, int64 WEIGHT_DIMENSION, int64 TIME_DIMENSION>
struct Quantity
{
    template <int64 L, int64 W, int64 T>
    Quantity& operator+(Quantity<L, W, T> _other)
    {
        static_assert(LENGTH_DIMENSION == L && WEIGHT_DIMENSION == W && TIME_DIMENSION == T,
                      "Incompatible unit dimensions");
        value_ += _other.value_;
    }

private:
    explicit Quantity(float _value)
        : value_{ _value }
    {}

    float value_;
};

// real32 Mantissa is 24bits (23bits stored), exponent 8 bits, sign 1bit
// Time in seconds with a precision of a milliseconds -> 10 bits
// values have 13 bits of stored precision for the number of seconds so 16383 -> 4h30minutes... not enough
// let's say after one day the precision of 10ms

// real64 Mantissa is 53bits (52bits stored), exponent 11 bits, sign 1bit
// precision better than tenth of a milliseconds -> 14 bits
// values have 38 bits of stored precision => 549755813887 seconds => more than 17480 years => OK

// if we consider that time will in range [-8710 years, 8710years] we need to able to store 38bits
// so it remains 18bits of precision to subdivide a second

// need to test performances