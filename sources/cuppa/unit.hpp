#pragma once

namespace cuppa
{
    struct Meter
    {
        explicit constexpr Meter(float _value) : value_{_value} {}
        constexpr auto operator-() const { return Meter{-value_}; }
        float GetFloat() const { return value_; }
    private:
        float value_;
    };

    struct Point2D
    {
        /*implicit*/ Point2D(Meter _x, Meter _y) : x{_x}, y{_y}  { }
        Meter x;
        Meter y;
    };

    // size, displacement, delta
    struct Move2D
    {
        /*implicit*/ Move2D(Meter _width, Meter _height) : width{_width}, height{_height}  { }
        Meter width;
        Meter height;
    };
} // namespace cuppa

constexpr auto operator"" _m(unsigned long long int _meter) { return cuppa::Meter{_meter}; }
constexpr auto operator"" _m(long double _meter) { return cuppa::Meter{_meter}; }