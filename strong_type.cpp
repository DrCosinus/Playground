#include <memory>
#include <type_traits>
#include <utility>

namespace detail
{
    template<typename TYPE, typename DUMMY>
    struct can_check_equality : std::false_type {};
    template<typename XTYPE>
    struct can_check_equality<XTYPE, std::enable_if_t<std::is_same_v<decltype(XTYPE::Equals(std::declval<float>(),std::declval<float>())),bool>>> : std::true_type {};

    template<typename TYPE, typename DUMMY>
    struct can_check_order : std::false_type {};
    template<typename XTYPE>
    struct can_check_order<XTYPE, std::enable_if_t<std::is_same_v<decltype(XTYPE::Less(std::declval<float>(),std::declval<float>())),bool>>> : std::true_type {};
}

template<typename TYPE>
using can_check_equality = detail::can_check_equality<TYPE, void>;

template<typename TYPE>
[[maybe_unused]]constexpr bool can_check_equality_v = can_check_equality<TYPE>::value;

template<typename TYPE>
using can_check_order = detail::can_check_order<TYPE, void>;

template<typename TYPE>
[[maybe_unused]]constexpr bool can_check_order_v = can_check_order<TYPE>::value;

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

// comparison
// comparison/equality
    std::enable_if_t<!std::is_void_v<equality_type>, bool> operator==(const strong_type& _rhs) const
    {
        return equality_type::Equals(value_, _rhs.value_);
    }
    std::enable_if_t<!std::is_void_v<equality_type>, bool> operator!=(const strong_type& _rhs) const
    {
        return !equality_type::Equals(value_, _rhs.value_);
    }
// comparison/ordering
    std::enable_if_t<!std::is_void_v<ordering_type>, bool> operator<(const strong_type& _rhs) const
    {
        return ordering_type::Less(value_, _rhs.value_);
    }
    std::enable_if_t<!std::is_void_v<ordering_type>, bool> operator>(const strong_type& _rhs) const
    {
        return ordering_type::Less(_rhs.value_, value_);
    }
    std::enable_if_t<!std::is_void_v<ordering_type>, bool> operator<=(const strong_type& _rhs) const
    {
        return !ordering_type::Less(_rhs.value_, value_);
    }
    std::enable_if_t<!std::is_void_v<ordering_type>, bool> operator>=(const strong_type& _rhs) const
    {
        return !ordering_type::Less(value_, _rhs.value_);
    }

    //bool operator<(const strong_type& _rhs) const { return value_ < _rhs.value_; }
    explicit operator UNDERLYING_TYPE() const { return value_; }
private:
    UNDERLYING_TYPE value_;
};

struct comparable
{
    static bool Equals(float _lhs, float _rhs) { return _lhs == _rhs; }
    static constexpr const char* name_ = "comparable";
};

struct orderable
{
    static bool Less(float _lhs, float _rhs) { return _lhs < _rhs; }
    static constexpr const char* name_ = "orderable";
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
        // cout << "(a != b) is " << (a != b) << endl;
        // cout << "(a <  b) is " << (a <  b) << endl;
        // cout << "(a >  b) is " << (a >  b) << endl;
        // cout << "(a <= b) is " << (a <= b) << endl;
        // cout << "(a >= b) is " << (a >= b) << endl;
        cout << "------------" << endl;
    };
    test(v1, v2);
    test(v2, v3);
    cout << length_unit::ordering_type::name_ << endl;

    cout << "Check for MSVC" << endl;
    cout << can_check_equality_v<comparable> << (can_check_equality_v<comparable>?" -OK":" **ERROR**") << endl;
    cout << can_check_equality_v<orderable> << (can_check_equality_v<orderable>?" **ERROR**":" -OK") << endl;
    cout << can_check_equality_v<nul> << (can_check_equality_v<nul>?" **ERROR**":" -OK") << endl;

    cout << can_check_order_v<comparable> << (can_check_order_v<comparable>?" **ERROR**":" -OK") << endl;
    cout << can_check_order_v<orderable> << (can_check_order_v<orderable>?" -OK":" **ERROR**") << endl;
    cout << can_check_order_v<nul> << (can_check_order_v<nul>?" **ERROR**":" -OK") << endl;

    cout << can_check_order<comparable>::value << (detail::can_check_order<comparable, void>::value?" **ERROR**":" -OK") << endl;
    cout << can_check_order<orderable>::value << (detail::can_check_order<orderable, void>::value?" -OK":" **ERROR**") << endl;
    cout << can_check_order<nul>::value << (detail::can_check_order<nul, void>::value?" **ERROR**":" -OK") << endl;

    //cout << typeid(decltype(comparable::Less(std::declval<float>(),std::declval<float>()))) << endl;
}