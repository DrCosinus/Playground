#include <tuple>
#include <typeinfo>
#include <utility>

namespace wit
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

            template<typename OUTPUT_STREAM_TYPE>
            friend OUTPUT_STREAM_TYPE& operator<<(OUTPUT_STREAM_TYPE& _os, const DERIVED_VALUE_TYPE&)
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

            template<typename OUTPUT_STREAM_TYPE>
            friend OUTPUT_STREAM_TYPE& operator<<(OUTPUT_STREAM_TYPE& _os, const Invalid&)
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
        }

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

        template<typename OUTPUT_STREAM_TYPE>
        friend OUTPUT_STREAM_TYPE& operator<<(OUTPUT_STREAM_TYPE& _os, const CRTP& _enum)
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
} // namespace wit
