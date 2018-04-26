#include <type_traits>
#include <utility>

namespace wit
{
    namespace detail
    {
        // modifier predicated MUST:
        // - be templated by the UNDERLYING_TYPE and have a static constant boolean member variable called value and templated by the MODIFIER_TYPE
        template<typename UNDERLYING_TYPE>
        struct can_check_equality
        {
        private:
            template<typename U> static int Test(...);
            template<typename U> static void Test(std::enable_if_t<std::is_same_v<decltype(U::equals(std::declval<UNDERLYING_TYPE>(), std::declval<UNDERLYING_TYPE>())), bool>>*);
        public:
            template<typename MODIFIER_TYPE>
            static constexpr bool value = std::is_void_v<decltype(Test<MODIFIER_TYPE>(nullptr))>;
        };

        template<typename UNDERLYING_TYPE>
        struct can_check_order
        {
        private:
            template<typename U> static int Test(...);
            template<typename U> static void Test(std::enable_if_t<std::is_same_v<decltype(U::less(std::declval<UNDERLYING_TYPE>(), std::declval<UNDERLYING_TYPE>())), bool>>*);
        public:
            template<typename MODIFIER_TYPE>
            static constexpr bool value = std::is_void_v<decltype(Test<MODIFIER_TYPE>(nullptr))>;
        };

        template<typename UNDERLYING_TYPE>
        struct can_explicitly_convert_to
        {
        private:
            template<typename U> static int Test(...);
            template<typename U> static void Test(decltype(U::template can_explicitly_convertible_to<UNDERLYING_TYPE>)*);
        public:
            template<typename MODIFIER_TYPE>
            static constexpr bool value = std::is_void_v<decltype(Test<MODIFIER_TYPE>(nullptr))>;
        };

        template<typename CONDITION_TYPE, typename... OTHER_TYPES>
        struct find_if;

        template<typename CONDITION_TYPE, typename... OTHER_TYPES>
        using find_if_t = typename find_if<CONDITION_TYPE, OTHER_TYPES...>::type;

        template<typename CONDITION_TYPE, typename FIRST_TYPE, typename... OTHER_TYPES>
        struct find_if<CONDITION_TYPE, FIRST_TYPE, OTHER_TYPES...>
        {
            using type = typename std::conditional<CONDITION_TYPE::template value<FIRST_TYPE>, FIRST_TYPE, typename find_if<CONDITION_TYPE, OTHER_TYPES...>::type >::type;
        };

        template<typename CONDITION_TYPE>
        struct find_if<CONDITION_TYPE>
        {
            using type = void;
        };
    } // namespace detail

    // -------------------------------------

    // the tag type is in the modifier types
    template<typename UNDERLYING_TYPE, typename... MODIFIER_TYPES>
    struct strong_type
    {
        constexpr strong_type(UNDERLYING_TYPE _value) : value_(std::move(_value)) {} // implicit construction from UNDERLYING_TYPE
        template<typename ANOTHER_UNDERLYING_TYPE, typename ANOTHER_TAG_TYPE>
        strong_type(const strong_type<ANOTHER_UNDERLYING_TYPE, ANOTHER_TAG_TYPE>&) = delete; // conversion from another strong_type

        template<typename U>
        using equality_type = detail::find_if_t<detail::can_check_equality<U>, MODIFIER_TYPES...>;
        template<typename U>
        using ordering_type = detail::find_if_t<detail::can_check_order<U>, MODIFIER_TYPES...>;
        template<typename U>
        using explicit_convertible_to = detail::find_if_t<detail::can_explicitly_convert_to<U>, MODIFIER_TYPES...>;

    // comparison
    // comparison/equality
        template<typename T = UNDERLYING_TYPE>
        constexpr std::enable_if_t<!std::is_void_v<equality_type<T>>, bool> operator==(const strong_type& _rhs) const
        {
            return equality_type<T>::equals(value_, _rhs.value_);
        }
        template<typename T = UNDERLYING_TYPE>
        constexpr std::enable_if_t<!std::is_void_v<equality_type<T>>, bool> operator!=(const strong_type& _rhs) const
        {
            return !equality_type<T>::equals(value_, _rhs.value_);
        }
    // comparison/ordering
        template<typename T = UNDERLYING_TYPE>
        constexpr std::enable_if_t<!std::is_void_v<ordering_type<T>>,bool>  operator<(const strong_type& _rhs) const
        {
            return ordering_type<T>::less(value_, _rhs.value_);
        }
        template<typename T = UNDERLYING_TYPE>
        constexpr std::enable_if_t<!std::is_void_v<ordering_type<T>>,bool> operator>(const strong_type& _rhs) const
        {
            return ordering_type<T>::less(_rhs.value_, value_);
        }
        template<typename T = UNDERLYING_TYPE>
        constexpr std::enable_if_t<!std::is_void_v<ordering_type<T>>,bool> operator<=(const strong_type& _rhs) const
        {
            return !ordering_type<T>::less(_rhs.value_, value_);
        }
        template<typename T = UNDERLYING_TYPE>
        constexpr std::enable_if_t<!std::is_void_v<ordering_type<T>>,bool> operator>=(const strong_type& _rhs) const
        {
            return !ordering_type<T>::less(value_, _rhs.value_);
        }
    // conversion operator
        template<typename T = UNDERLYING_TYPE> // to enable SFINAE
        constexpr explicit operator std::enable_if_t<!std::is_void_v<explicit_convertible_to<T>>, T>() const { return T{value_}; }
    // value getter (for now, for test purpose only)
        // UNDERLYING_TYPE& get() { return value_; }
        const UNDERLYING_TYPE& get() const { return value_; }
    private:
        UNDERLYING_TYPE value_;
    };
} // namespace wit