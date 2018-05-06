#include <type_traits>
#include <utility>
#include <string>
#include <tuple>
#include <array>

namespace wit
{
    template<typename T>     struct is_std_tuple :                    std::false_type {};
    template<typename... Ts> struct is_std_tuple<std::tuple<Ts...>> : std::true_type {};
    template<typename T> constexpr bool is_std_tuple_v = is_std_tuple<T>::value;

    template<typename TUPLE, std::size_t... INDICES>
    constexpr TUPLE tuple_add_helper(const TUPLE& _lhs, const TUPLE& _rhs, std::index_sequence<INDICES...>)
    {
        return std::make_tuple((std::get<INDICES>(_lhs) + std::get<INDICES>(_rhs))...);
    }

    template<typename TUPLE, std::size_t... INDICES>
    constexpr TUPLE tuple_sub_helper(const TUPLE& _lhs, const TUPLE& _rhs, std::index_sequence<INDICES...>)
    {
        return std::make_tuple((std::get<INDICES>(_lhs) - std::get<INDICES>(_rhs))...);
    }

    template<typename T>                struct is_std_array :                   std::false_type {};
    template<typename T, std::size_t N> struct is_std_array<std::array<T, N>> : std::true_type {};
    template<typename T> constexpr bool is_std_array_v = is_std_array<T>::value;

    template<typename ARRAY, std::size_t... INDICES>
    constexpr ARRAY array_add_helper(const ARRAY& _lhs, const ARRAY& _rhs, std::index_sequence<INDICES...>)
    {
        return { (_lhs[INDICES] + _rhs[INDICES])... };
    }
} // namespace wit

template<typename ARRAY, typename = std::enable_if_t<wit::is_std_array_v<ARRAY>>>
constexpr ARRAY operator+(const ARRAY& _lhs, const ARRAY& _rhs)
{
    return wit::array_add_helper(_lhs, _rhs, std::make_index_sequence<std::tuple_size_v<ARRAY>>{});
}

// lexicographically adds the values in the tuples
template<typename TUPLE, std::enable_if_t<wit::is_std_tuple_v<TUPLE>, int> = 0>
constexpr TUPLE operator+(const TUPLE& _lhs, const TUPLE& _rhs)
{
    return wit::tuple_add_helper(_lhs, _rhs, std::make_index_sequence<std::tuple_size_v<TUPLE>>{});
}

// lexicographically subtracts the values in the tuples
template<typename TUPLE, std::enable_if_t<wit::is_std_tuple_v<TUPLE>, int> = 0>
constexpr TUPLE operator-(const TUPLE& _lhs, const TUPLE& _rhs)
{
    return wit::tuple_sub_helper(_lhs, _rhs, std::make_index_sequence<std::tuple_size_v<TUPLE>>{});
}

namespace wit
{
    struct equalable {}; // operators == and != only (need only operator == on the underlying type)
    struct comparable {}; // operators == , != , <, >, <= and >= (need only operator == and < on the underlying type)
    struct self_addable {};
    struct self_subtractable {};
    struct self_multipliable {};
    struct self_dividable {};
    struct incrementable {};
    struct decrementable {};
    struct stringable {};
    struct convertible_to_value {}; // explicit conversion operator to types in which the value can be converted to
    struct signable {};

    namespace detail
    {
        template <typename U> struct is_equalable : std::conditional_t<std::is_base_of_v<equalable, U>, std::true_type, std::false_type> {};
        template <typename U> struct is_comparable : std::conditional_t<std::is_base_of_v<comparable, U>, std::true_type, std::false_type> {};
        template <typename U> struct is_self_addable : std::conditional_t<std::is_base_of_v<self_addable, U>, std::true_type, std::false_type> {};
        template <typename U> struct is_self_subtractable : std::conditional_t<std::is_base_of_v<self_subtractable, U>, std::true_type, std::false_type> {};
        template <typename U> struct is_self_multipliable : std::conditional_t<std::is_base_of_v<self_multipliable, U>, std::true_type, std::false_type> {};
        template <typename U> struct is_self_dividable : std::conditional_t<std::is_base_of_v<self_dividable, U>, std::true_type, std::false_type> {};
        template <typename U> struct is_incrementable : std::conditional_t<std::is_base_of_v<incrementable, U>, std::true_type, std::false_type> {};
        template <typename U> struct is_decrementable : std::conditional_t<std::is_base_of_v<decrementable, U>, std::true_type, std::false_type> {};
        template <typename U> struct is_stringable : std::conditional_t<std::is_base_of_v<stringable, U>, std::true_type, std::false_type> {};
        template <typename U> struct is_convertible_to_value : std::conditional_t<std::is_base_of_v<convertible_to_value, U>, std::true_type, std::false_type> {};
        template <typename U> struct is_signable : std::conditional_t<std::is_base_of_v<signable, U>, std::true_type, std::false_type> {};
    } // namespace detail

    // DERIVED_TYPE = at first it was a unique tag (thank to CRTP) but now we use it to cast the result of some operations into this type
    template<typename VALUE_TYPE, typename DERIVED_TYPE, typename... MODIFIER_TYPES>
    struct strong_type : MODIFIER_TYPES...
    {
        using value_type = VALUE_TYPE;
        using derived_type = DERIVED_TYPE;

// constructors
        constexpr explicit strong_type(value_type _value) : value_(std::move(_value)) {}
        template<typename U, std::enable_if_t<std::is_convertible_v<U, value_type>, int> = 0>
        constexpr explicit strong_type(U&& _value) : value_(value_type(_value)) {}

        template<typename... Ts>
        constexpr explicit strong_type(Ts&&... _args) : value_{ std::forward<Ts>(_args)... } {}

// helpers
    private:
        template<typename U, template<typename> class... PREDICATES>
        static constexpr bool any_predicate_of() { return (... || PREDICATES<U>::value); }
        template<typename U, template<typename> class... PREDICATES> using check = std::enable_if_t<any_predicate_of<U, PREDICATES...>()>;
    public:

// convertion
        // template<typename U = strong_type, typename = check<U, detail::is_convertible_to_value>>
        // constexpr explicit operator value_type() const { return value_; }
        template<typename T, typename U = strong_type, typename = check<U, detail::is_convertible_to_value>, typename = std::enable_if_t<std::is_convertible_v<value_type, T>>>
        constexpr explicit operator T() const { return T(value_); }

// comparison
        template<typename U = strong_type, typename = check<U, detail::is_equalable, detail::is_comparable>>
        constexpr bool operator==(const derived_type& _rhs) const { return value_ == _rhs.value_; } // should eventually consider using !(a<b || b<a) to not use operator ==
        template<typename U = strong_type, typename = check<U, detail::is_equalable, detail::is_comparable>>
        constexpr bool operator!=(const derived_type& _rhs) const { return  !(derived_object() == _rhs); }
        template<typename U = strong_type, typename = check<U, detail::is_comparable>>
        constexpr bool operator<(const derived_type& _rhs) const { return  value_ < _rhs.value_; }
        template<typename U = strong_type, typename = check<U, detail::is_comparable>>
        constexpr bool operator>(const derived_type& _rhs) const { return  _rhs < derived_object(); }
        template<typename U = strong_type, typename = check<U, detail::is_comparable>>
        constexpr bool operator<=(const derived_type& _rhs) const { return  !(derived_object() > _rhs); }
        template<typename U = strong_type, typename = check<U, detail::is_comparable>>
        constexpr bool operator>=(const derived_type& _rhs) const { return  !(derived_object() < _rhs); }

// arithmetic
        template<typename U = strong_type, typename = check<U, detail::is_self_addable>>
        constexpr auto operator+(const derived_type& _rhs) const { return derived_type{ value_ + _rhs.value_ }; }
        template<typename U = strong_type, typename = check<U, detail::is_self_subtractable>>
        constexpr auto operator-(const derived_type& _rhs) const { return derived_type{ value_ - _rhs.value_ }; }
        template<typename U = strong_type, typename = check<U, detail::is_signable>>
        constexpr auto operator+() const { return derived_object(); }
        template<typename U = strong_type, typename = check<U, detail::is_signable>>
        constexpr auto operator-() const { return derived_type{ -value_ }; }

        template<typename U = strong_type, typename = check<U, detail::is_incrementable>>
        constexpr decltype(auto) operator++() { ++value_; return derived_object(); }
        template<typename U = strong_type, typename = check<U, detail::is_incrementable>>
        constexpr auto operator++(int) { auto res = derived_object(); ++value_; return res; }
        template<typename U = strong_type, typename = check<U, detail::is_incrementable>>
        constexpr decltype(auto) operator+=(const derived_type& _rhs) { value_ += _rhs.value_; return derived_object(); }

        template<typename U = strong_type, typename = check<U, detail::is_decrementable>>
        constexpr decltype(auto) operator--() { --value_; return derived_object(); }
        template<typename U = strong_type, typename = check<U, detail::is_decrementable>>
        constexpr auto  operator--(int) { auto res = derived_object(); --value_; return res; }
        template<typename U = strong_type, typename = check<U, detail::is_decrementable>>
        constexpr decltype(auto) operator-=(const derived_type& _rhs) { value_ -= _rhs.value_; return derived_object(); }

        template<typename U = strong_type, typename = check<U, detail::is_self_multipliable>>
        constexpr auto operator*(const derived_type& _rhs) const { return derived_type{ value_ * _rhs.value_ }; }

        template<typename U = strong_type, typename = check<U, detail::is_self_dividable>>
        constexpr auto operator/(const derived_type& _rhs) const { return derived_type{ value_ / _rhs.value_ }; }


// to_string
        template<typename U = strong_type, typename = check<U, detail::is_stringable>>
        std::string to_string() const { return std::to_string( value_ ); }

    protected:
        constexpr derived_type& derived_object() { return static_cast<derived_type&>(*this); }
        constexpr const derived_type& derived_object() const { return static_cast<const derived_type&>(*this); }
        constexpr const value_type& get_value() const { return value_; }
    private:
        value_type value_;
    };
} // namespace wit

namespace std
{
    template<typename U, std::enable_if_t<wit::detail::is_stringable<U>::value, int> = 0>
    string to_string(const U& _strongly_typed_object)
    {
        return _strongly_typed_object.to_string();
    }
}