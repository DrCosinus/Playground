#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "extern_libs/catch.hpp"

#include "strong_type.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <tuple>

template<typename T, std::size_t N>
std::ostream& operator <<(std::ostream& _os, const std::array<T, N>& _value)
{
    _os << "{ ";
    if constexpr (N>1)
    {
        _os << _value[0];
        for (std::size_t i = 1; i < N; ++i)
        {
            _os << ", " << _value[i];
        }
    }
    _os << " }";
    return _os;
}

struct print_tuple
{
    print_tuple(std::ostream& _os) : os_{_os} {}
    template<typename T, typename... Ts>
    void operator()(const T& _value0, const Ts&... _values)
    { 
        os_ << "{ " << _value0;
        (..., [this](const auto& _value)
        {
            os_ << ", " << _value;
        }(_values));
        os_ << " }";
    }
    std::ostream& os_;
};

template<typename... Ts>
std::ostream& operator <<(std::ostream& _os, const std::tuple<Ts...>& _value)
{
    std::apply(print_tuple{_os}, _value);
    return _os;
}

template<typename VALUE_TYPE, typename DERIVED_TYPE, typename... FLAG_TYPES>
struct testable_strong_type : wit::strong_type<VALUE_TYPE, DERIVED_TYPE, FLAG_TYPES...>
{
    using wit::strong_type<VALUE_TYPE, DERIVED_TYPE, FLAG_TYPES...>::strong_type;

    friend std::ostream& operator <<(std::ostream& _os, const testable_strong_type& _value)
    {
        _os << _value.get_value();
        return _os;
    }
};

namespace test_minimal
{
    struct minimal : testable_strong_type<unsigned long long, minimal>
    {
        using testable_strong_type::testable_strong_type;
        using testable_strong_type::get_value; // for test purpose
    };

    TEST_CASE( "Minimal strong type", "[strong type]")
    {
        minimal foo{ 1234 };
        REQUIRE( foo.get_value() == 1234 );
    }
} // namespace test_minimal

namespace test_comparable
{
    struct foo : testable_strong_type<unsigned long long, foo, wit::comparable>
    {
        using testable_strong_type::testable_strong_type;
    };

    TEST_CASE( "Equatable/Comparable", "[strong type]")
    {
        foo a{12}, b{12}, c{42};

        REQUIRE( a==b );
        REQUIRE_FALSE( a==c );
        REQUIRE( b!=c );
        REQUIRE_FALSE( b!=a );

        REQUIRE_FALSE( a<b );
        REQUIRE_FALSE( a>b );
        REQUIRE( a<=b );
        REQUIRE( a>=b );

        REQUIRE( a<c );
        REQUIRE_FALSE( a>c );
        REQUIRE( a<=c );
        REQUIRE_FALSE( a>=c );

        REQUIRE_FALSE( c<a );
        REQUIRE( c>a );
        REQUIRE_FALSE( c<=a );
        REQUIRE( c>=a );

        //REQUIRE( std::max(a,b,c) == foo{42});
        //REQUIRE( std::min(a,b,c) == foo{12});
    }
} // namespace test_comparable

namespace test_self_arithmetic
{
    struct scalar : testable_strong_type<int, scalar, wit::signable, wit::self_addable, wit::self_subtractable, wit::self_multipliable, wit::self_dividable, wit::incrementable, wit::decrementable, wit::equalable>
    {
        using testable_strong_type::testable_strong_type;
    };

    scalar operator""_z(unsigned long long _value) { return scalar{_value}; }

    TEST_CASE( "self arithmetic", "[strong type]" )
    {
        REQUIRE( scalar{-59} == -scalar{59} );
        REQUIRE( scalar{3} + scalar{5} == scalar{8} );
        REQUIRE( scalar{5} - scalar{3} == scalar{2} );
        REQUIRE( scalar{5} * scalar{3} == scalar{15} );
        REQUIRE( scalar{63} / scalar{7} == scalar{9} );

        SECTION( "Incrementation operators")
        {
            auto t = scalar{ 37 };
            REQUIRE( ++t == scalar{ 38 } );
            REQUIRE( t++ == scalar{ 38 } );
            REQUIRE( t == scalar{ 39 } );
            t += scalar{ 17 };
            REQUIRE( t == scalar{ 56 } );
            (++((t += scalar{ 3 }) += scalar{ 5 }))++;
            REQUIRE( t == scalar{ 66 } );
        }
    }

    TEST_CASE( "Check for the type of the returned value by some operators", "[strong type]" )
    {
        REQUIRE(std::is_same_v<scalar, std::decay_t<decltype(std::declval<scalar>()++)>>);
        REQUIRE(std::is_same_v<scalar, std::decay_t<decltype(++std::declval<scalar>())>>);
        REQUIRE(std::is_same_v<scalar, std::decay_t<decltype(std::declval<scalar>()+=std::declval<scalar>())>>);

        REQUIRE(std::is_same_v<scalar, std::decay_t<decltype(std::declval<scalar>()--)>>);
        REQUIRE(std::is_same_v<scalar, std::decay_t<decltype(--std::declval<scalar>())>>);
        REQUIRE(std::is_same_v<scalar, std::decay_t<decltype(std::declval<scalar>()-=std::declval<scalar>())>>);

        REQUIRE(std::is_same_v<scalar, std::decay_t<decltype(std::declval<scalar>()+std::declval<scalar>())>>);
        REQUIRE(std::is_same_v<scalar, std::decay_t<decltype(std::declval<scalar>()-std::declval<scalar>())>>);
        REQUIRE(std::is_same_v<scalar, std::decay_t<decltype(std::declval<scalar>()*std::declval<scalar>())>>);
        REQUIRE(std::is_same_v<scalar, std::decay_t<decltype(std::declval<scalar>()/std::declval<scalar>())>>);
    }
} // namespace test_self_arithmetic

namespace test_stringable
{
    struct stringable : testable_strong_type<unsigned long long, stringable, wit::stringable>
    {
        using testable_strong_type::testable_strong_type;
    };

    TEST_CASE( "Stringable strong type", "[strong type]")
    {
        stringable foo{ 42 };
        REQUIRE( foo.to_string() == "42" );
        REQUIRE( std::to_string(foo) == "42" );
    }
} // namespace test_stringable

namespace test_convertible
{
    struct convertible : testable_strong_type<unsigned long long, convertible, wit::convertible_to_value>
    {
        using testable_strong_type::testable_strong_type;
    };

    TEST_CASE( "Convertible to value strong type", "[strong type]")
    {
        convertible foo{ 59 };
        REQUIRE( (int)foo == 59 );
    }
} // namespace test_convertible

namespace test_composition
{
    // safe length unit => value is in meter
    struct meter : testable_strong_type<
        float           // value type
        , meter
        , wit::comparable
        , wit::self_addable, wit::self_subtractable, wit::signable
        , wit::incrementable, wit::decrementable
        >
    {
        using testable_strong_type::testable_strong_type;
    };

    meter operator""_m(long double _meters) { return meter{float(_meters)}; }
    meter operator""_m(unsigned long long _meters) { return meter{float(_meters)}; }

    namespace test_handmade_composition
    {
        // a 3d vector without any semantic
        template<typename T>
        struct vector3
        {
            vector3(T _x, T _y, T _z) : x{std::move(_x)}, y{std::move(_y)}, z{std::move(_z)} {}
            bool operator==(const vector3& _rhs) const { return x == _rhs.x && y == _rhs.y && z == _rhs.z; }
            bool operator!=(const vector3& _rhs) const { return !(*this == _rhs); }
            vector3 operator+(const vector3& _rhs) const { return { x + _rhs.x, y + _rhs.y, z + _rhs.z }; }
            T x, y, z;
        };
        // deduction guide (supported by MSVC from 19.14)
        // to be able to use vector3{ 4_m, 5_m, 6_m } instead of vector3<meter>{ 4_m, 5_m, 6_m }
        #if !defined(_MSC_VER) || _MSC_VER >= 1914
        template<typename T> vector3(T _x, T _y, T _z) -> vector3<T>;
        using vec3 = vector3<meter>;
        #else
        template<typename T> using vec3 = vector3<T>;
        #endif

        TEST_CASE( "Strong type: vector3<meter>", "[strong_type]" )
        {
            auto v0 = vec3{ 1_m, 2_m, 3_m };
            auto v1 = vec3{ 1_m, 2_m, 3_m };
            auto v2 = vec3{ 4_m, 5_m, 6_m };

            REQUIRE(v0==v1);
            REQUIRE(v1!=v2);
            REQUIRE(v1+v2 == vec3{ 5_m, 7_m, 9_m } );
        }
    }

    // strong type aggregate tentative
    namespace test_strongly_typed_tuple
    {
        template<typename T>
        struct vector3 : testable_strong_type<std::tuple<T, T, T>, vector3<T>, wit::equalable, wit::self_addable, wit::self_subtractable>
        {
            using testable_strong_type<std::tuple<T, T, T>, vector3<T>, wit::equalable, wit::self_addable, wit::self_subtractable>::testable_strong_type;
        };

        TEST_CASE( "Strongly typed tuple", "[strong_type]" )
        {
            REQUIRE(sizeof(testable_strong_type<float, struct plop, wit::equalable, wit::self_addable, wit::self_subtractable>)==sizeof(float));
            REQUIRE(sizeof(meter)==sizeof(float));

            using vec3 = vector3<meter>;

            REQUIRE(sizeof(vec3)==sizeof(meter[3]));

            auto v0 = vec3{ 1_m, 2_m, 3_m };
            auto v1 = vec3{ 1_m, 2_m, 3_m };
            auto v2 = vec3{ 3_m, 5_m, 7_m };

            REQUIRE( v0==v1 );
            REQUIRE( v1!=v2 );
            REQUIRE( v1+v2 == vec3{ 4_m, 7_m, 10_m });
            REQUIRE( v2-v1 == vec3{ 2_m, 3_m, 4_m });
        }
    } // namespace test_strongly_typed_tuple

    namespace test_strongly_typed_array
    {
        template<typename T>
        struct vector3 : testable_strong_type<std::array<T, 3>, vector3<T>, wit::equalable, wit::self_addable, wit::self_subtractable>
        {
            using testable_strong_type<std::array<T, 3>, vector3<T>, wit::equalable, wit::self_addable, wit::self_subtractable>::testable_strong_type;
        };

        TEST_CASE( "Strongly typed array", "[strong_type]" )
        {
            using vec3 = vector3<meter>;

            REQUIRE(sizeof(vec3)==sizeof(meter[3]));

            auto v0 = vec3{ 1_m, 2_m, 3_m };
            auto v1 = vec3{ 1_m, 2_m, 3_m };
            auto v2 = vec3{ 3_m, 5_m, 7_m };

            REQUIRE( v0==v1 );
            REQUIRE( v1!=v2 );
            REQUIRE( v1+v2 == vec3{ 4_m, 7_m, 10_m });
            REQUIRE( v2-v1 == vec3{ 2_m, 3_m, 4_m });
        }
    } // namespace test_strongly_typed_array
} // namespace test_composition
