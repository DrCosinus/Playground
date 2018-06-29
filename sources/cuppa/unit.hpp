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

    struct Angle
    {
        static constexpr auto FromRadian(float radian) { return Angle{radian}; }
        static constexpr auto FromDegree(float degree) { return Angle{degree/180.0f*PI}; }
        static constexpr auto FromTurn(float turn) { return Angle{turn*PI}; }
        constexpr auto operator-() const { return Meter{-radian}; }
        float ToRadian() const { return radian; }
        float ToDegree() const { return radian/PI*180.0f; }
        float ToTurn() const { return radian/PI; }
        auto operator-(Angle rhs) const { return Angle{radian - rhs.radian}; }
    private:
        static constexpr float PI = 3.1415926535897932384626433832795f;

        explicit constexpr Angle(float _radian) : radian{_radian} {}
        float radian;
    };

    struct Color
    {
        using U8 = unsigned char;
        static constexpr auto FromGray(U8 gray) { return Color{ gray / 255.0f, 1.0f }; }
        static constexpr auto FromRGB(U8 red, U8 green, U8 blue) { return Color{ red / 255.0f, green / 255.0f, blue / 255.0f, 1.0f }; }
        static constexpr auto FromRGBA(U8 red, U8 green, U8 blue, U8 alpha) { return Color{ red / 255.0f, green / 255.0f, blue / 255.0f, alpha / 255.0f }; }
        //static constexpr auto FromHSV() { return Color{}; }
        //static constexpr auto FromHSB() { return Color{}; }
        static constexpr unsigned int ToARGB32() { return 0U; }
    private:
        explicit constexpr Color(float _red, float _green, float _blue, float _alpha)
        : alpha{ _alpha }, red{ _red }, green{ _green }, blue{ _blue }
        {}
        explicit constexpr Color(float _gray, float _alpha)
        : Color{ _gray, _gray, _gray, _alpha }
        {}
        [[Maybe_unused]]
        float alpha, red, green, blue;
    };

} // namespace cuppa

constexpr auto operator"" _m(unsigned long long int _meter) { return cuppa::Meter{_meter}; }
constexpr auto operator"" _m(long double _meter) { return cuppa::Meter{_meter}; }

constexpr auto operator"" _rad(unsigned long long int radian) { return cuppa::Angle::FromRadian(radian); }
constexpr auto operator"" _rad(long double radian) { return cuppa::Angle::FromRadian(radian); }
constexpr auto operator"" _deg(unsigned long long int degree) { return cuppa::Angle::FromDegree(degree); }
constexpr auto operator"" _deg(long double degree) { return cuppa::Angle::FromDegree(degree); }
constexpr auto operator"" _turn(unsigned long long int turn) { return cuppa::Angle::FromTurn(turn); }
constexpr auto operator"" _turn(long double turn) { return cuppa::Angle::FromTurn(turn); }