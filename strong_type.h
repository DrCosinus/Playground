#include <type_traits>
#include <utility>
#include <string>

namespace wit
{
    namespace detail
    {
        template<typename STRONG_TYPE, template<typename> class MODIFIER_TYPE > // second type only because of multiple modifiers and avoid crash (CRTP)
        struct modifier
        {
            // modifier is friend of strong_type but the children of modifier will not
            const auto& get_value() const { return strongly_typed_object().get_value(); }
            static const auto& get_value(const STRONG_TYPE& _object) { return _object.get_value(); }
        private:
            STRONG_TYPE& strongly_typed_object() { return static_cast<STRONG_TYPE&>(*this); }
            const STRONG_TYPE& strongly_typed_object() const { return static_cast<const STRONG_TYPE&>(*this); }
        };
    } // namespace detail

    template<typename STRONG_TYPE>
    struct comparable : detail::modifier<STRONG_TYPE, comparable>
    {
        bool operator==(const STRONG_TYPE& _rhs) const { return  this->get_value() == this->get_value(_rhs); }
        bool operator!=(const STRONG_TYPE& _rhs) const { return  !(*this == _rhs); }
    };

    template<typename STRONG_TYPE>
    struct orderable : detail::modifier<STRONG_TYPE, orderable>
    {
        bool operator<(const STRONG_TYPE& _rhs) const { return  this->get_value() < this->get_value(_rhs); }
        bool operator>(const STRONG_TYPE& _rhs) const { return  this->get_value(_rhs) < this->get_value(); }
        bool operator<=(const STRONG_TYPE& _rhs) const { return  !(*this > _rhs); }
        bool operator>=(const STRONG_TYPE& _rhs) const { return  !(*this < _rhs); }
    };

    template<typename STRONG_TYPE>
    struct self_addable : detail::modifier<STRONG_TYPE, self_addable>
    {
        STRONG_TYPE operator+(const STRONG_TYPE& _rhs) const { return STRONG_TYPE{ this->get_value() + this->get_value(_rhs) }; }
    };

    template<typename STRONG_TYPE>
    struct self_subtractable : detail::modifier<STRONG_TYPE, self_subtractable>
    {
        STRONG_TYPE operator-(const STRONG_TYPE& _rhs) const { return STRONG_TYPE{ this->get_value() - this->get_value(_rhs) }; }
    };

    template<typename STRONG_TYPE>
    struct self_multipliable : detail::modifier<STRONG_TYPE, self_multipliable>
    {
        STRONG_TYPE operator-(const STRONG_TYPE& _rhs) const { return STRONG_TYPE{ this->get_value() * this->get_value(_rhs) }; }
    };

    template<typename STRONG_TYPE>
    struct self_dividable : detail::modifier<STRONG_TYPE, self_multipliable>
    {
        STRONG_TYPE operator/(const STRONG_TYPE& _rhs) const { return STRONG_TYPE{ this->get_value() / this->get_value(_rhs) }; }
    };

    template<typename STRONG_TYPE>
    struct stringable : detail::modifier<STRONG_TYPE, stringable>
    {
        std::string str() const { return std::to_string( this->get_value() ); }
        friend std::string to_string(const STRONG_TYPE& _strongly_typed_object) { return std::to_string( detail::modifier<STRONG_TYPE, stringable>::get_value(_strongly_typed_object) ); }
    };


    template<typename TYPE>
    struct explicitly_convertible_to
    {
        template<typename STRONG_TYPE>
        struct modifier : detail::modifier<STRONG_TYPE, modifier>
        {
            explicit operator TYPE() const
            {
                return this->get_value();
            }
        };
    };

    // -------------------------------------

    template<typename UNDERLYING_TYPE, typename TAG_TYPE, template<typename> class... MODIFIER_TYPES>
    struct strong_type : MODIFIER_TYPES<strong_type<UNDERLYING_TYPE, TAG_TYPE, MODIFIER_TYPES...>>...
    {
        using underlying_type = UNDERLYING_TYPE;
        explicit constexpr strong_type(UNDERLYING_TYPE _value) : value_(std::move(_value)) {}
        template<typename ANOTHER_UNDERLYING_TYPE, typename ANOTHER_TAG_TYPE>
        strong_type(const strong_type<ANOTHER_UNDERLYING_TYPE, ANOTHER_TAG_TYPE>&) = delete; // conversion from another strong_type
        const UNDERLYING_TYPE& get() const { return value_; }
    private:
        template<typename STRONG_TYPE, template<typename> class MODIFIER_TYPE >
        friend struct detail::modifier;
        const UNDERLYING_TYPE& get_value() const { return value_; }
        UNDERLYING_TYPE value_;
    };
} // namespace wit