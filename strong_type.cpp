#include <memory>
#include <type_traits>
#include <utility>

namespace detail
{
    template<typename UNDERLYING_TYPE>
    struct can_check_equality
    {
    private:
        template<typename U> static int Test(...);
        template<typename U> static void Test(std::enable_if_t<std::is_same_v<decltype(U::equals(std::declval<UNDERLYING_TYPE>(), std::declval<UNDERLYING_TYPE>())), bool>>*);
    public:
        template<typename MODIFIER_TYPE>
        static constexpr bool value = std::is_void_v<decltype(Test<MODIFIER_TYPE>(nullptr))>;
    };

    template<typename UNDERLYING_TYPE>
    struct can_check_order
    {
    private:
        template<typename U> static int Test(...);
        template<typename U> static void Test(std::enable_if_t<std::is_same_v<decltype(U::less(std::declval<UNDERLYING_TYPE>(), std::declval<UNDERLYING_TYPE>())), bool>>*);
    public:
        template<typename MODIFIER_TYPE>
        static constexpr bool value = std::is_void_v<decltype(Test<MODIFIER_TYPE>(nullptr))>;
    };

    template<typename UNDERLYING_TYPE>
    struct can_explicitly_convert_to
    {
    private:
        template<typename U> static int Test(...);
        template<typename U> static void Test(decltype(U::template can_explicitly_convertible_to<UNDERLYING_TYPE>)*);
    public:
        template<typename MODIFIER_TYPE>
        static constexpr bool value = std::is_void_v<decltype(Test<MODIFIER_TYPE>(nullptr))>;
    };
}

// -------------------------------------

template<typename CONDITION_TYPE, typename... OTHER_TYPES>
struct find_if;

template<typename CONDITION_TYPE, typename FIRST_TYPE, typename... OTHER_TYPES>
struct find_if<CONDITION_TYPE, FIRST_TYPE, OTHER_TYPES...>
{
    using type = std::conditional_t<CONDITION_TYPE::template value<FIRST_TYPE>, FIRST_TYPE, typename find_if<CONDITION_TYPE, OTHER_TYPES...>::type>;
};

template<typename CONDITION_TYPE>
struct find_if<CONDITION_TYPE>
{
     using type = void;
};

template<typename CONDITION_TYPE, typename... OTHER_TYPES>
using find_if_t = typename find_if<CONDITION_TYPE, OTHER_TYPES...>::type;

template<typename UNDERLYING_TYPE, typename, typename... MODIFIER_TYPES>
struct strong_type
{
    constexpr strong_type(UNDERLYING_TYPE _value) : value_(std::move(_value)) {} // implicit construction from UNDERLYING_TYPE
    template<typename ANOTHER_UNDERLYING_TYPE, typename ANOTHER_TAG_TYPE>
    strong_type(const strong_type<ANOTHER_UNDERLYING_TYPE, ANOTHER_TAG_TYPE>&) = delete; // conversion from another strong_type

    using equality_type = find_if_t<detail::can_check_equality<UNDERLYING_TYPE>, MODIFIER_TYPES...>;
    using ordering_type = find_if_t<detail::can_check_order<UNDERLYING_TYPE>, MODIFIER_TYPES...>;
    template<typename OTHER_TYPE>
    using explicit_convertible_to = find_if_t<detail::can_explicitly_convert_to<OTHER_TYPE>, MODIFIER_TYPES...>;

// comparison
// comparison/equality
    constexpr std::enable_if_t<!std::is_void_v<equality_type>, bool> operator==(const strong_type& _rhs) const
    {
        return equality_type::equals(value_, _rhs.value_);
    }
    constexpr std::enable_if_t<!std::is_void_v<equality_type>, bool> operator!=(const strong_type& _rhs) const
    {
        return !equality_type::equals(value_, _rhs.value_);
    }
// comparison/ordering
    constexpr std::enable_if_t<!std::is_void_v<ordering_type>, bool> operator<(const strong_type& _rhs) const
    {
        return ordering_type::less(value_, _rhs.value_);
    }
    constexpr std::enable_if_t<!std::is_void_v<ordering_type>, bool> operator>(const strong_type& _rhs) const
    {
        return ordering_type::less(_rhs.value_, value_);
    }
    constexpr std::enable_if_t<!std::is_void_v<ordering_type>, bool> operator<=(const strong_type& _rhs) const
    {
        return !ordering_type::less(_rhs.value_, value_);
    }
    constexpr std::enable_if_t<!std::is_void_v<ordering_type>, bool> operator>=(const strong_type& _rhs) const
    {
        return !ordering_type::less(value_, _rhs.value_);
    }
// conversion operator
    template<typename T = UNDERLYING_TYPE> // to enable SFINAE
    constexpr explicit operator std::enable_if_t<!std::is_void_v<explicit_convertible_to<T>>, T>() const { return T{value_}; }
// value getter
    // UNDERLYING_TYPE& get() { return value_; }
    // const UNDERLYING_TYPE& get() const { return value_; }
private:
    UNDERLYING_TYPE value_;
};

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

struct explicitly_convertible_to_float
{
    template<typename U>
    static constexpr bool can_explicitly_convertible_to = false;
};
template<>
constexpr bool explicitly_convertible_to_float::can_explicitly_convertible_to<float> = true;

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
struct length_unit : strong_type<
    float           // underlying type
    , length_unit   // unique tag (thank to CRTP, it is the type itself)
    , comparable
    , orderable
    , explicitly_convertible_to_float
    //, explicitly_convertible_to<float>
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

struct time_unit : strong_type<
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

#include <iostream>

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
            #define CHECK(OP) test_count++; if ((float(a) OP float(b))!=(a OP b)) { cout << (float)a << #OP << (float)b << " returned " << (a OP b) << "**ERROR**" << endl; } else { success_count++; }
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
        [[maybe_unused]]time_unit t1 = 1;
    }

    if constexpr(true)
    {
        #define CHECK(CONDITION) test_count++; if (!CONDITION) { cout << #CONDITION << " **ERROR**" << endl; } else { success_count++; }
        CHECK(detail::can_check_equality<float>::value<comparable>);
        CHECK(!detail::can_check_equality<float>::value<orderable>);
        CHECK(!detail::can_check_equality<float>::value<nul>);

        CHECK(!detail::can_check_order<float>::value<comparable>);
        CHECK(detail::can_check_order<float>::value<orderable>);
        CHECK(!detail::can_check_order<float>::value<nul>);

        CHECK(detail::can_check_equality<long long>::value<comparable>);
        CHECK(!detail::can_check_equality<long long>::value<orderable>);
        CHECK(!detail::can_check_equality<long long>::value<nul>);

        CHECK(!detail::can_check_order<long long>::value<comparable>);
        CHECK(detail::can_check_order<long long>::value<orderable>);
        CHECK(!detail::can_check_order<long long>::value<nul>);
        #undef CHECK
    }

    cout << success_count << " success over " << test_count << " tests." << endl;
}