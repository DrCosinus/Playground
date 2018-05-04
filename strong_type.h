#include <type_traits>
#include <utility>
#include <string>
#include <tuple>

template<typename T>     struct is_tuple :                    std::false_type {};
template<typename... Ts> struct is_tuple<std::tuple<Ts...>> : std::true_type {};
template<typename T> constexpr bool is_tuple_v = is_tuple<T>::value;

template<typename Tuple, std::size_t... Ints>
constexpr Tuple tuple_add_helper(const Tuple& _lhs, const Tuple& _rhs, std::index_sequence<Ints...>)
{
    return std::make_tuple((std::get<Ints>(_lhs) + std::get<Ints>(_rhs))...);
}

template<typename TUPLE, std::enable_if_t<is_tuple_v<TUPLE>, int> = 0>
constexpr TUPLE operator+(const TUPLE& _lhs, const TUPLE& _rhs)
{
    return tuple_add_helper(_lhs, _rhs, std::make_index_sequence<std::tuple_size_v<TUPLE>>{});
}

template<typename Tuple, std::size_t... Ints>
constexpr Tuple tuple_sub_helper(const Tuple& _lhs, const Tuple& _rhs, std::index_sequence<Ints...>)
{
    return std::make_tuple((std::get<Ints>(_lhs) - std::get<Ints>(_rhs))...);
}

template<typename TUPLE, std::enable_if_t<is_tuple_v<TUPLE>, int> = 0>
constexpr TUPLE operator-(const TUPLE& _lhs, const TUPLE& _rhs)
{
    return tuple_sub_helper(_lhs, _rhs, std::make_index_sequence<std::tuple_size_v<TUPLE>>{});
}

namespace wit
{
    template<typename UNDERLYING_TYPE, typename TAG_TYPE, template<typename> class... MODIFIER_TYPES>
    struct strong_type;

    namespace detail
    {
        template<typename STRONG_TYPE, template<typename> class MODIFIER_TYPE> // second type only because of multiple modifiers and avoid crash (CRTP)
        struct modifier;

        template<template<typename> class MODIFIER_TYPE, typename UNDERLYING_TYPE, typename TAG_TYPE, template<typename> class... MODIFIER_TYPES>
        struct modifier<strong_type<UNDERLYING_TYPE, TAG_TYPE, MODIFIER_TYPES...>, MODIFIER_TYPE>
        {
        protected:
            using STRONG_TYPE = strong_type<UNDERLYING_TYPE, TAG_TYPE, MODIFIER_TYPES...>;
            // modifier is friend of strong_type but the children of modifier will not
            auto& get_value() { return strongly_typed_object().get_value(); }
            const auto& get_value() const { return strongly_typed_object().get_value(); }
            static const auto& get_value(const STRONG_TYPE& _object) { return _object.get_value(); }
            static auto& get_value(STRONG_TYPE& _object) { return _object.get_value(); }
            STRONG_TYPE& strongly_typed_object() { return static_cast<STRONG_TYPE&>(*this); }
            const STRONG_TYPE& strongly_typed_object() const { return static_cast<const STRONG_TYPE&>(*this); }
        };
    } // namespace detail

    template<typename UNDERLYING_TYPE, typename TAG_TYPE, template<typename> class... MODIFIER_TYPES>
    struct strong_type : MODIFIER_TYPES<strong_type<UNDERLYING_TYPE, TAG_TYPE, MODIFIER_TYPES...>>...
    {
        using underlying_type = UNDERLYING_TYPE;
        using tag_type = TAG_TYPE;
        explicit constexpr strong_type(UNDERLYING_TYPE _value) : value_(std::move(_value)) {}
        template<typename ANOTHER_UNDERLYING_TYPE, typename ANOTHER_TAG_TYPE>
        strong_type(const strong_type<ANOTHER_UNDERLYING_TYPE, ANOTHER_TAG_TYPE>&) = delete; // conversion from another strong_type
        const UNDERLYING_TYPE& get() const { return value_; }

        template<typename... Ts>
        explicit strong_type(Ts&&... _args) : value_{ std::forward<Ts>(_args)... } {}

        template<bool B> using int_if = std::enable_if_t<B, int>;

        //template<int dummy = 0> static constexpr bool is_equatable_ = false;
        //template<typename U> std::enable_if_t<U::is_equalable, bool> is_equatable_<0> = U::is_equatable;

// convertion
        template<typename U = strong_type, int_if<U::is_explicitly_convertible_to_value> = 0>
        explicit operator underlying_type() const { return value_; }

// comparison
        template<typename U = strong_type, int_if<U::is_equalable> = 0>
        bool operator==(const tag_type& _rhs) const { return value_ == _rhs.value_; } // should eventually consider using !(a<b || b<a) to not use operator ==
        template<typename U = strong_type, int_if<U::is_comparable> = 0>
        bool operator==(const tag_type& _rhs) const { return value_ == _rhs.value_; } // should eventually consider using !(a<b || b<a) to not use operator ==
        template<typename U = strong_type, int_if<U::is_equalable> = 0>
        bool operator!=(const tag_type& _rhs) const { return  !(tag_object() == _rhs); }
        template<typename U = strong_type, int_if<U::is_comparable> = 0>
        bool operator!=(const tag_type& _rhs) const { return  !(tag_object() == _rhs); }
        template<typename U = strong_type, int_if<U::is_comparable> = 0>
        bool operator<(const tag_type& _rhs) const { return  value_ < _rhs.value_; }
        template<typename U = strong_type, int_if<U::is_comparable> = 0>
        bool operator>(const tag_type& _rhs) const { return  _rhs < tag_object(); }
        template<typename U = strong_type, int_if<U::is_comparable> = 0>
        bool operator<=(const tag_type& _rhs) const { return  !(tag_object() > _rhs); }
        template<typename U = strong_type, int_if<U::is_comparable> = 0>
        bool operator>=(const tag_type& _rhs) const { return  !(tag_object() < _rhs); }

// arithmetic
        template<typename U = strong_type, int_if<U::is_self_addable> = 0>
        auto operator+(const tag_type& _rhs) const { return tag_type{ value_ + _rhs.value_ }; }
        template<typename U = strong_type, int_if<U::is_self_substractable> = 0>
        auto operator-(const tag_type& _rhs) const { return tag_type{ value_ - _rhs.value_ }; }
        template<typename U = strong_type, int_if<U::has_unary_sign_operators> = 0>
        auto operator+() const { return tag_type{ value_ }; }
        template<typename U = strong_type, int_if<U::has_unary_sign_operators> = 0>
        auto operator-() const { return tag_type{ -value_ }; }

        template<typename U = strong_type, int_if<U::is_incrementable> = 0>
        decltype(auto) operator++() { ++value_; return tag_object(); }
        template<typename U = strong_type, int_if<U::is_incrementable> = 0>
        auto operator++(int) { auto res = tag_object(); ++value_; return res; }
        template<typename U = strong_type, int_if<U::is_incrementable> = 0>
        decltype(auto) operator+=(const tag_type& _rhs) { value_ += _rhs.value_; return tag_object(); }

        template<typename U = strong_type, int_if<U::is_decrementable> = 0>
        decltype(auto) operator--() { --value_; return tag_object(); }
        template<typename U = strong_type, int_if<U::is_decrementable> = 0>
        auto  operator--(int) { auto res = tag_object(); --value_; return res; }
        template<typename U = strong_type, int_if<U::is_decrementable> = 0>
        decltype(auto) operator-=(const tag_type& _rhs) { value_ -= _rhs.value_; return tag_object(); }

        template<typename U = strong_type, int_if<U::is_stringable> = 0>
        std::string to_string() const { return std::to_string( value_ ); }
    protected:
        template<typename STRONG_TYPE, template<typename> class MODIFIER_TYPE>
        friend struct detail::modifier;
        const UNDERLYING_TYPE& get_value() const { return value_; }
        UNDERLYING_TYPE& get_value() { return value_; }
        tag_type& tag_object() { return static_cast<tag_type&>(*this); }
        const tag_type& tag_object() const { return static_cast<const tag_type&>(*this); }
        UNDERLYING_TYPE value_;
    };

    // operators == and != only (need only operator == on the underlying type)
    template<typename>
    struct equalable
    {
        static constexpr bool is_equalable = true;
    };

    // operators == , != , <, >, <= and >= (need only operator == and < on the underlying type)
    template<typename>
    struct comparable
    {
        static constexpr bool is_comparable = true;
    };

    template<typename>
    struct self_addable
    {
        static constexpr bool is_self_addable = true;
    };

    template<typename>
    struct self_subtractable
    {
        static constexpr bool is_self_substractable = true;
    };

    // template<typename STRONG_TYPE>
    // struct self_multipliable : detail::modifier<STRONG_TYPE, self_multipliable>
    // {
    //     STRONG_TYPE operator-(const STRONG_TYPE& _rhs) const { return STRONG_TYPE{ this->get_value() * this->get_value(_rhs) }; }
    // };

    // template<typename STRONG_TYPE>
    // struct self_dividable : detail::modifier<STRONG_TYPE, self_dividable>
    // {
    //     STRONG_TYPE operator/(const STRONG_TYPE& _rhs) const { return STRONG_TYPE{ this->get_value() / this->get_value(_rhs) }; }
    // };

    template<typename>
    struct incrementable
    {
        static constexpr bool is_incrementable = true;
    };

    template<typename>
    struct decrementable
    {
        static constexpr bool is_decrementable = true;
    };

    template<typename>
    struct stringable
    {
        static constexpr bool is_stringable = true;
    };

    template<typename>
    struct explicitly_convertible_to_value
    {
        static constexpr bool is_explicitly_convertible_to_value = true;
        // template<typename STRONG_TYPE>
        // struct modifier : detail::modifier<STRONG_TYPE, modifier>
        // {
        //     explicit operator TYPE() const
        //     {
        //         return this->get_value();
        //     }
        // };
    };

    template<typename STRONG_TYPE>
    struct unary_sign
    {
        static constexpr bool has_unary_sign_operators = true;
    };
    // template<template<typename, typename, template<typename> class...> MULTIPLICAND_STRONG_TYPE, typename PRODUCT_STRONG_TYPE>
    // struct multipliable_by
    // {
    //     template<typename STRONG_TYPE>
    //     struct modifier : detail::modifier<STRONG_TYPE, modifier>
    //     {
    //         PRODUCT_STRONG_TYPE operator+(const MULTIPLICAND_STRONG_TYPE&) { this->get_value()}
    //     }
    // };

} // namespace wit

namespace std
{
    template<typename UNDERLYING_TYPE, typename TAG_TYPE, template<typename> class... MODIFIER_TYPES, typename = std::enable_if_t<wit::strong_type<UNDERLYING_TYPE, TAG_TYPE, MODIFIER_TYPES...>::is_stringable>>
    string to_string(const wit::strong_type<UNDERLYING_TYPE, TAG_TYPE, MODIFIER_TYPES...>& _strongly_typed_object)
    {
        return _strongly_typed_object.to_string();
    }
}