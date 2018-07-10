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
        float ToRadian() const { return radian; }
        float ToDegree() const { return radian/PI*180.0f; }
        float ToTurn() const { return radian/PI; }
        auto operator-(Angle rhs) const { return Angle{radian - rhs.radian}; }
        auto operator<(Angle rhs) const { return radian < rhs.radian; }
        auto cos() const { return cosf(radian); }
        auto sin() const { return sinf(radian); }
        auto tan() const { return tanf(radian); }
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