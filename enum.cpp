#include <iostream>
#include <utility>

    /*
namespace wit
{
    // same as strong_type.h
    namespace detail
    {
        // check if the type `U` is amongst the set of types `Ts`
        template <typename U, typename... Ts> struct has;
        template <typename U, typename... Ts> struct has<U, U, Ts...> : std::true_type {};
        template <typename U, typename T, typename... Ts> struct has<U, T, Ts...> : has<U, Ts...> {};
        template <typename U> struct has<U> : std::false_type {};
    } // namespace detail

    struct invalid_enum_t
    {
    };

    template<typename... Ts>
    struct enumeration
    {
        // is possible type?
        template<typename T>
        static constexpr bool is_assignable_v = detail::has<T, Ts...>::value || std::is_same_v<T, invalid_enum_t>;

        // template<typename T, typename = std::enable_if_t<is_assignable_v<T>>>
        // static constexpr const T& arg() { return *((const T*)1); }

        template<typename T, typename = std::enable_if_t<is_assignable_v<T>>>
        struct Enumerator_
        {
        };

        template<typename T>
        static constexpr auto Enumerator = Enumerator_<T>{};

        constexpr enumeration()
        : value_{ &ID<invalid_enum_t> }
        {
        }
        constexpr enumeration(const enumeration&) = default;
        constexpr enumeration(enumeration&&) = default;
        template<typename T, typename = std::enable_if_t<is_assignable_v<T>>>
        constexpr enumeration(const Enumerator_<T>&)
        : value_{ &ID<T> }
        {
        }
        enumeration& operator=(invalid_enum_t&)
        {
            // ...
            return *this;
        }

        template<typename T, typename = std::enable_if_t<is_assignable_v<T>>>
        void assign()
        {
            //static_assert(detail::has<T, Ts...>::value, "Value not allowed");
            // ...
            value_ = &ID<T>;
        }

        template<typename T, typename = std::enable_if_t<is_assignable_v<T>>>
        constexpr bool is() const
        {
            return &ID<T> == value_;
        }

        bool operator==(const enumeration& _rhs) const { return value_==_rhs.value_; }
    private:
        template<typename T>
        static constexpr bool ID = false;
        
        const bool* value_;
    };
}*/

struct test_enum
{
    // For now only Value addresses matter
    struct Value final
    {   // neither copyable nor movable
        Value(const Value&)=delete;
        Value(Value&&)=delete;
        Value& operator=(const Value&)=delete;
        Value& operator=(Value&&)=delete;

        bool operator==(const Value& _rhs) const { return this == &_rhs; }
        bool operator!=(const Value& _rhs) const { return !(*this == _rhs); }
    //private:
    //    friend test_enum;
        constexpr Value() = default;
    };

    static constexpr Value ValueA;
    static constexpr Value ValueB;
    static constexpr Value ValueC;

    constexpr test_enum() = default;
    constexpr test_enum(const Value& _value) : value_ptr_{&_value} {} // intentionally implicit
    constexpr bool operator==(const Value& _value) const { return value_ptr_ == &_value; }
    constexpr bool operator!=(const Value& _value) const { return !(*this == _value); }
    constexpr bool operator==(const test_enum& _rhs) const { return value_ptr_==_rhs.value_ptr_; }
    constexpr bool operator!=(const test_enum& _rhs) const { return !(*this == _rhs); }

    template<typename F>
    static void forEach(F f)
    {
        // fake implementation
        f(ValueA);
        f(ValueB);
        f(ValueC);
    }
private:
    const Value* value_ptr_ = nullptr;
};

template<typename T, typename U>
auto CHECK_EQ(const T& e, const U& v)
{
    return (e==v) ? "OK" : "-> FAILURE <-";
}

template<typename T, typename U>
auto CHECK_NE(const T& e, const U& v)
{
    return (e!=v) ? "OK" : "-> FAILURE <-";
}

int main(void)
{
    using namespace std;
    {
        test_enum foo;
        cout << "#01 " << CHECK_NE( foo, test_enum::ValueA ) << endl;
        cout << "    " << CHECK_NE( foo, test_enum::ValueB ) << endl;
        cout << "    " << CHECK_NE( foo, test_enum::ValueC ) << endl;
    }
    {
        test_enum foo{ test_enum::ValueA };
        cout << "#02 " << CHECK_EQ( foo, test_enum::ValueA ) << endl;
        cout << "    " << CHECK_NE( foo, test_enum::ValueB ) << endl;
        cout << "    " << CHECK_NE( foo, test_enum::ValueC ) << endl;
    }
    {
        test_enum foo( test_enum::ValueB );
        cout << "#03 " << CHECK_NE( foo, test_enum::ValueA ) << endl;
        cout << "    " << CHECK_EQ( foo, test_enum::ValueB ) << endl;
        cout << "    " << CHECK_NE( foo, test_enum::ValueC ) << endl;
    }
    {
        test_enum foo = test_enum::ValueC;
        cout << "#04 " << CHECK_NE( foo, test_enum::ValueA ) << endl;
        cout << "    " << CHECK_NE( foo, test_enum::ValueB ) << endl;
        cout << "    " << CHECK_EQ( foo, test_enum::ValueC ) << endl;
    }
    {
        test_enum foo1{ test_enum::ValueA }, foo2{ test_enum::ValueA }, foo3{ test_enum::ValueB };
        cout << "#05 " << CHECK_EQ( foo1, foo2 ) << endl;
        cout << "    " << CHECK_NE( foo2, foo3 ) << endl;
        cout << "    " << CHECK_NE( foo1, foo3 ) << endl;
    }
    {
        test_enum foo;
        foo = test_enum::ValueB;
        cout << "#06 " << CHECK_EQ( foo, test_enum::ValueB ) << endl;
    }
    {
        test_enum foo1, foo2{ test_enum::ValueC };
        foo1 = foo2;
        cout << "#07 " << CHECK_EQ( foo1, foo2 ) << endl;
    }
    {
        const test_enum::Value* values[] = { &test_enum::ValueA, &test_enum::ValueB, &test_enum::ValueC };
        std::size_t index = 0;
        cout << "#08 ";
        test_enum::forEach([&values, &index](const test_enum::Value& _value){ cout << CHECK_EQ(*values[index++], _value) << " "; });
        cout << endl;
    }
}