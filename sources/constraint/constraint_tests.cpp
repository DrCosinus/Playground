#include "simple_tests.hpp"

#include <iostream>
#include <type_traits>

#include "constraint.h"

struct FailureHandler
{
    static void raise_failure() { ++failure_count; }
    static bool did_any_failure_raised() { return failure_count != 0; }
    static void reset() { failure_count = 0; }
private:
    inline static std::size_t failure_count = 0;
};

// enable requirement checking during production, disable them for retail (zero overload)
template<typename T, typename REQUIREMENT_CHECKER>
using CheckArg = std::conditional_t<DEBUG, experimental::Constrained< T, REQUIREMENT_CHECKER>, T >;

struct Vector3
{
    float x, y, z;

    auto ComputeSquredLength() const { return x * x + y * y + z * z; }
};

template<typename STREAM_TYPE>
STREAM_TYPE& operator<<(STREAM_TYPE& _stream, const Vector3& _vector)
{
    _stream << "vector3{ " << _vector.x << ", " << _vector.y << ", " << _vector.z << " }";
    return _stream;
}

// for test purpose only
bool almost_equal(float _lhs, float _rhs, float _epsilon = 0.0001f)
{
    return std::abs(_lhs-_rhs) < _epsilon;
}

template<typename FAILURE_HANDLER = FailureHandler>
struct IsNormal
{
    constexpr void operator()(const Vector3& _value) const { if (!(almost_equal(_value.ComputeSquredLength(), 1.f))) { FAILURE_HANDLER::raise_failure(); } }
};

int main()
{
    Vector3 v_normal{ 1.f, 0.f, 0.f };
    Vector3 v_not_normal{ 1.f, 1.f, 0.f };

    using namespace std;
    using namespace experimental;

    using IntegerBetween6and8 = Constrained<int, InRange< 6, 8, FailureHandler>>;
    using NotNullPointer = Constrained<void*, NotNull<FailureHandler>>;
    using NormalVector = Constrained<const Vector3&, IsNormal<FailureHandler>>;
    using Int_GE_42_ConstructionOnly = Constrained<int, GreaterOrEqual<int, 42, FailureHandler, ConstructionOnlyPolicy>>;
    using Int_GE_42_AnyModification = Constrained<int, GreaterOrEqual<int, 42, FailureHandler, AllModificationsPolicy>>;

    {
        FailureHandler::reset();
        CHECK_TRUE(IntegerBetween6and8{ 6 });
        CHECK_TRUE(!FailureHandler::did_any_failure_raised());
    }
    {
        FailureHandler::reset();
        NotNullPointer{ &v_normal };
        CHECK_TRUE(!FailureHandler::did_any_failure_raised());
    }
    {
        FailureHandler::reset();
        NormalVector{ v_normal };
        CHECK_TRUE(!FailureHandler::did_any_failure_raised());
    }
    {
        FailureHandler::reset();
        Int_GE_42_ConstructionOnly i = 42;
        i--;
        CHECK_EQ( i, 41 );
        CHECK_TRUE(!FailureHandler::did_any_failure_raised());
    }

    {
        FailureHandler::reset();
        IntegerBetween6and8{ 5 };
        CHECK_TRUE(FailureHandler::did_any_failure_raised());
    }
    {
        FailureHandler::reset();
        NotNullPointer{ nullptr };
        CHECK_TRUE(FailureHandler::did_any_failure_raised());
    }
    {
        FailureHandler::reset();
        NormalVector{ v_not_normal };
        CHECK_TRUE(FailureHandler::did_any_failure_raised());
    }
    {
        FailureHandler::reset();
        Int_GE_42_AnyModification i = 42;
        i--;
        CHECK_TRUE(FailureHandler::did_any_failure_raised());
    }

    tdd::PrintTestResults([](const char* line){ std::cout << line << std::endl; } );
    return 0;
}