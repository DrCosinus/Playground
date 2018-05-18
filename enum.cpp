#include "tdd/simple_tests.hpp"

#include <iostream>
#include <utility>
#include <typeinfo>
#include <tuple>
#include <string>

namespace wit
{
    inline namespace experimental
    {
        namespace detail
        {
            template<typename TYPE, typename TUPLE>
            struct tuple_search : std::false_type {};

            template<typename TYPE, typename HEAD, typename... TAIL>
            struct tuple_search<TYPE, std::tuple<HEAD, TAIL...>> : std::conditional_t<(sizeof...(TAIL)>0), tuple_search<TYPE, std::tuple<TAIL...>>, std::false_type>
            {};
            template<typename TYPE, typename... TAIL>
            struct tuple_search<TYPE, std::tuple<TYPE, TAIL...>> : std::true_type
            {};
            template<typename TYPE, typename TUPLE>
            inline constexpr bool tuple_search_v = tuple_search<TYPE, TUPLE>::value;

            inline constexpr std::size_t magic_length_of_struct = 7; // 7 is length of "struct "
        }

        template<typename CRTP>
        struct nonintegral_enum
        {
            // all values of the same type are equalivent, identical, equal.
            // all values of any other types are different
            template<typename DERIVED_VALUE_TYPE>
            struct value
            {
                bool operator==(const value&) const { return true;}
                bool operator==(const DERIVED_VALUE_TYPE&) const { return true;}
                template<typename T>
                bool operator==(const T&) const { return false; }
                template<typename T>
                bool operator!=(const T& _value) const { return !(*this==_value); }

                static const char* c_str() { return typeid(DERIVED_VALUE_TYPE).name() + detail::magic_length_of_struct; }

                friend std::ostream& operator<<(std::ostream& _os, const DERIVED_VALUE_TYPE&)
                {
                    _os << c_str();
                    return _os;
                }
            };

            struct Invalid
            {
                bool operator==(const Invalid&) { return true; }
                template<typename T>
                bool operator==(const T&) const { return false; }
                template<typename T>
                bool operator!=(const T& _v) const { return !(*this==_v); }

                static const char* c_str() { return /*(typeid(CRTP).name() + detail::magic_length_of_struct) + "::" + */ "Invalid"; }

                friend std::ostream& operator<<(std::ostream& _os, const Invalid&)
                {
                    _os << c_str();
                    return _os;
                }
            };

            nonintegral_enum() : value_{ ID<Invalid>() } {}
            template<typename T>
            nonintegral_enum(const T&) : value_{ ID<T>() }
            {
                static_assert(detail::tuple_search_v<T, typename CRTP::values_type>, "unexpected value");
            };

            bool operator==(const CRTP& _other) const
            {
                return value_==_other.value_;
            }
            bool operator!=(const CRTP& _other) const
            {
                return !(*this==_other);
            }

            template<typename T>
            bool operator==(const T&) const
            {
                static_assert(detail::tuple_search_v<T, typename CRTP::values_type> || std::is_same_v<T, Invalid>, "unexpected value");
                return value_==ID<T>(); 
            }
            template<typename T>
            bool operator!=(const T& _value) const
            {
                static_assert(detail::tuple_search_v<T, typename CRTP::values_type> || std::is_same_v<T, Invalid>, "unexpected value");
                return !(*this==_value);
            }

            template<typename F>
            static void forEach(F f)
            {
                forEach(f, std::make_index_sequence<std::tuple_size_v<typename CRTP::values_type>>{});
            }

            template<typename HEAD, typename... TAIL>
            const char* c_str_helper(std::tuple<HEAD, TAIL...>&&) const
            {
                if (value_ == &typeid(HEAD))
                {
                    return HEAD::c_str();
                }
                else
                {
                    if constexpr (sizeof...(TAIL)>0)
                        return c_str_helper(std::tuple<TAIL...>{});
                    else
                        return Invalid::c_str();
                }
            }

            const char* c_str() const
            {
                using values_type = typename CRTP::values_type;
                return c_str_helper(values_type{});
            }

            friend std::ostream& operator<<(std::ostream& _os, const CRTP& _enum)
            {
                _os << _enum.value_->name() + detail::magic_length_of_struct;
                return _os;
            }
        protected:
            template<typename T>
            static constexpr auto ID() { return &typeid(T); } // typeid does not return a constant expression with MSVC

            const std::type_info* value_;
        private:
            template<typename F, std::size_t... INDICES>
            static constexpr void forEach(F f, std::index_sequence<INDICES...>)
            {
                ( f(std::tuple_element_t<INDICES, typename CRTP::values_type>{}), ... );
            }
        };
    } // namespace experimental
} // namespace wit

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