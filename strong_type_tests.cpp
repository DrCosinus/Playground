#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "extern_libs/catch.hpp"

#include "strong_type.h"

// safe length unit => underlying value is in meter (should be carved in the code, not in a comment)
struct length_unit : wit::strong_type<
    float           // underlying type
    , length_unit   // unique tag (thank to CRTP, it is the type itself)
    , wit::comparable
    , wit::explicitly_convertible_to<float>::modifier
    , wit::self_addable, wit::self_subtractable, wit::unary_sign
    , wit::stringable
    , wit::incrementable
    >
{
    using strong_type::strong_type;
    //length_unit(strong_type _super) : strong_type(std::move(_super)) {}
    //length_unit() : strong_type{ 0 } {}
    //using strong_type::operator+;
    //using strong_type::operator-;
    //length_unit operator+() { return *this; }
    //length_unit operator-() { return length_unit{ -value_ }; }
};

length_unit operator""_m(long double _meters) { return length_unit{float(_meters)}; }
length_unit operator""_m(unsigned long long _meters) { return length_unit{float(_meters)}; }

TEST_CASE( "Strong type: length_unit", "[strong_type]" )
{
    SECTION( "Explicit construction from underlying type, unary plus/minus operator and custom literal operators" )
    {
        REQUIRE( length_unit{ 17 } == 17_m);
        REQUIRE( 17_m == length_unit{ 17 });
        REQUIRE( +17_m == length_unit{ 17 });
        REQUIRE( -17_m == -length_unit{ 17 });
        REQUIRE( +17_m == length_unit{ 17 });
        REQUIRE( -509_m == length_unit{ -509 });
    }
    SECTION( "Comparison operators" )
    {
        REQUIRE_FALSE( -17_m == 42_m );
        REQUIRE( -17_m != 42_m );
        REQUIRE( -17_m < 42_m );
        REQUIRE_FALSE( -17_m > 42_m );
        REQUIRE( -17_m <= 42_m );
        REQUIRE_FALSE( -17_m >= 42_m );

        REQUIRE_FALSE( 42_m == -17_m );
        REQUIRE( 42_m != -17_m );
        REQUIRE_FALSE( 42_m < -17_m );
        REQUIRE( 42_m > -17_m );
        REQUIRE_FALSE( 42_m <= -17_m );
        REQUIRE( 42_m >= -17_m );

        REQUIRE( 23_m == 23_m );
        REQUIRE_FALSE( 23_m != 23_m );
        REQUIRE_FALSE( 23_m <  23_m );
        REQUIRE_FALSE( 23_m >  23_m );
        REQUIRE( 23_m <= 23_m );
        REQUIRE( 23_m >= 23_m );
    }
    SECTION( "Explicit convertion to underlying type" )
    {
        REQUIRE( float(5_m) == 5 );
    }
    SECTION( "Arithmetic operators" )
    {
        REQUIRE( 4_m + 7_m == 11_m );
        REQUIRE( 24_m - 11_m == 13_m );
    }
    SECTION( "std::to_string")
    {
        REQUIRE( std::to_string(18_m) == std::to_string(18.f));
    }
    SECTION( "Incrementation operators")
    {
        auto t = 37_m;
        REQUIRE( ++t == 38_m );
        REQUIRE( t++ == 38_m );
        REQUIRE( t == 39_m );
        t += 17_m;
        REQUIRE( t == 56_m );
    }
}

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

TEST_CASE( "Strong type: vector3<length_unit>", "[strong_type]" )
{
    auto v0 = vector3<length_unit>{ 1_m, 2_m, 3_m };
    auto v1 = vector3<length_unit>{ 1_m, 2_m, 3_m };
    auto v2 = vector3<length_unit>{ 4_m, 5_m, 6_m };

    REQUIRE(v0==v1);
    REQUIRE(v1!=v2);
}

// strong type aggregate tentative
namespace experimental
{
    template<typename T>
    struct vector3 : wit::strong_type<std::tuple<T, T, T>, vector3<T>, wit::equalable, wit::self_addable>
    {
        using super = wit::strong_type<std::tuple<T, T, T>, vector3<T>, wit::equalable, wit::self_addable>;
        using wit::strong_type<std::tuple<T, T, T>, vector3<T>, wit::equalable, wit::self_addable>::strong_type;
        vector3(T _x, T _y, T _z) : super{ std::make_tuple(std::move(_x), std::move(_y), std::move(_z)) } {}
        vector3() : vector3( T{}, T{}, T{}) {}
        template<typename U, std::enable_if_t<std::is_base_of_v<U,T>,int> =0>
        vector3(U x, U y, U z) : vector3{ T{x}, T{y}, T{z} } {}
    };
} // namespace experimental

#include <tuple>

TEST_CASE( "Tuple helper", "[Tuple helpers]")
{
    {
        auto sum = std::make_tuple(1,2,3) + std::make_tuple(10,20,30);
        REQUIRE(std::get<0>(sum)==11);
        REQUIRE(std::get<1>(sum)==22);
        REQUIRE(std::get<2>(sum)==33);
    }
    {
        auto x1 = 1_m;
        auto p1 = std::make_tuple( 1_m, 2_m, 3_m);
        auto sum = std::make_tuple(1_m,2_m,3_m) + std::make_tuple(10_m,20_m,30_m);
        REQUIRE(std::get<0>(sum)==11_m);
        REQUIRE(std::get<1>(sum)==22_m);
        REQUIRE(std::get<2>(sum)==33_m);
    }
}

TEST_CASE( "Strong type: experimental::vector3<length_unit>", "[strong_type]" )
{
    using vec3 = experimental::vector3<length_unit>;

    auto v0 = vec3{ 1_m, 2_m, 3_m };
    auto v1 = vec3{ 1_m, 2_m, 3_m };
    auto v2 = vec3{ 4_m, 5_m, 6_m };

    REQUIRE( v0==v1 );
    REQUIRE( v1!=v2 );
    auto v3 = v1+v2;
    REQUIRE( v1+v2 == vec3{ 5_m, 7_m, 9_m });
}
