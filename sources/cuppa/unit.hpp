#pragma once

namespace cuppa
{
    struct Meter
    {
        explicit constexpr Meter(float _value) : value_{_value} {}
        constexpr auto operator-() const { return Meter{-value_}; }
        float GetFloat() const { return value_; }

        auto operator+(Meter rhs) const { return Meter{ value_ + rhs.value_ }; }
        auto operator-(Meter rhs) const { return Meter{ value_ - rhs.value_ }; }
        auto operator*(float ratio) const { return Meter{ value_ * ratio }; }
    private:
        float value_;
    };

    struct Move2D;

    struct Point2D
    {
        /*implicit*/ Point2D(Meter _x, Meter _y) : x{_x}, y{_y}  { }

        Point2D operator+(Move2D offset) const;
        Point2D operator-(Move2D offset) const;

        Meter x;
        Meter y;
    };

    // size, displacement, delta
    struct Move2D
    {
        /*implicit*/ Move2D(Meter _width, Meter _height) : width{_width}, height{_height}  { }

        Move2D operator*(float factor) const { return { width * factor, height * factor }; }

        Meter width;
        Meter height;
    };

    inline Point2D Point2D::operator+(Move2D offset) const
    {
        return { x + offset.width, y + offset.height };
    }
    inline Point2D Point2D::operator-(Move2D offset) const
    {
        return { x - offset.width, y - offset.height };
    }
} // namespace cuppa

constexpr auto operator"" _m(unsigned long long int _meter) { return cuppa::Meter{_meter}; }
constexpr auto operator"" _m(long double _meter) { return cuppa::Meter{_meter}; }