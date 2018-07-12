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
        auto operator*(float ratio) const { return Pixel{ value_ * ratio }; }
        friend auto operator*(float ratio, Pixel px) { return px * ratio; }

        Pixel& operator++() { value_+=1.0f; return *this; }
        Pixel operator++(int) { Pixel result{value_}; value_+=1.0f; return result; }
        Pixel& operator--() { value_-=1.0f; return *this; }
        Pixel operator--(int) { Pixel result{value_}; value_-=1.0f; return result; }
        Pixel& operator+=(Pixel rhs) { value_+=rhs.value_; return *this; } 
        Pixel& operator-=(Pixel rhs) { value_-=rhs.value_; return *this; } 

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
        using COMPONENT = short;
        using diff_type = Color;
        explicit constexpr Color(COMPONENT _red, COMPONENT _green, COMPONENT _blue, COMPONENT _alpha=255)
        : alpha{ _alpha }, red{ _red }, green{ _green }, blue{ _blue }
        {}
        explicit constexpr Color(COMPONENT _gray, COMPONENT _alpha=255)
        : Color{ _gray, _gray, _gray, _alpha }
        {}

        constexpr Color ModulateAlpha(COMPONENT _alpha) const
        {
            return Color{ red, green, blue, COMPONENT(alpha * _alpha / 255) };
        }

        constexpr auto operator-(const Color& rhs) const { return diff_type{ red - rhs.red, green - rhs.green, blue - rhs.blue, alpha - rhs.alpha }; }
        constexpr auto operator*(float scale) const { return Color{ static_cast<COMPONENT>(scale*red), static_cast<COMPONENT>(scale*green), static_cast<COMPONENT>(scale*blue), static_cast<COMPONENT>(scale*alpha) }; }
        constexpr auto operator+(const diff_type& rhs) const { return Color{ red + rhs.red, green + rhs.green, blue + rhs.blue, alpha + rhs.alpha }; }
        // static constexpr auto FromGray(U8 gray) { return Color{ gray, 255 }; }
        // static constexpr auto FromRGB(U8 red, U8 green, U8 blue) { return Color{ red, green, blue, 255 }; }
        // static constexpr auto FromRGBA(U8 red, U8 green, U8 blue, U8 alpha) { return Color{ red, green, blue, alpha }; }
        //static constexpr auto FromHSV() { return Color{}; }
        //static constexpr auto FromHSB() { return Color{}; }
        //static constexpr unsigned int ToARGB32() { return 0U; }
        template<typename T>
        T GetRed() const { return static_cast<T>(red); }
        template<typename T>
        T GetGreen() const { return static_cast<T>(green); }
        template<typename T>
        T GetBlue() const { return static_cast<T>(blue); }
        template<typename T>
        T GetAlpha() const { return static_cast<T>(alpha); }
    private:
        COMPONENT alpha, red, green, blue;
    };

} // namespace cuppa
