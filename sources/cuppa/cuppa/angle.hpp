#pragma once

#include <math.h>

#include <type_traits>

namespace cuppa
{
    struct Angle
    {
        template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
        static constexpr auto FromRadian(T radian) { return Angle{static_cast<float>(radian)}; }
        template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
        static constexpr auto FromDegree(T degree) { return Angle{static_cast<float>(degree)/180.0f*PI}; }
        template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
        static constexpr auto FromTurn(T turn) { return Angle{static_cast<float>(turn)*PI}; }
        constexpr auto operator-() const { return Angle{-radian}; }
        constexpr auto operator+=(Angle arc) { radian += arc.radian; }
        constexpr float ToRadian() const { return radian; }
        constexpr float ToDegree() const { return radian/PI*180.0f; }
        constexpr float ToTurn() const { return radian/PI; }
        constexpr auto operator+(Angle rhs) const { return Angle{radian + rhs.radian}; }
        constexpr auto operator-(Angle rhs) const { return Angle{radian - rhs.radian}; }
        template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
        constexpr auto operator*(T ratio) const { return Angle{radian * static_cast<float>(ratio)}; }
        template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
        constexpr auto operator/(T ratio) const { return Angle{radian / static_cast<float>(ratio)}; }
        template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
        friend constexpr auto operator*(T ratio, Angle angle) { return angle * static_cast<float>(ratio); }
        constexpr auto operator<(Angle rhs) const { return radian < rhs.radian; }
        friend auto cos(Angle angle) { return cosf(angle.radian); }
        friend auto sin(Angle angle) { return sinf(angle.radian); }
        friend auto tan(Angle angle) { return tanf(angle.radian); }
    private:
        static constexpr float PI = 3.1415926535897932384626433832795f;

        explicit constexpr Angle(float _radian) : radian{_radian} {}
        float radian;
    };
} // namespace cuppa

inline constexpr auto operator"" _rad(unsigned long long int radian) { return cuppa::Angle::FromRadian(radian); }
inline constexpr auto operator"" _rad(long double radian) { return cuppa::Angle::FromRadian(radian); }
inline constexpr auto operator"" _deg(unsigned long long int degree) { return cuppa::Angle::FromDegree(degree); }
inline constexpr auto operator"" _deg(long double degree) { return cuppa::Angle::FromDegree(degree); }
inline constexpr auto operator"" _turn(unsigned long long int turn) { return cuppa::Angle::FromTurn(turn); }
inline constexpr auto operator"" _turn(long double turn) { return cuppa::Angle::FromTurn(turn); }