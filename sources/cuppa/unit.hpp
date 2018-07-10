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
        Pixel& operator++() { value_+=1.0f; return *this; }
        Pixel operator++(int) { Pixel result{value_}; value_+=1.0f; return result; }
        Pixel& operator--() { value_-=1.0f; return *this; }
        Pixel operator--(int) { Pixel result{value_}; value_-=1.0f; return result; }
        Pixel& operator+=(Pixel rhs) { value_+=rhs.value_; return *this; } 
        Pixel& operator-=(Pixel rhs) { value_-=rhs.value_; return *this; } 
        auto operator-(Pixel rhs) const { return Pixel{ value_ - rhs.value_ }; }
        auto operator*(float ratio) const { return Pixel{ value_ * ratio }; }
        friend auto operator*(float ratio, Pixel px) { return px * ratio; }

        auto operator<(Pixel rhs) const { return value_ < rhs.value_; }
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
        constexpr Point() : Point(0_px, 0_px) {}

        Point operator+(Direction offset) const;
        Point operator-(Direction offset) const;
        bool operator==(Point rhs) const { return x==rhs.x && y==rhs.y; }

        Pixel x;
        Pixel y;
    };

    // size, displacement, delta, direction
    struct Direction
    {
        /*implicit*/ constexpr Direction(Pixel _width, Pixel _height) : width{_width}, height{_height}  { }

        Direction operator*(float factor) const { return { width * factor, height * factor }; }

        Pixel width;
        Pixel height;
    };

    inline Point Point::operator+(Direction offset) const
    {
        return { x + offset.width, y + offset.height };
    }
    inline Point Point::operator-(Direction offset) const
    {
        return { x - offset.width, y - offset.height };
    }

    struct Color
    {
        using U8 = unsigned char;
        explicit constexpr Color(U8 _red, U8 _green, U8 _blue, U8 _alpha=255)
        : alpha{ _alpha }, red{ _red }, green{ _green }, blue{ _blue }
        {}
        explicit constexpr Color(U8 _gray, U8 _alpha=255)
        : Color{ _gray, _gray, _gray, _alpha }
        {}

        constexpr Color ModulateAlpha(U8 _alpha) const
        {
            return Color{ red, green, blue, U8(alpha * _alpha / 255) };
        }
        // static constexpr auto FromGray(U8 gray) { return Color{ gray, 255 }; }
        // static constexpr auto FromRGB(U8 red, U8 green, U8 blue) { return Color{ red, green, blue, 255 }; }
        // static constexpr auto FromRGBA(U8 red, U8 green, U8 blue, U8 alpha) { return Color{ red, green, blue, alpha }; }
        //static constexpr auto FromHSV() { return Color{}; }
        //static constexpr auto FromHSB() { return Color{}; }
        //static constexpr unsigned int ToARGB32() { return 0U; }
        U8 GetRed() const { return red; }
        U8 GetGreen() const { return green; }
        U8 GetBlue() const { return blue; }
        U8 GetAlpha() const { return alpha; }
    private:
        U8 alpha, red, green, blue;
    };

} // namespace cuppa
