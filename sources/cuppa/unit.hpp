#pragma once

namespace cuppa
{
    struct Pixel
    {
        explicit constexpr Pixel(float _value) : value_{_value} {}
        constexpr auto operator-() const { return Pixel{-value_}; }
        template<typename T>
        T getAs() const { return static_cast<T>(value_); }

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

    struct Direction;

    struct Point
    {
        /*implicit*/ constexpr Point(Pixel _x, Pixel _y) : x{_x}, y{_y}  { }

        Point operator+(Direction offset) const;
        Point operator-(Direction offset) const;

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

    enum struct ArcMode{ PIE, OPEN, CHORD };

    struct Angle
    {
        static constexpr auto FromRadian(float radian) { return Angle{radian}; }
        static constexpr auto FromDegree(float degree) { return Angle{degree/180.0f*PI}; }
        static constexpr auto FromTurn(float turn) { return Angle{turn*PI}; }
        constexpr auto operator-() const { return Angle{-radian}; }
        constexpr auto operator+=(Angle arc) { radian += arc.radian; }
        float ToRadian() const { return radian; }
        float ToDegree() const { return radian/PI*180.0f; }
        float ToTurn() const { return radian/PI; }
        auto operator-(Angle rhs) const { return Angle{radian - rhs.radian}; }
        auto operator<(Angle rhs) const { return radian < rhs.radian; }
    private:
        static constexpr float PI = 3.1415926535897932384626433832795f;

        explicit constexpr Angle(float _radian) : radian{_radian} {}
        float radian;
    };

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

constexpr auto operator"" _px(unsigned long long int _pixel) { return cuppa::Pixel{static_cast<float>(_pixel)}; }
constexpr auto operator"" _px(long double _pixel) { return cuppa::Pixel{static_cast<float>(_pixel)}; }

constexpr auto operator"" _rad(unsigned long long int radian) { return cuppa::Angle::FromRadian(static_cast<float>(radian)); }
constexpr auto operator"" _rad(long double radian) { return cuppa::Angle::FromRadian(static_cast<float>(radian)); }
constexpr auto operator"" _deg(unsigned long long int degree) { return cuppa::Angle::FromDegree(static_cast<float>(degree)); }
constexpr auto operator"" _deg(long double degree) { return cuppa::Angle::FromDegree(static_cast<float>(degree)); }
constexpr auto operator"" _turn(unsigned long long int turn) { return cuppa::Angle::FromTurn(static_cast<float>(turn)); }
constexpr auto operator"" _turn(long double turn) { return cuppa::Angle::FromTurn(static_cast<float>(turn)); }