#include <type_traits>
#include <utility>
#include <string>
#include <tuple>

namespace wit
{
    template<typename T>     struct is_tuple :                    std::false_type {};
    template<typename... Ts> struct is_tuple<std::tuple<Ts...>> : std::true_type {};
    template<typename T> constexpr bool is_tuple_v = is_tuple<T>::value;

    template<typename Tuple, std::size_t... Ints>
    constexpr Tuple tuple_add_helper(const Tuple& _lhs, const Tuple& _rhs, std::index_sequence<Ints...>)
    {
        return std::make_tuple((std::get<Ints>(_lhs) + std::get<Ints>(_rhs))...);
    }

    template<typename Tuple, std::size_t... Ints>
    constexpr Tuple tuple_sub_helper(const Tuple& _lhs, const Tuple& _rhs, std::index_sequence<Ints...>)
    {
        return std::make_tuple((std::get<Ints>(_lhs) - std::get<Ints>(_rhs))...);
    }
} // namespace wit

template<typename TUPLE, std::enable_if_t<wit::is_tuple_v<TUPLE>, int> = 0>
constexpr TUPLE operator+(const TUPLE& _lhs, const TUPLE& _rhs)
{
    return wit::tuple_add_helper(_lhs, _rhs, std::make_index_sequence<std::tuple_size_v<TUPLE>>{});
}

template<typename TUPLE, std::enable_if_t<wit::is_tuple_v<TUPLE>, int> = 0>
constexpr TUPLE operator-(const TUPLE& _lhs, const TUPLE& _rhs)
{
    return wit::tuple_sub_helper(_lhs, _rhs, std::make_index_sequence<std::tuple_size_v<TUPLE>>{});
}

namespace wit
{
    template<typename VALUE_TYPE, typename TAG_TYPE, typename... MODIFIER_TYPES>
    struct strong_type : MODIFIER_TYPES...
    {
        using value_type = VALUE_TYPE;
        using tag_type = TAG_TYPE;

// constructors
        constexpr explicit strong_type(value_type _value) : value_(std::move(_value)) {}

        template<typename... Ts>
        constexpr explicit strong_type(Ts&&... _args) : value_{ std::forward<Ts>(_args)... } {}

// helpers
    private:
        template<bool B> using int_if = std::enable_if_t<B, int>;
    public:
        //template<int dummy = 0> static constexpr bool is_equatable_ = false;
        //template<typename U> std::enable_if_t<U::is_equalable, bool> is_equatable_<0> = U::is_equatable;

// convertion
        template<typename U = strong_type, int_if<U::is_explicitly_convertible_to_value> = 0>
        constexpr explicit operator value_type() const { return value_; }

// comparison
        template<typename U = strong_type, int_if<U::is_equalable> = 0>
        constexpr bool operator==(const tag_type& _rhs) const { return value_ == _rhs.value_; } // should eventually consider using !(a<b || b<a) to not use operator ==
        template<typename U = strong_type, int_if<U::is_comparable> = 0>
        constexpr bool operator==(const tag_type& _rhs) const { return value_ == _rhs.value_; } // should eventually consider using !(a<b || b<a) to not use operator ==
        template<typename U = strong_type, int_if<U::is_equalable> = 0>
        constexpr bool operator!=(const tag_type& _rhs) const { return  !(tag_object() == _rhs); }
        template<typename U = strong_type, int_if<U::is_comparable> = 0>
        constexpr bool operator!=(const tag_type& _rhs) const { return  !(tag_object() == _rhs); }
        template<typename U = strong_type, int_if<U::is_comparable> = 0>
        constexpr bool operator<(const tag_type& _rhs) const { return  value_ < _rhs.value_; }
        template<typename U = strong_type, int_if<U::is_comparable> = 0>
        constexpr bool operator>(const tag_type& _rhs) const { return  _rhs < tag_object(); }
        template<typename U = strong_type, int_if<U::is_comparable> = 0>
        constexpr bool operator<=(const tag_type& _rhs) const { return  !(tag_object() > _rhs); }
        template<typename U = strong_type, int_if<U::is_comparable> = 0>
        constexpr bool operator>=(const tag_type& _rhs) const { return  !(tag_object() < _rhs); }

// arithmetic
        template<typename U = strong_type, int_if<U::is_self_addable> = 0>
        constexpr auto operator+(const tag_type& _rhs) const { return tag_type{ value_ + _rhs.value_ }; }
        template<typename U = strong_type, int_if<U::is_self_substractable> = 0>
        constexpr auto operator-(const tag_type& _rhs) const { return tag_type{ value_ - _rhs.value_ }; }
        template<typename U = strong_type, int_if<U::has_unary_sign_operators> = 0>
        constexpr auto operator+() const { return tag_object(); }
        template<typename U = strong_type, int_if<U::has_unary_sign_operators> = 0>
        constexpr auto operator-() const { return tag_type{ -value_ }; }

        template<typename U = strong_type, int_if<U::is_incrementable> = 0>
        constexpr decltype(auto) operator++() { ++value_; return tag_object(); }
        template<typename U = strong_type, int_if<U::is_incrementable> = 0>
        constexpr auto operator++(int) { auto res = tag_object(); ++value_; return res; }
        template<typename U = strong_type, int_if<U::is_incrementable> = 0>
        constexpr decltype(auto) operator+=(const tag_type& _rhs) { value_ += _rhs.value_; return tag_object(); }

        template<typename U = strong_type, int_if<U::is_decrementable> = 0>
        constexpr decltype(auto) operator--() { --value_; return tag_object(); }
        template<typename U = strong_type, int_if<U::is_decrementable> = 0>
        constexpr auto  operator--(int) { auto res = tag_object(); --value_; return res; }
        template<typename U = strong_type, int_if<U::is_decrementable> = 0>
        constexpr decltype(auto) operator-=(const tag_type& _rhs) { value_ -= _rhs.value_; return tag_object(); }

// to_string
        template<typename U = strong_type, int_if<U::is_stringable> = 0>
        std::string to_string() const { return std::to_string( value_ ); }

    protected:
        constexpr tag_type& tag_object() { return static_cast<tag_type&>(*this); }
        constexpr const tag_type& tag_object() const { return static_cast<const tag_type&>(*this); }
        constexpr const value_type& get_value() const { return value_; }
    private:
        value_type value_;
    };

    // operators == and != only (need only operator == on the underlying type)
    struct equalable            { static constexpr bool is_equalable = true; };
    // operators == , != , <, >, <= and >= (need only operator == and < on the underlying type)
    struct comparable           { static constexpr bool is_comparable = true; };
    struct self_addable         { static constexpr bool is_self_addable = true; };
    struct self_subtractable    { static constexpr bool is_self_substractable = true; };
    struct self_multipliable    { static constexpr bool is_self_multipliable = true; }; // no yet implemented
    struct self_dividable       { static constexpr bool is_self_dividable = true; }; // no yet implemented
    struct incrementable        { static constexpr bool is_incrementable = true; };
    struct decrementable        { static constexpr bool is_decrementable = true; };
    struct stringable           { static constexpr bool is_stringable = true; };
    // explicit convertion
    struct convertible_to_value { static constexpr bool is_explicitly_convertible_to_value = true; };
    struct unary_sign           { static constexpr bool has_unary_sign_operators = true; };
} // namespace wit

namespace std
{
    template<typename U, std::enable_if_t<U::is_stringable, int> = 0>
    string to_string(const U& _strongly_typed_object)
    {
        return _strongly_typed_object.to_string();
    }
}