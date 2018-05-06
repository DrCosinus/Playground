#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "extern_libs/catch.hpp"

#include "strong_type.h"

#include <array>
#include <tuple>

TEST_CASE( "Tuple helper", "[Tuple helpers]")
{
    auto sum = std::make_tuple(1,2,3) + std::make_tuple(10,20,30);
    REQUIRE(std::get<0>(sum)==11);
    REQUIRE(std::get<1>(sum)==22);
    REQUIRE(std::get<2>(sum)==33);
}

namespace test_minimal
{
    struct minimal : wit::strong_type<unsigned long long, minimal>
    {
        using strong_type::strong_type;
        using strong_type::get_value; // for test purpose
    };

    TEST_CASE( "Minimal strong type", "[strong type]")
    {
        minimal foo{ 1234 };
        REQUIRE( foo.get_value() == 1234 );
    }
} // namespace test_minimal

namespace test_comparable
{
    struct foo : wit::strong_type<unsigned long long, foo, wit::comparable>
    {
        using strong_type::strong_type;
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
    }
} // namespace test_comparable

namespace test_self_arithmetic
{
    struct scalar : wit::strong_type<int, scalar, wit::signable, wit::self_addable, wit::self_subtractable, wit::self_multipliable, wit::self_dividable, wit::incrementable, wit::decrementable, wit::equalable>
    {
        using strong_type::strong_type;
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
    struct stringable : wit::strong_type<unsigned long long, stringable, wit::stringable>
    {
        using strong_type::strong_type;
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
    struct convertible : wit::strong_type<unsigned long long, convertible, wit::convertible_to_value>
    {
        using strong_type::strong_type;
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
    struct meter : wit::strong_type<
        float           // value type
        , meter
        , wit::comparable
        , wit::self_addable, wit::self_subtractable, wit::signable
        , wit::incrementable, wit::decrementable
        >
    {
        using strong_type::strong_type;
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
        #endif
        using vec3 = vector3<meter>;

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
        struct vector3 : wit::strong_type<std::tuple<T, T, T>, vector3<T>, wit::equalable, wit::self_addable, wit::self_subtractable>
        {
            using wit::strong_type<std::tuple<T, T, T>, vector3<T>, wit::equalable, wit::self_addable, wit::self_subtractable>::strong_type;
        };

        TEST_CASE( "Strongly typed tuple", "[strong_type]" )
        {
            using vec3 = vector3<meter>;

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
        struct vector3 : wit::strong_type<std::array<T, 3>, vector3<T>, wit::equalable, wit::self_addable, wit::self_subtractable>
        {
            using wit::strong_type<std::array<T, 3>, vector3<T>, wit::equalable, wit::self_addable, wit::self_subtractable>::strong_type;
        };

        TEST_CASE( "Strongly typed array", "[strong_type]" )
        {
            using vec3 = vector3<meter>;

            auto v0 = vec3{ 1_m, 2_m, 3_m };
            auto v1 = vec3{ 1_m, 2_m, 3_m };
            auto v2 = vec3{ 3_m, 5_m, 7_m };

            REQUIRE( v0==v1 );
            REQUIRE( v1!=v2 );
            //REQUIRE( v1+v2 == vec3{ 4_m, 7_m, 10_m });
            //REQUIRE( v2-v1 == vec3{ 2_m, 3_m, 4_m });
        }
    } // namespace test_strongly_typed_array
} // namespace test_composition
