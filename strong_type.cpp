#include <memory>
#include <type_traits>
#include <utility>

namespace detail
{
    template<typename TYPE>
    struct can_check_equality
    {
    private:
        template<typename U, bool(*)(float, float)> struct SFINAE {};
        template<typename U> static int Test(...);
        template<typename U> static void Test(SFINAE<U, U::equals>*);
    public:
        static constexpr bool value = std::is_void_v<decltype(Test<TYPE>(nullptr))>;
    };

    template<typename TYPE>
    struct can_check_order
    {
    private:
        template<typename U, bool(*)(float, float)> struct SFINAE {};
        template<typename U> static int Test(...);
        template<typename U> static void Test(SFINAE<U, U::less>*);
    public:
        static constexpr bool value = std::is_void_v<decltype(Test<TYPE>(nullptr))>;
    };

    template<typename TYPE>
    struct can_explicitly_convert_to_underlying_type
    {
    private:
        template<typename U, bool> struct SFINAE {};
        template<typename U> static int Test(...);
        template<typename U> static void Test(SFINAE<U, U::can_explicitly_convertible_to_underlying_type>*);
    public:
        static constexpr bool value = std::is_void_v<decltype(Test<TYPE>(nullptr))>;
    };
}

template<typename TYPE>
using can_check_equality = detail::can_check_equality<TYPE>;

template<typename TYPE>
[[maybe_unused]]constexpr bool can_check_equality_v = can_check_equality<TYPE>::value;

template<typename TYPE>
using can_check_order = detail::can_check_order<TYPE>;

template<typename TYPE>
[[maybe_unused]]constexpr bool can_check_order_v = can_check_order<TYPE>::value;

template<typename TYPE>
using can_explicitly_convert_to_underlying_type = detail::can_explicitly_convert_to_underlying_type<TYPE>;

template<typename TYPE>
[[maybe_unused]]constexpr bool can_explicitly_convert_to_underlying_type_v = can_explicitly_convert_to_underlying_type<TYPE>::value;

// -------------------------------------

template<template<typename> class CONDITION_TYPE, typename... OTHER_TYPES>
struct find_if;

template<template<typename> class CONDITION_TYPE, typename FIRST_TYPE, typename... OTHER_TYPES>
struct find_if<CONDITION_TYPE, FIRST_TYPE, OTHER_TYPES...>
{
    using type = std::conditional_t<CONDITION_TYPE<FIRST_TYPE>::value, FIRST_TYPE, typename find_if<CONDITION_TYPE, OTHER_TYPES...>::type>;
};

template<template<typename> class CONDITION_TYPE>
struct find_if<CONDITION_TYPE>
{
     using type = void;
};

template<template<typename> class CONDITION_TYPE, typename... OTHER_TYPES>
using find_if_t = typename find_if<CONDITION_TYPE, OTHER_TYPES...>::type;

template<typename UNDERLYING_TYPE, typename, typename... MODIFIER_TYPES>
struct strong_type
{
    strong_type(UNDERLYING_TYPE _value) : value_(std::move(_value)) {} // implicit construction from UNDERLYING_TYPE
    template<typename ANOTHER_UNDERLYING_TYPE, typename ANOTHER_TAG_TYPE>
    strong_type(const strong_type<ANOTHER_UNDERLYING_TYPE, ANOTHER_TAG_TYPE>&) = delete; // conversion from another strong_type

    using equality_type = find_if_t<can_check_equality, MODIFIER_TYPES...>;
    using ordering_type = find_if_t<can_check_order, MODIFIER_TYPES...>;
    using explicit_convert_to_utype = find_if_t<can_explicitly_convert_to_underlying_type, MODIFIER_TYPES...>;

// comparison
// comparison/equality
    std::enable_if_t<!std::is_void_v<equality_type>, bool> operator==(const strong_type& _rhs) const
    {
        return equality_type::equals(value_, _rhs.value_);
    }
    std::enable_if_t<!std::is_void_v<equality_type>, bool> operator!=(const strong_type& _rhs) const
    {
        return !equality_type::equals(value_, _rhs.value_);
    }
// comparison/ordering
    std::enable_if_t<!std::is_void_v<ordering_type>, bool> operator<(const strong_type& _rhs) const
    {
        return ordering_type::less(value_, _rhs.value_);
    }
    std::enable_if_t<!std::is_void_v<ordering_type>, bool> operator>(const strong_type& _rhs) const
    {
        return ordering_type::less(_rhs.value_, value_);
    }
    std::enable_if_t<!std::is_void_v<ordering_type>, bool> operator<=(const strong_type& _rhs) const
    {
        return !ordering_type::less(_rhs.value_, value_);
    }
    std::enable_if_t<!std::is_void_v<ordering_type>, bool> operator>=(const strong_type& _rhs) const
    {
        return !ordering_type::less(value_, _rhs.value_);
    }

    explicit operator std::enable_if_t<!std::is_void_v<explicit_convert_to_utype>, UNDERLYING_TYPE>() const { return value_; }
private:
    UNDERLYING_TYPE value_;
};

struct comparable
{
    static bool equals(float _lhs, float _rhs) { return _lhs == _rhs; }
};

struct orderable
{
    static bool less(float _lhs, float _rhs) { return _lhs < _rhs; }
};

struct explicitly_convertible_to_underlying_type
{
    static constexpr bool can_explicitly_convertible_to_underlying_type = true;
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

#include <iostream>

// a 3d vector without any semantic
template<typename T>
struct vector3
{
    T x, y, z;
};

// safe length unit => underlying value is in meter (should be carved in the code, not in a comment)
struct length_unit : strong_type<
    float           // underlying type
    , length_unit   // unique tag (thank to CRTP, it is the type itself)
    , comparable
    , orderable
    , explicitly_convertible_to_underlying_type
    //, hashable
    //, printable
    >
{
    using strong_type::strong_type;
    length_unit() : strong_type{ 0 } {}
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
    length_unit v1 = 17, v2 = 42, v3 = 42;

    cout << boolalpha;
    auto test = [](const length_unit& a, const length_unit& b)
    {
        cout << "a = " << (float)a << ", b = " << (float)b << endl;
        cout << "(a == b) is " << (a == b) << endl;
        cout << "(a != b) is " << (a != b) << endl;
        cout << "(a <  b) is " << (a <  b) << endl;
        cout << "(a >  b) is " << (a >  b) << endl;
        cout << "(a <= b) is " << (a <= b) << endl;
        cout << "(a >= b) is " << (a >= b) << endl;
        cout << "------------" << endl;
    };
    test(v1, v2);
    test(v2, v3);

    if constexpr(false)
    {
        auto Expect_True = [](bool condition, const char* message) { cout << message << (condition ? " -OK":" **ERROR**") << endl; };
        auto Expect_False = [](bool condition, const char* message) { cout << message << (!condition ? " -OK":" **ERROR**") << endl; };

        Expect_True( can_check_equality_v<comparable>, "can_check_equality_v<comparable>" );
        Expect_False( can_check_equality_v<orderable>, "can_check_equality_v<orderable>" );
        Expect_False( can_check_equality_v<nul>, "can_check_equality_v<nul>" );

        Expect_False( can_check_order_v<comparable>, "can_check_order_v<comparable>" );
        Expect_True( can_check_order_v<orderable>, "can_check_order_v<orderable>" );
        Expect_False( can_check_order_v<nul>, "can_check_order_v<nul>" );
    }
}