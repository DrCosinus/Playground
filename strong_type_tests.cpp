#include "strong_type.h"

#include <iostream>

// commutative_addition
// divisible

// a 3d vector without any semantic
// template<typename T>
// struct vector3
// {
//     T x, y, z;
// };

// safe length unit => underlying value is in meter (should be carved in the code, not in a comment)
struct length_unit : wit::strong_type<
    float           // underlying type
    , length_unit   // unique tag (thank to CRTP, it is the type itself)
    , wit::comparable
    , wit::orderable
    , wit::explicitly_convertible_to<float>::modifier
    , wit::self_addable
    //, hashable
    , wit::stringable
    >
{
    struct meter;
    using strong_type::strong_type;
    length_unit() : strong_type{ 0 } {}
};

// struct milliseconds;
// struct seconds;
// struct minutes;

struct time_unit : wit::strong_type<
    long long
    , time_unit
    , wit::comparable
    , wit::orderable
    //, from_to<milliseconds, 1>
    //, from_to<seconds, 1000>
    //, from_to<minutes, 60000>
    //, serializable
    >
{
    using strong_type::strong_type;
    time_unit() : strong_type{ 0 } {}
};
/*
struct position;

struct timespan;

struct velocity;

using displacement = strong_type
    <
    vector3<float>, // underlying type
    struct string_int_tag, // unique tag
    comparable, // displacement can use the underlying type comparison operators
    commutative_addition<position, position>, // displacement + position = position + displacement => give a position
    divisible<timespan, velocity> // displacement divide by timespan gives velocity
    >;
*/
using namespace std;

struct stats
{
    unsigned int test_count = 0;
    unsigned int success_count = 0;

    void Check(bool _condition, const char* _failure_message)
    {
        test_count++; 
        if (!_condition)
        {
            cout << _failure_message << "**ERROR**" << endl;
        }
        else
        {
            success_count++;
        }
    }
};

template<typename T>
stats test(stats _stats, const T& a, const T& b)
{
    #define CHECK(OP) _stats.test_count++; if ((a.get() OP b.get())!=(a OP b)) { cout << a.get() << #OP << b.get() << " returned " << (a OP b) << "**ERROR**" << endl; } else { _stats.success_count++; }
    CHECK(==);
    CHECK(!=);
    CHECK(<);
    CHECK(>);
    CHECK(<=);
    CHECK(>=);
    #undef CHECK
    return _stats;
};

int main()
{
    stats stats;

    cout << boolalpha;

    stats = test(stats, length_unit{ 17 }, length_unit{ 42 });
    stats = test(stats, length_unit{ 23 }, length_unit{ 23 });

    stats = test(stats, time_unit{ 17 }, time_unit{ 42 });
    stats = test(stats, time_unit{ 23 }, time_unit{ 23 });
    stats.Check(float{ length_unit{ 5 } }==5, "float{ length_unit{ 5 } }==5");
    stats.Check(length_unit{ 4 } + length_unit{ 7 }==length_unit{ 11 }, "length_unit{ 4 } + length_unit{ 7 }==length_unit{ 11 }");
    stats.Check(to_string(length_unit{ 18 }) == std::to_string(18.f), "to_string(length_unit{ 18 }) == std::to_string(18)");
    cout << stats.success_count << " success over " << stats.test_count << " tests." << endl;
}