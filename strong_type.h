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

    template<typename T>                struct is_std_array :                   std::false_type {};
    template<typename T, std::size_t N> struct is_std_array<std::array<T, N>> : std::true_type {};
    template<typename T> constexpr bool is_std_array_v = is_std_array<T>::value;
} // namespace wit

namespace wit
{
    struct equalable; // operators == and != only (need only operator == on the value type)
    struct comparable; // operators == , != , <, >, <= and >= (need only operator < on the value type)
    struct self_addable;
    struct self_subtractable;
    struct self_multipliable;
    struct self_dividable;
    struct incrementable;
    struct decrementable;
    struct stringable;
    struct convertible_to_value; // explicit conversion operator to types in which the value can be converted to
    struct signable;

    namespace detail
    {
        // check if the type `U` is amongst the set of types `Ts`
        template <typename U, typename... Ts> struct has;
        template <typename U, typename... Ts> struct has<U, U, Ts...> : std::true_type {};
        template <typename U, typename T, typename... Ts> struct has<U, T, Ts...> : has<U, Ts...> {};
        template <typename U> struct has<U> : std::false_type {};
    } // namespace detail

    // DERIVED_TYPE = at first it was a unique tag (thank to CRTP) but now we use it to cast the result of some operations into this type
    template<typename VALUE_TYPE, typename DERIVED_TYPE, typename... FLAG_TYPES>
    struct strong_type
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
        template<typename U>
        static constexpr bool has_flag = detail::has<U, FLAG_TYPES...>::value;
    private:
        template<typename... FLAG_TYPES>
        static constexpr bool any_flag() { return (... || has_flag<FLAG_TYPES>); }
        template<typename... FLAG_TYPES> using check = std::enable_if_t<any_flag<FLAG_TYPES...>()>;

    // adds/subtracts the values in the tuples/arrays "member to member"
        template<typename TUPLE, std::size_t... INDICES>
        static constexpr TUPLE tuple_add_helper(const TUPLE& _lhs, const TUPLE& _rhs, std::index_sequence<INDICES...>)
        {
            return { (std::get<INDICES>(_lhs) + std::get<INDICES>(_rhs))... };
        }
        template<typename TUPLE, std::size_t... INDICES>
        static constexpr TUPLE tuple_subtract_helper(const TUPLE& _lhs, const TUPLE& _rhs, std::index_sequence<INDICES...>)
        {
            return { (std::get<INDICES>(_lhs) - std::get<INDICES>(_rhs))... };
        }
    public:
    // convertion
        template<typename T, typename U = strong_type, typename = typename U::template check<convertible_to_value>, typename = std::enable_if_t<std::is_convertible_v<value_type, T>>>
        constexpr explicit operator T() const { return T(value_); }

    // comparison
        #if __clang__ // with MSVC, for now it induces an internal compiler error :(
        template<typename U = strong_type, typename = typename U::template check<equalable, comparable>>
        constexpr bool operator==(const U& _rhs) const
        {
            if constexpr (U::template has_flag<equalable>)
                return value_ == _rhs.value_;
            else
                return !(value_ < _rhs.value_ || _rhs.value_ < value_); // using !(a<b || b<a) instead of operator ==
        }
        #else // __clang__
            template<typename U = strong_type, typename = typename U::template check<equalable, comparable>>
            constexpr bool operator==(const U& _rhs) const
            {
                return value_ == _rhs.value_;
            }
        #endif // __clang__
        template<typename U = strong_type, typename = typename U::template check<equalable, comparable>>
        constexpr bool operator!=(const derived_type& _rhs) const { return  !(derived_object() == _rhs); }
        template<typename U = strong_type, typename = typename U::template check<comparable>>
        constexpr bool operator<(const derived_type& _rhs) const { return  value_ < _rhs.value_; }
        template<typename U = strong_type, typename = typename U::template check<comparable>>
        constexpr bool operator>(const derived_type& _rhs) const { return  _rhs < derived_object(); }
        template<typename U = strong_type, typename = typename U::template check<comparable>>
        constexpr bool operator<=(const derived_type& _rhs) const { return  !(derived_object() > _rhs); }
        template<typename U = strong_type, typename = typename U::template check<comparable>>
        constexpr bool operator>=(const derived_type& _rhs) const { return  !(derived_object() < _rhs); }

    // arithmetic
        template<typename U = strong_type, typename = typename U::template check<self_addable>>
        constexpr auto operator+(const derived_type& _rhs) const
        {
            if constexpr (is_std_array_v<typename U::value_type> || is_std_tuple_v<typename U::value_type>)
                return derived_type{ tuple_add_helper( value_, _rhs.value_, std::make_index_sequence<std::tuple_size_v<typename U::value_type>>{}) };
            else
                return derived_type{ value_ + _rhs.value_ };
        }
        template<typename U = strong_type, typename = typename U::template check<self_subtractable>>
        constexpr auto operator-(const derived_type& _rhs) const
        { 
            if constexpr (is_std_array_v<typename U::value_type> || is_std_tuple_v<typename U::value_type>)
                return derived_type{ tuple_subtract_helper( value_, _rhs.value_, std::make_index_sequence<std::tuple_size_v<typename U::value_type>>{}) };
            else
                return derived_type{ value_ - _rhs.value_ };
        }
        template<typename U = strong_type, typename = typename U::template check<signable>>
        constexpr auto operator+() const { return derived_object(); }
        template<typename U = strong_type, typename = typename U::template check<signable>>
        constexpr auto operator-() const { return derived_type{ -value_ }; }

        template<typename U = strong_type, typename = typename U::template check<incrementable>>
        constexpr decltype(auto) operator++() { ++value_; return derived_object(); }
        template<typename U = strong_type, typename = typename U::template check<incrementable>>
        constexpr auto operator++(int) { auto res = derived_object(); ++value_; return res; }
        template<typename U = strong_type, typename = typename U::template check<incrementable>>
        constexpr decltype(auto) operator+=(const derived_type& _rhs) { value_ += _rhs.value_; return derived_object(); }

        template<typename U = strong_type, typename = typename U::template check<decrementable>>
        constexpr decltype(auto) operator--() { --value_; return derived_object(); }
        template<typename U = strong_type, typename = typename U::template check<decrementable>>
        constexpr auto  operator--(int) { auto res = derived_object(); --value_; return res; }
        template<typename U = strong_type, typename = typename U::template check<decrementable>>
        constexpr decltype(auto) operator-=(const derived_type& _rhs) { value_ -= _rhs.value_; return derived_object(); }

        template<typename U = strong_type, typename = typename U::template check<self_multipliable>>
        constexpr auto operator*(const derived_type& _rhs) const { return derived_type{ value_ * _rhs.value_ }; }

        template<typename U = strong_type, typename = typename U::template check<self_dividable>>
        constexpr auto operator/(const derived_type& _rhs) const { return derived_type{ value_ / _rhs.value_ }; }

    // to_string
        template<typename U = strong_type, typename = typename U::template check<stringable>>
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
    template<typename U, typename = enable_if_t<U::template has_flag<wit::stringable>>>
    inline string to_string(const U& _object)
    {
        return _object.to_string();
    }
}