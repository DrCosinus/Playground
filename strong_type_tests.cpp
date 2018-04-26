#include "strong_type.h"

#include <iostream>

struct comparable
{
    template<typename T>
    static bool equals(const T& _lhs, const T& _rhs) { return _lhs == _rhs; }
};

struct orderable
{
    template<typename T>
    static bool less(const T& _lhs, const T& _rhs) { return _lhs < _rhs; }
};

template<typename TYPE>
struct explicitly_convertible_to
{
    template<typename U, typename = void>
    static constexpr bool can_explicitly_convertible_to = false;
    template<typename DUMMY>
    static constexpr bool can_explicitly_convertible_to<TYPE, DUMMY> = true;
};

struct nul
{
};

template<typename...>
struct commutative_addition
{
};

template<typename...>
struct divisible
{
};

// ------------------------

// a 3d vector without any semantic
template<typename T>
struct vector3
{
    T x, y, z;
};

// safe length unit => underlying value is in meter (should be carved in the code, not in a comment)
struct length_unit : wit::strong_type<
    float           // underlying type
    , length_unit   // unique tag (thank to CRTP, it is the type itself)
    , comparable
    , orderable
    , explicitly_convertible_to<float>
    //, hashable
    //, printable
    >
{
    struct meter;
    using strong_type::strong_type;
    length_unit() : strong_type{ 0 } {}
};

struct milliseconds;
struct seconds;
struct minutes;

struct time_unit : wit::strong_type<
    long long
    , struct milliseconds
    , comparable
    , orderable
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

int main()
{
    using namespace std;

    unsigned int test_count = 0;
    unsigned int success_count = 0;

    cout << boolalpha;
    {
        //length_unit v1 = 17, v2 = 42, v3 = 42;

        auto test = [&](const length_unit& a, const length_unit& b)
        {
            #define CHECK(OP) test_count++; if ((a.get() OP b.get())!=(a OP b)) { cout << a.get() << #OP << b.get() << " returned " << (a OP b) << "**ERROR**" << endl; } else { success_count++; }
            CHECK(==);
            CHECK(!=);
            CHECK(<);
            CHECK(>);
            CHECK(<=);
            CHECK(>=);
            #undef CHECK
        };
        test(17, 42);
        test(23, 23);
    }
    {
        auto test = [&](const time_unit& a, const time_unit& b)
        {
            (void)a; (void)b;
            #define CHECK(OP) test_count++; if ((a.get() OP b.get())!=(a OP b)) { cout << a.get() << #OP << b.get() << " returned " << (a OP b) << "**ERROR**" << endl; } else { success_count++; }
            CHECK(==);
            CHECK(!=);
            CHECK(<);
            CHECK(>);
            CHECK(<=);
            CHECK(>=);
            #undef CHECK
        };
        test(17, 42);
        test(23, 23);
    }

    // wit::detail tests
    if constexpr(true)
    {
        #define CHECK(CONDITION) test_count++; if (!CONDITION) { cout << #CONDITION << " **ERROR**" << endl; } else { success_count++; }
        CHECK(wit::detail::can_check_equality<float>::value<comparable>);
        CHECK(!wit::detail::can_check_equality<float>::value<orderable>);
        CHECK(!wit::detail::can_check_equality<float>::value<nul>);

        CHECK(!wit::detail::can_check_order<float>::value<comparable>);
        CHECK(wit::detail::can_check_order<float>::value<orderable>);
        CHECK(!wit::detail::can_check_order<float>::value<nul>);

        CHECK(wit::detail::can_check_equality<long long>::value<comparable>);
        CHECK(!wit::detail::can_check_equality<long long>::value<orderable>);
        CHECK(!wit::detail::can_check_equality<long long>::value<nul>);

        CHECK(!wit::detail::can_check_order<long long>::value<comparable>);
        CHECK(wit::detail::can_check_order<long long>::value<orderable>);
        CHECK(!wit::detail::can_check_order<long long>::value<nul>);
        #undef CHECK
    }

    cout << success_count << " success over " << test_count << " tests." << endl;
}