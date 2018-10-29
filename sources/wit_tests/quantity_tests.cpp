#include "simple_tests.hpp"

#include "quantity.hpp"

#include <iostream>

using namespace wit;

constexpr auto operator"" _m(long double _value) { return Meter(static_cast<float>(_value)); }
constexpr auto operator"" _m(unsigned long long int _value) { return Meter(static_cast<float>(_value)); }
constexpr auto operator"" _s(long double _value) { return Second(static_cast<float>(_value)); }
constexpr auto operator"" _s(unsigned long long int _value) { return Second(static_cast<float>(_value)); }

int main()
{
    auto length = 7_m;
    auto length2 = length + 4_m;
    auto time = 3_s;

    auto ok = length - length2;

    CHECK_FALSE(IsScalar(ok));
    CHECK_TRUE(IsLength(ok));
    CHECK_FALSE(IsAcceleration(ok));
    CHECK_FALSE(IsForce(ok));

    //auto not_ok = length + time;
    length = length2;

    CHECK_TRUE(IsScalar(length / length));

    CHECK_TRUE(IsAcceleration(length / (time * time)));

    CHECK_EQ((length + length2).value_, 22);

    tdd::PrintTestResults([](const char *line) { std::cout << line << std::endl; });
    return 0;
}