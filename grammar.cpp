#include "tdd/simple_tests.hpp"

// for tests
#include <iostream>
#include <string>

// for lib
#include <cstddef>
#include <string_view>

// some references
// https://swtch.com/~rsc/regexp/               (Implementing Regular Expressions)
// https://swtch.com/~rsc/regexp/regexp1.html   (Regular Expression Matching Can Be Simple And Fast)

namespace grammar
{
// terminals
    template<char C, char... Cs>
    struct char_among
    {
        template<typename FUNCTOR>
        constexpr bool operator()(const std::string_view& _sview, FUNCTOR yield_return) const
        {
            if constexpr(C=='\0')
            {
                if (_sview.empty())
                {   // special handling for empty view when C is '\0'
                    yield_return( _sview );
                    return true;
                }
            }
            if (_sview.front() == C)
            {
                auto view_copy = _sview;
                view_copy.remove_prefix(1);
                yield_return( view_copy );
                return true;
            }
            if constexpr (sizeof...(Cs)!=0)
                return char_among<Cs...>{}(_sview, yield_return);
            else
                return false;
        }
    };

    using whitespace = char_among<' ', '\t', '\r', '\n', '\0'>;

    struct any_char
    {
        template<typename FUNCTOR>
        constexpr bool operator()(const std::string_view& _sview, FUNCTOR yield_return) const
        {
            auto view_copy = _sview;
            view_copy.remove_prefix(1);
            yield_return( view_copy );
            return true;
        }
    };

    // some classical terminals to be considered
    // - digit
    // - alpha
    // - alphanum
    // - start of line
    // - end of line
    // - start of file
    // - end of file

// compositors
    template<typename HEAD, typename... TAIL>
    struct any_of
    {
        template<typename FUNCTOR>
        constexpr bool operator()(const std::string_view& _sview, FUNCTOR yield_return) const
        {
            if (HEAD{}(_sview, [&yield_return](const auto& _trailing_sview){ return yield_return(_trailing_sview); } ))
            {
                return true;
            }
            else
            {
                if constexpr (sizeof...(TAIL)==0)
                    return false;
                else
                    return any_of<TAIL...>{}(_sview, yield_return);
            }
        }
    };

    template<typename PREDICATE>
    struct is_not
    {
        template<typename FUNCTOR>
        constexpr bool operator()(const std::string_view& _sview, FUNCTOR yield_return) const
        {
            if (PREDICATE{}(_sview, [](const auto&){ return false; }))
            {
                return false;
            }
            else
            {
                auto view_copy = _sview;
                view_copy.remove_prefix(1); // the HARDCODED count 1 should depend on the underlying operators
                yield_return(view_copy);
                return true;
            }
        }
    };

    template<std::size_t N, typename PREDICATE>
    struct at_least
    {
        template<typename FUNCTOR>
        constexpr bool operator()(const std::string_view& _sview, FUNCTOR yield_return) const
        {
            PREDICATE checker{};

            if (!_sview.empty())
            {
                return checker(_sview, [&yield_return](const auto& trailing_view)
                {
                    if constexpr(N <= 1)
                    {
                        yield_return(trailing_view);
                        at_least<0, PREDICATE>{}(trailing_view, yield_return);
                    }
                    else
                    {
                        at_least<N-1, PREDICATE>{}(trailing_view, yield_return);
                    }
                });
            }
            return N == 0;
        }
    };

    template<typename HEAD, typename... TAIL>
    struct sequence // aka concatenate
    {
        template<typename FUNCTOR>
        constexpr bool operator()(const std::string_view& _sview, FUNCTOR yield_return) const
        {
            auto success = false;
            HEAD{}(_sview, [&success, &yield_return](const auto& _trailing_view)
            {
                if constexpr(sizeof...(TAIL) == 0)
                {
                    yield_return(_trailing_view);
                    success = true;
                }
                else
                {
                    success = sequence<TAIL...>{}(_trailing_view, yield_return);
                }
            });
            return success;
        }
    };

    template<typename PREDICATE>
    struct optional // aka one_or_zero
    {
        template<typename FUNCTOR>
        constexpr bool operator()(const std::string_view& _sview, FUNCTOR yield_return) const
        {
            PREDICATE{}(_sview, [&yield_return](const auto& trailing_view)
            {
                yield_return(trailing_view);
            });
            yield_return(_sview);
            return true;
        }
    };

// algorithm
    enum class Verboseness { Silent, Verbose };
    template<typename PATTERN>
    auto search(std::string_view _sview, Verboseness _verboseness = Verboseness::Silent)
    {
        // maybe yield_callback could return YIELD_CONTINUE or YIELD_BREAK
        std::size_t match_count = 0;
        if (_verboseness==Verboseness::Verbose)
        {
            std::cout << "grammar::search in \"" << _sview << "\":" << std::endl;
        }
        auto yield_callback = [&match_count, &_verboseness](const auto& _trailing_view)
        {
            ++match_count;
            if (_verboseness==Verboseness::Verbose)
            {
                (void)_trailing_view;
                // There is a bug if we display the trailing view :-/
                std::cout << "--> (" << _trailing_view.size() << ")\"" << _trailing_view << "\"" << std::endl;
            }
        };

        PATTERN{}(_sview, yield_callback);

        if (_verboseness==Verboseness::Verbose)
        {
            std::cout << "... " << match_count << " " << ( match_count > 1 ?"matches":"match") << std::endl;
        }
        return match_count != 0;
    }
}

int main(void)
{
    using grammar::any_of;
    using grammar::char_among;
    using grammar::whitespace;
    using grammar::is_not;
    using grammar::at_least;
    using grammar::sequence;
    using grammar::optional;
    using grammar::any_char;
    using grammar::Verboseness;

    using grammar::search;

    using gr_ABC =
        sequence<
            at_least<0, any_char>,
            at_least<
                3,
                is_not<
                    any_of<
                        char_among<'E', 'F', 'G'>,
                        char_among<'I', 'J', 'K'>
                    >
                >
            >,
            any_of<char_among<'H'>,whitespace>
        >;

    CHECK_TRUE(search<gr_ABC>("CplusHminus")); // 3 matches: "Cpl" "Cplu" "Cplus"
    CHECK_FALSE(search<gr_ABC>("CJus"));
    CHECK_FALSE(search<gr_ABC>("CpKu"));
    CHECK_TRUE(search<gr_ABC>("Auto"));
    CHECK_TRUE(search<gr_ABC>("Main"));
    CHECK_FALSE(search<gr_ABC>("Enu"));
    CHECK_FALSE(search<gr_ABC>("Kod"));
    CHECK_TRUE(search<gr_ABC>("Enum"));
    CHECK_TRUE(search<gr_ABC>("Goto"));
    CHECK_TRUE(search<gr_ABC>("Kode"));
    CHECK_TRUE(search<gr_ABC>("DCBA"));
    CHECK_FALSE(search<gr_ABC>("D"));
    CHECK_FALSE(search<gr_ABC>(""));

    using gr_path_allowed_character =
        is_not<
            any_of<
                char_among<'\\','/','*','\"',':','<','>','|','?'>,
                whitespace
            >
        >;
    using gr_extension_allowed_character =
        is_not<
            any_of<
                char_among<'\\','/','*','\"',':','<','>','|','?', '.'>,
                whitespace
            >
        >;
    using gr_filename =
        sequence<
            at_least<1, gr_path_allowed_character>,
            optional<
                sequence<
                    char_among<'.'>,
                    at_least<1, gr_extension_allowed_character>
                >
            >,
            whitespace
        >;

    CHECK_TRUE(search<gr_filename>("allo.wed.ext", Verboseness::Verbose)); // OK: dots in filename allowed, extension should be "d"
    CHECK_FALSE(search<gr_filename>("allowed&.-_=.")); // NOT OK: trailing dot not allowed (no extension)
    CHECK_TRUE(search<gr_filename>("allowed.cpp")); // OK
    CHECK_TRUE(search<gr_filename>("allowed..cpp")); // OK
    CHECK_FALSE(search<gr_filename>("notallowed.ext>")); // NOT OK, leading and trailing forbidden characters
    CHECK_TRUE(search<gr_filename>("allowed")); // OK no extension is allowed
    CHECK_FALSE(search<gr_filename>("notallowed.")); // NOT OK: final dot without extension is not allowed
    CHECK_FALSE(search<gr_filename>(".ext")); // NOT OK: extension only not allowed for filenames (OK for folder names)
    CHECK_FALSE(search<gr_filename>("")); // NOT OK: empty
    CHECK_FALSE(search<gr_filename>("<notallowed")); // NOT OK, leading and trailing forbidden characters
    CHECK_FALSE(search<gr_filename>("notallowed>")); // NOT OK, leading and trailing forbidden characters
    CHECK_TRUE(search<gr_filename>("seperated.cpp names")); // OK: matches "seperated"

    tdd::PrintTestResults([](const char* line){ std::cout << line << std::endl; } );
}