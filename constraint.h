#include <type_traits>
#include <utility>

// Purpose: Check that an object meets some requirements
namespace experimental
{
    namespace detail
    {
        template<typename T, typename REQUIREMENT_CHECKER, bool isArithmetic, bool isPointer>
        class __Constrained;
        
        // implementation for arithmetic (integral and floating-point) types.
        template<typename T, typename REQUIREMENT_CHECKER>
        class __Constrained<T, REQUIREMENT_CHECKER, true, false>
        {
            using value_type = T;
            value_type value_;

            constexpr void CheckConstruction()
            {
                if constexpr (REQUIREMENT_CHECKER::CheckOnConstruction)
                {
                    REQUIREMENT_CHECKER{}(value_);
                }
            }

            constexpr void CheckModification()
            {
                if constexpr (REQUIREMENT_CHECKER::CheckOnModification)
                {
                    REQUIREMENT_CHECKER{}(value_);
                }
            }
            
            // unconstrained constructor for internal use only
            constexpr __Constrained(value_type _value, bool)
               : value_{ _value }
            {
            }
        public:
            constexpr __Constrained(const __Constrained& _source)
                : value_{ _source.value_ }
            {
                CheckConstruction();
            }

            // intentionnally implicit constructor from value_type
            constexpr __Constrained(value_type _value)
                : value_{ _value }
            {
                CheckConstruction();
            }

            // intentionnally implicit conversion to value_type
            constexpr operator value_type() const
            {
                return value_;
            } 

        // -------------------------------------------------
            // assignment operators
            template<typename RHST>
            constexpr __Constrained& operator=(const RHST& _rhs)
            {
                value_ = _rhs;
                CheckModification();
                return *this;
            }
            
            template<typename RHST>
            constexpr __Constrained& operator+=(const RHST& _rhs)
            {
                value_ += _rhs;
                CheckModification();
                return *this;
            }
            
            template<typename RHST>
            constexpr __Constrained& operator-=(const RHST& _rhs)
            {
                value_ -= _rhs;
                CheckModification();
                return *this;
            }
            
            template<typename RHST>
            constexpr __Constrained& operator*=(const RHST& _rhs)
            {
                value_ *= _rhs;
                CheckModification();
                return *this;
            }
            
            template<typename RHST>
            constexpr __Constrained& operator/=(const RHST& _rhs)
            {
                value_ /= _rhs;
                CheckModification();
                return *this;
            }
            
            template<typename RHST>
            constexpr __Constrained& operator%=(const RHST& _rhs)
            {
                value_ %= _rhs;
                CheckModification();
                return *this;
            }
            
            template<typename RHST>
            constexpr __Constrained& operator&=(const RHST& _rhs)
            {
                value_ &= _rhs;
                CheckModification();
                return *this;
            }
            
            template<typename RHST>
            constexpr __Constrained& operator|=(const RHST& _rhs)
            {
                value_ |= _rhs;
                CheckModification();
                return *this;
            }

            template<typename RHST>
            constexpr __Constrained& operator^=(const RHST& _rhs)
            {
                value_ ^= _rhs;
                CheckModification();
                return *this;
            }

            template<typename RHST>
            constexpr __Constrained& operator<<=(const RHST& _rhs)
            {
                value_ <<= _rhs;
                CheckModification();
                return *this;
            }

            template<typename RHST>
            constexpr __Constrained& operator>>=(const RHST& _rhs)
            {
                value_ >>= _rhs;
                CheckModification();
                return *this;
            }

        // -------------------------------------------------
            // increments/decrements
            constexpr __Constrained operator++(int)
            {
                value_type value = value_;
                value_++;
                CheckModification();
                return { value, false };
            }

            constexpr __Constrained& operator++()
            {
                ++value_;
                CheckModification();
                return *this;
            }

            // post decrement
            constexpr __Constrained operator--(int)
            {
                value_type value = value_;
                value_--;
                CheckModification();
                return { value, false };
            }

            constexpr __Constrained& operator--()
            {
                --value_;
                CheckModification();
                return *this;
            }
        };

        // implementation for pointer types.
        template<typename T, typename REQUIREMENT_CHECKER>
        class __Constrained<T, REQUIREMENT_CHECKER, false, true>
        {
            using value_type = T;
            value_type value_;

            constexpr void CheckConstruction()
            {
                if constexpr (REQUIREMENT_CHECKER::CheckOnConstruction)
                {
                    REQUIREMENT_CHECKER{}(value_);
                }
            }

            constexpr void CheckModification()
            {
                if constexpr (REQUIREMENT_CHECKER::CheckOnModification)
                {
                    REQUIREMENT_CHECKER{}(value_);
                }
            }
        public:
            template <typename U, typename = std::enable_if_t<std::is_convertible_v<U, T>>>
            constexpr __Constrained(U&& _value) : value_{std::forward<U>(_value) }
            {
                CheckConstruction();
            }

            // intentionnally implicit constructor from value_type
            constexpr __Constrained(value_type _value)
                : value_{ _value }
            {
                CheckConstruction();
            }

            constexpr T get() const
            {
                return /*CheckConstraint*/(value_);
            }

            constexpr operator T() const { return get(); }
            constexpr T operator->() const { return get(); }
            constexpr decltype(auto) operator*() const { return *get(); } 

            // intentionnally implicit conversion to value_type
            template <typename U> //, typename = std::enable_if_t<std::is_convertible_v<U, T>>>
            operator U() const
            {
                return reinterpret_cast<U>(value_);
            }
        };

        // implementation for non arithmetic and non pointer types.
        template<typename T, typename REQUIREMENT_CHECKER>
        class __Constrained<T, REQUIREMENT_CHECKER, false, false> : public std::remove_cv_t<std::remove_reference_t<T>>
        {
            using Super = std::remove_cv_t<std::remove_reference_t<T>>;
        public:
            template<typename... PARAMETER_TYPES>
            constexpr __Constrained(PARAMETER_TYPES&&... _arguments)
                : Super{ std::forward<PARAMETER_TYPES>(_arguments)... }
            {
                REQUIREMENT_CHECKER{}(*this);
            }
        };

    } // namespace detail

    struct NullErrorHandling
    {
        template<typename... Ts>
        void operator()(Ts...) const
        {
        }
    };

    struct ConstructionOnlyPolicy
    {
        static constexpr bool CheckOnConstruction = true;
        static constexpr bool CheckOnModification = false;
        static constexpr bool CheckOnGet = false;
    };

    struct AllModificationsPolicy
    {
        static constexpr bool CheckOnConstruction = true;
        static constexpr bool CheckOnModification = true;
        static constexpr bool CheckOnGet = false;
    };

    template<typename T, typename REQUIREMENT_CHECKER>
    using Constrained = detail::__Constrained<T, REQUIREMENT_CHECKER, std::is_arithmetic_v<T>, std::is_pointer_v<T>>;

    template<typename T, T ref_value, typename FAILURE_HANDLER, typename CHECK_POLICY = ConstructionOnlyPolicy>
    struct Less : CHECK_POLICY
    {
        constexpr void operator()(T _value) const { if (!(_value < ref_value)) { FAILURE_HANDLER::raise_failure(); } }
    };

    template<typename T, T ref_value, typename FAILURE_HANDLER, typename CHECK_POLICY = ConstructionOnlyPolicy>
    struct GreaterOrEqual : CHECK_POLICY
    {
        constexpr void operator()(T _value) const { if (!(_value >= ref_value)) { FAILURE_HANDLER::raise_failure(); } }
    };

    template<typename T, T ref_value, typename FAILURE_HANDLER, typename CHECK_POLICY = ConstructionOnlyPolicy>
    struct Greater : CHECK_POLICY
    {
        constexpr void operator()(T _value) const { if (!(_value > ref_value)) { FAILURE_HANDLER::raise_failure(); } }
    };

    template<typename T, T ref_value, typename FAILURE_HANDLER, typename CHECK_POLICY = ConstructionOnlyPolicy>
    struct LessOrEqual : CHECK_POLICY
    {
        constexpr void operator()(T _value) const { if (!(_value <= ref_value)) { FAILURE_HANDLER::raise_failure(); } }
    };

    // Check that the value is the range [ lower_bound, upper_bound )
    template<auto first_bound, decltype(first_bound) second_bound, typename FAILURE_HANDLER, typename CHECK_POLICY = ConstructionOnlyPolicy>
    struct InRange : CHECK_POLICY
    {
        using value_type = decltype(first_bound);
        static constexpr value_type lower_bound = ( first_bound < second_bound ) ? first_bound : second_bound; // included
        static constexpr value_type upper_bound = ( first_bound > second_bound ) ? first_bound : second_bound; // excluded
        constexpr void operator()(value_type _value) const { if (!(_value >= lower_bound && _value <upper_bound)) { FAILURE_HANDLER::raise_failure(); } }
    };
    // template<typename T, T first_bound, T second_bound, typename FAILURE_HANDLER, typename CHECK_POLICY = ConstructionOnlyPolicy>
    // struct InRange : CHECK_POLICY
    // {
    //     static constexpr T lower_bound = ( first_bound < second_bound ) ? first_bound : second_bound; // included
    //     static constexpr T upper_bound = ( first_bound > second_bound ) ? first_bound : second_bound; // excluded
    //     constexpr void operator()(T _value) const { if (!(_value >= lower_bound && _value <upper_bound)) { FAILURE_HANDLER::raise_failure(); } }
    // };

    template<typename FAILURE_HANDLER, typename CHECK_POLICY = ConstructionOnlyPolicy>
    struct NotNull : CHECK_POLICY
    {
        template<typename T/*, std::enable_if_t<std::is_pointer_v<T>>*/>
        constexpr void operator()(T _value) const { if (!(_value != nullptr)) { FAILURE_HANDLER::raise_failure(); } }
    };
} // namespace experimental