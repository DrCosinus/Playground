#include "strong_type.h"

#include <iostream>
#include <tuple>

// commutative_addition
// divisible

struct meter;

// safe length unit => underlying value is in meter (should be carved in the code, not in a comment)
struct length_unit : wit::strong_type<
    float           // underlying type
    , length_unit   // unique tag (thank to CRTP, it is the type itself)
    , wit::comparable
    , wit::explicitly_convertible_to<float>::modifier
    , wit::self_addable, wit::self_subtractable, wit::unary_sign
    , wit::stringable
    //, wit::from<meter>::modifier
    >
{
    using strong_type::strong_type;
    length_unit(strong_type _super) : strong_type(std::move(_super)) {}
    length_unit() : strong_type{ 0 } {}
};

length_unit operator""_m(long double _meters) { return length_unit{float(_meters)}; }
length_unit operator""_m(unsigned long long _meters) { return length_unit{float(_meters)}; }

// struct milliseconds;
// struct seconds;
// struct minutes;

struct time_unit : wit::strong_type<
    long long
    , time_unit
    , wit::comparable
    , wit::incrementable
    //, from_to<milliseconds, 1>
    //, from_to<seconds, 1000>
    //, from_to<minutes, 60000>
    >
{
    using strong_type::strong_type;
    time_unit() : strong_type{ 0 } {}
};

// a 3d vector without any semantic
template<typename T>
struct vector3
{
    vector3(T _x, T _y, T _z) : x{std::move(_x)}, y{std::move(_y)}, z{std::move(_z)} {}
    bool operator==(const vector3& _rhs) const { return x == _rhs.x && y == _rhs.y && z == _rhs.z; }
    bool operator!=(const vector3& _rhs) const { return !(*this == _rhs); }
    T x, y, z;
};

namespace experimental
{
    template<typename T>
    struct vector3 : wit::strong_type<std::tuple<T, T, T>, vector3<T>, wit::equalable, wit::self_addable>
    {
        using super = wit::strong_type<std::tuple<T, T, T>, vector3<T>, wit::equalable, wit::self_addable>;
        vector3(T _x, T _y, T _z) : super{ std::make_tuple(std::move(_x), std::move(_y), std::move(_z)) } {}
        vector3() : vector3( T{}, T{}, T{}) {}
        template<typename U, std::enable_if_t<std::is_base_of_v<U,T>,int> =0>
        vector3(U x, U y, U z) : vector3{ T{x}, T{y}, T{z} } {}
    };
} // namespace experimental

using namespace std;

struct test_group
{
    unsigned int test_count = 0;
    unsigned int success_count = 0;

    void Check(bool _condition, const char* _failure_message)
    {
        test_count++; 
        if (!_condition)
        {
            cout << _failure_message << " **FAILED**" << endl;
        }
        else
        {
            success_count++;
        }
    }
};

template<typename T>
test_group test(test_group&& _tg, const T& a, const T& b)
{
    _tg.Check((a.get()==b.get())==(a==b), "equal to");
    _tg.Check((a.get()!=b.get())==(a!=b), "not equal to");
    _tg.Check((a.get()<b.get())==(a<b), "less than");
    _tg.Check((a.get()>b.get())==(a>b), "greater than");
    _tg.Check((a.get()<=b.get())==(a<=b), "less than or equal to");
    _tg.Check((a.get()>=b.get())==(a>=b), "greater than or equal to");
    return _tg;
};

int main()
{
    test_group tg;

    cout << boolalpha;

    tg = test(move(tg), 17_m, 42_m);
    tg = test(move(tg), 23_m, 23_m);
    tg.Check( +5_m == length_unit{ 5 }, "unary plus operator" );
    tg.Check( -5_m == length_unit{ -5 }, "unary minus operator" );

    tg = test(move(tg), time_unit{ 17 }, time_unit{ 42 });
    tg = test(move(tg), time_unit{ 23 }, time_unit{ 23 });
    tg.Check(float{ 5_m }==5, "explicit conversion to");
    tg.Check(4_m + 7_m == 11_m, "self_addable");
    tg.Check(std::to_string( 18_m ) == std::to_string(18.f), "stringable: std::to_string");

    {
        auto t = time_unit{37};
        tg.Check(++t == time_unit{38}, "incrementable: pre increment");
        tg.Check(t++ == time_unit{38} && t == time_unit{39}, "incrementable: post increment");
        t += time_unit{17};
        tg.Check(t == time_unit{56}, "incrementable: addition assignment");
    }

    {
        auto v0 = vector3<length_unit>{ 1_m, 2_m, 3_m };
        auto v1 = vector3<length_unit>{ 1_m, 2_m, 3_m };
        auto v2 = vector3<length_unit>{ 4_m, 5_m, 6_m };
        tg.Check(v0==v1,"vector3 equal operator");
        tg.Check(v1!=v2,"vector3 not equal operator");
    }

    {
        using vec3 = experimental::vector3<length_unit>;
        auto v0 = vec3{ 1_m, 2_m, 3_m };
        auto v1 = vec3{ 1_m, 2_m, 3_m };
        auto v2 = vec3{ 4_m, 5_m, 6_m };
        tg.Check(v0==v1,"experimental::vector3 equal operator");
        tg.Check(v1!=v2,"experimental::vector3 not equal operator");
        tg.Check(v1+v2 == vec3{ 5_m, 7_m, 9_m }, "experimental::vector3 self addition operator");
    }
    cout << tg.success_count << " success over " << tg.test_count << " tests." << endl;
}