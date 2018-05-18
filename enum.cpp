#include <iostream>
#include <utility>
#include <typeinfo>
#include <tuple>
#include <typeindex>

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
        }
        // all values of the same type are equalivent, identical, equal.
        // all values of any other types are different
        template<typename CRTP>
        struct value
        {
            bool operator==(const CRTP&) const { return true;}
            template<typename T>
            bool operator==(const T&) const { return false; }
            template<typename T>
            bool operator!=(const T& _value) const { return !(*this==_value); }

            friend std::ostream& operator<<(std::ostream& _os, const CRTP&)
            {
                _os << typeid(CRTP).name();
                return _os;
            }
        };

        template<typename CRTP>
        struct model
        {
            struct Invalid : value<Invalid> { };

            model() : value_{ ID<Invalid>() } {}
            template<typename T>
            model(const T&) : value_{ ID<T>() }
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
                static_assert(detail::tuple_search_v<T, typename CRTP::values_type>, "unexpected value");
                return value_==ID<T>(); 
            }
            template<typename T>
            bool operator!=(const T& _value) const
            {
                static_assert(detail::tuple_search_v<T, typename CRTP::values_type>, "unexpected value");
                return !(*this==_value);
            }

            template<typename F>
            static void forEach(F f)
            {
                forEach(f, std::make_index_sequence<std::tuple_size_v<typename CRTP::values_type>>{});
            }

            friend std::ostream& operator<<(std::ostream& _os, const CRTP& _enum)
            {
                _os << _enum.value_->name();
                return _os;
            }
            template<typename T>
            friend std::ostream& operator<<(std::ostream& _os, const T&)
            {
                static_assert(detail::tuple_search_v<T, typename CRTP::values_type>, "unexpected value");
                _os << typeid(T).name();
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

using wit::model;

struct test_enum : model<test_enum>
{
    struct ValueTypeA : wit::value<ValueTypeA> { };
    struct ValueTypeB : wit::value<ValueTypeB> { };
    struct ValueTypeC : wit::value<ValueTypeC> { };
    struct ValueTypeD : wit::value<ValueTypeD> { };

    using values_type = std::tuple<ValueTypeA, ValueTypeB, ValueTypeC, ValueTypeD>;

    static constexpr ValueTypeA ValueA = {};
    static constexpr ValueTypeB ValueB = {};
    static constexpr ValueTypeC ValueC = {};
    static constexpr ValueTypeD ValueD = {};

    using model::model;


};

std::size_t FailureCount = 0;
std::size_t CheckCount = 0;

#define __CHECK_OP(__E,__V,__OP,__FILE,__LINE) \
{ \
    ++CheckCount; \
    if (!(__E __OP __V)) \
    { \
        ++FailureCount; \
        cout << __FILE << '(' << __LINE << "): FAILURE: " << #__E << #__OP << #__V << " aka " << __E << #__OP << __V  << endl; \
    } \
}

#define CHECK_EQ(__E,__V) __CHECK_OP(__E,__V,== ,__FILE__,__LINE__)
#define CHECK_NE(__E,__V) __CHECK_OP(__E,__V,!= ,__FILE__,__LINE__)
#define FAIL() cout << __FILE__ << '(' << __LINE__ << "): FAILURE: always" << endl;

int main(void)
{
    using namespace std;
    {
        test_enum foo;
        CHECK_NE( foo, test_enum::ValueA );
        CHECK_NE( foo, test_enum::ValueB );
        CHECK_NE( foo, test_enum::ValueC );
    }
    {
        test_enum foo{ test_enum::ValueA };
        CHECK_EQ( foo, test_enum::ValueA );
        CHECK_NE( foo, test_enum::ValueB );
        CHECK_NE( foo, test_enum::ValueC );
    }
    {
        test_enum foo( test_enum::ValueB );
        CHECK_NE( foo, test_enum::ValueA );
        CHECK_EQ( foo, test_enum::ValueB );
        CHECK_NE( foo, test_enum::ValueC );
    }
    {
        test_enum foo = test_enum::ValueC;
        CHECK_NE( foo, test_enum::ValueA );
        CHECK_NE( foo, test_enum::ValueB );
        CHECK_EQ( foo, test_enum::ValueC );
    }
    {
        test_enum foo1{ test_enum::ValueA }, foo2{ test_enum::ValueA }, foo3{ test_enum::ValueB };
        CHECK_EQ( foo1, foo2 );
        CHECK_NE( foo2, foo3 );
        CHECK_NE( foo1, foo3 );
    }
    {
        test_enum foo;
        foo = test_enum::ValueB;
        CHECK_EQ( foo, test_enum::ValueB );
    }
    {
        test_enum foo1, foo2{ test_enum::ValueC };
        foo1 = foo2;
        CHECK_EQ( foo1, foo2 );
    }
    {
        std::size_t index = 0;
        test_enum::forEach([&index](const auto& _value)
        {
            //cout << typeid(_value).name() << endl;
            switch(index++)
            {
            case 0: CHECK_EQ(_value, test_enum::ValueA); break;
            case 1: CHECK_EQ(_value, test_enum::ValueB); break;
            case 2: CHECK_EQ(_value, test_enum::ValueC); break;
            case 3: CHECK_EQ(_value, test_enum::ValueD); break;
            default: FAIL(); break;
            }
        });
    }
    // {
    //     CHECK_NE(test_enum::Invalid, test_enum::Invalid);
    //     CHECK_NE(test_enum::Invalid, test_enum::ValueTypeA);
    //     CHECK_NE(test_enum::Invalid, test_enum::ValueTypeB);
    //     CHECK_NE(test_enum::Invalid, test_enum::ValueTypeC);
    //     CHECK_NE(test_enum::Invalid, test_enum::ValueTypeD);
    // }
    cout << test_enum::ValueA << endl;
    cout << test_enum::ValueB << endl;
    cout << test_enum::ValueC << endl;
    cout << test_enum::ValueD << endl;

    cout << "Tests: " << (CheckCount-FailureCount) << "/" << CheckCount << endl;
}