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
        template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
        friend auto operator*(T ratio, Pixel px) { return px * ratio; }

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

    // PREDEFINED COLORS (https://en.wikipedia.org/wiki/Web_colors#X11_color_names)
    inline constexpr Color White{ 255 };
    inline constexpr Color LightGray{ 211 };
    inline constexpr Color Silver{ 192 };
    inline constexpr Color DarkGray{ 169 };
    inline constexpr Color Gray{ 128 };
    inline constexpr Color DimGray{ 105 };
    inline constexpr Color Black{ 0 };
    // PINK COLORS
    inline constexpr Color Pink{ 255, 192, 203 };
    // RED COLORS
    inline constexpr Color Red{ 255, 0, 0 };
    inline constexpr Color DarkRed{ 139, 0, 0 };
    // ORANGE COLORS
    inline constexpr Color OrangeRed{ 255, 69, 0 };
    inline constexpr Color Coral{ 255, 127, 80 };
    inline constexpr Color DarkOrange{ 255, 140, 0 };
    inline constexpr Color Orange{ 255, 165, 0 };
    // YELLOW COLORS
    inline constexpr Color Yellow{ 255, 255, 0 };
    inline constexpr Color Gold{ 255, 215, 0 };
    // BROWN COLORS
    inline constexpr Color Goldenrod{ 218, 165, 32 };
    inline constexpr Color DarkGoldenrod{ 184, 134, 11 };
    inline constexpr Color SaddleBrown{ 139, 69, 19 };
    inline constexpr Color Sienna{ 160, 82, 45 };
    inline constexpr Color Brown{ 165, 42, 42 };
    // GREEN COLORS
    inline constexpr Color Lime{ 0, 255, 0 };
    inline constexpr Color Green{ 0, 128, 0 };
    inline constexpr Color DarkGreen{ 0, 100, 0 };
    // CYAN COLORS
    inline constexpr Color Cyan{ 0, 255, 255 };
    inline constexpr Color Aqua{ 0, 255, 255 };
    // BLUE COLORS
    inline constexpr Color CornflowerBlue{ 100, 149, 237 };
    inline constexpr Color Blue{ 0, 0, 255 };
    inline constexpr Color MediumBlue{ 0, 0, 205 };
    inline constexpr Color DarkBlue{ 0, 0, 139 };
    inline constexpr Color Navy{ 0, 0, 128 };
    // PURPLE, VIOLET AND MAGENTA COLORS
    inline constexpr Color Magenta{ 255, 0, 255 };
    inline constexpr Color Fuchsia{ 255, 0, 255 };
    inline constexpr Color DarkViolet{ 148, 0, 211 };
    inline constexpr Color Purple{ 128, 0, 128 };
    inline constexpr Color Indigo{ 75, 0, 130 };

} // namespace cuppa
