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
// normal values have 14 bits of precision for the number of seconds so 16383 -> 4h30minutes... not enough
// let's say after one day the precision of 10ms

// real64 Mantissa is 53bits (52bits stored), exponent 11 bits, sign 1bit