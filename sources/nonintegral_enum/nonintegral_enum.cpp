#include "simple_tests.hpp"
#include "nonintegral_enum.hpp"

#include <iostream>
#include <string>

using wit::nonintegral_enum;

struct test_enum : nonintegral_enum<test_enum>
{
    struct ValueA : value<ValueA> { };
    struct ValueB : value<ValueB> { };
    struct ValueC : value<ValueC> { };
    struct ValueD : value<ValueD> { };

    using values_type = std::tuple<ValueA, ValueB, ValueC, ValueD>;

    using nonintegral_enum::nonintegral_enum;
};

namespace N
{
    struct S
    {
        struct E : nonintegral_enum<E>
        {
            struct V : value<V> {};

            using values_type = std::tuple<V>;

            using nonintegral_enum::nonintegral_enum;
        };
    };
}

#if defined(_MSC_VER)
//#pragma warning (push)
#pragma warning (disable : 4127)
#endif //  defined(_MSC_VER)

int main(void)
{
    {
        CHECK_LE(sizeof(test_enum), sizeof(std::intptr_t));
        CHECK_LE(sizeof(test_enum::Invalid), sizeof(char));
        CHECK_LE(sizeof(test_enum::ValueA), sizeof(char));
    }
    {
        test_enum foo;
        CHECK_EQ( foo, test_enum::Invalid{} );
        CHECK_NE( foo, test_enum::ValueA{} );
        CHECK_NE( foo, test_enum::ValueB{} );
        CHECK_NE( foo, test_enum::ValueC{} );
        CHECK_NE( foo, test_enum::ValueD{} );
    }
    {
        test_enum foo{ test_enum::ValueA{} };
        CHECK_NE( foo, test_enum::Invalid{} );
        CHECK_EQ( foo, test_enum::ValueA{} );
        CHECK_NE( foo, test_enum::ValueB{} );
        CHECK_NE( foo, test_enum::ValueC{} );
    }
    {
        test_enum foo( test_enum::ValueB{} );
        CHECK_EQ( foo, test_enum::ValueB{} );
    }
    {
        test_enum foo = test_enum::ValueC{};
        CHECK_EQ( foo, test_enum::ValueC{} );
    }
    {
        auto foo = test_enum::ValueD{};
        CHECK_EQ( foo, test_enum::ValueD{} );
    }
    {
        test_enum foo1{ test_enum::ValueA{} }, foo2{ test_enum::ValueA{} }, foo3{ test_enum::ValueB{} };
        CHECK_EQ( foo1, foo2 );
        CHECK_NE( foo2, foo3 );
        CHECK_NE( foo1, foo3 );
    }
    {
        test_enum foo;
        foo = test_enum::ValueB{};
        CHECK_EQ( foo, test_enum::ValueB{} );
    }
    {
        test_enum foo1, foo2{ test_enum::ValueC{} };
        foo1 = foo2;
        CHECK_EQ( foo1, foo2 );
    }
    {
        std::size_t index = 0;
        test_enum::forEach([&index](const auto& _value)
        {
            switch(index++)
            {
            case 0: CHECK_EQ(_value, test_enum::ValueA{}); break;
            case 1: CHECK_EQ(_value, test_enum::ValueB{}); break;
            case 2: CHECK_EQ(_value, test_enum::ValueC{}); break;
            case 3: CHECK_EQ(_value, test_enum::ValueD{}); break;
            default: FAIL("Unknow value"); break;
            }
        });
    }
    {
        CHECK_EQ(test_enum::Invalid{}, test_enum::Invalid{});
        CHECK_NE(N::S::E::Invalid{}, test_enum::Invalid{});
        CHECK_NE(test_enum::Invalid{}, test_enum::ValueA{});
        CHECK_EQ(test_enum::ValueA{}, test_enum::ValueA{});
        CHECK_NE(test_enum::Invalid{}, test_enum::ValueB{});
        CHECK_NE(test_enum::Invalid{}, test_enum::ValueC{});
        CHECK_NE(test_enum::Invalid{}, test_enum::ValueD{});
    }
    {
        CHECK_EQ(test_enum::ValueA::c_str(), std::string{"test_enum::ValueA"});
        CHECK_EQ(test_enum::ValueC::c_str(), std::string{"test_enum::ValueC"});
        CHECK_EQ(N::S::E::V::c_str(), std::string{"N::S::E::V"});
        CHECK_EQ(test_enum::Invalid::c_str(), std::string{"Invalid"});
        CHECK_EQ(N::S::E::Invalid::c_str(), std::string{"Invalid"});
    }
    {
        test_enum foo = test_enum::ValueA{};
        CHECK_EQ(foo.c_str(), std::string{"test_enum::ValueA"});
    }

    tdd::PrintTestResults([](const char* line){ std::cout << line << std::endl; } );
}