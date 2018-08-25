#pragma once

#include <type_traits>

namespace cuppa
{
    struct Pixel
    {
        template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
        explicit constexpr Pixel(T _value) : value_{static_cast<float>(_value)} {}
        constexpr auto operator-() const { return Pixel{-value_}; }
        template<typename T>
        T getAs() const { return static_cast<T>(value_); }

        bool operator==(Pixel rhs) const { return value_ == rhs.value_; }
        auto operator+(Pixel rhs) const { return Pixel{ value_ + rhs.value_ }; }
        auto operator-(Pixel rhs) const { return Pixel{ value_ - rhs.value_ }; }
        template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
        auto operator*(T ratio) const { return Pixel{ value_ * static_cast<float>(ratio) }; }
        template<typename T>
        friend auto operator*(T ratio, std::enable_if_t<std::is_arithmetic_v<T>, Pixel> px) { return px * ratio; }
        auto operator/(Pixel rhs) const { return value_ / rhs.value_; }
        template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
        auto operator/(T divider) { return Pixel( value_/divider ); } // possible division by zero

        Pixel& operator++() { value_+=1.0f; return *this; }
        Pixel operator++(int) { Pixel result{value_}; value_+=1.0f; return result; }
        Pixel& operator--() { value_-=1.0f; return *this; }
        Pixel operator--(int) { Pixel result{value_}; value_-=1.0f; return result; }
        Pixel& operator+=(Pixel rhs) { value_+=rhs.value_; return *this; } 
        Pixel& operator-=(Pixel rhs) { value_-=rhs.value_; return *this; } 

        auto operator<(Pixel rhs) const { return value_ < rhs.value_; }
        friend auto operator>(Pixel lhs, Pixel rhs) { return rhs < lhs; }
    private:
        float value_;
    };
} // namespace cuppa

constexpr auto operator"" _px(unsigned long long int _pixel) { return cuppa::Pixel{_pixel}; }
constexpr auto operator"" _px(long double _pixel) { return cuppa::Pixel{_pixel}; }

namespace cuppa
{
    struct Direction;

    struct Point
    {
        /*implicit*/ constexpr Point(Pixel _x, Pixel _y) : x{_x}, y{_y}  { }
        constexpr Point() : Point{ 0_px, 0_px } {}

        constexpr Point operator+(Direction offset) const;
        constexpr Point operator-(Direction offset) const;
        Point& operator+=(Direction offset);

        bool operator==(Point rhs) const { return x==rhs.x && y==rhs.y; }

        Pixel x;
        Pixel y;
    };

    // size, displacement, delta, direction
    struct Direction
    {
        /*implicit*/ constexpr Direction(Pixel _width, Pixel _height) : width{_width}, height{_height}  { }
        constexpr Direction() : Direction{ 0_px, 0_px } {}

        Direction operator*(float factor) const { return { width * factor, height * factor }; }

        Direction flipY() const { return { width, -height }; }

        Pixel width;
        Pixel height;
    };

    inline constexpr Point Point::operator+(Direction offset) const
    {
        return { x + offset.width, y + offset.height };
    }
    inline constexpr Point Point::operator-(Direction offset) const
    {
        return { x - offset.width, y - offset.height };
    }
    inline Point& Point::operator+=(Direction offset)
    {
        x += offset.width;
        y += offset.height;
        return *this;
    }
} // namespace cuppa
