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
    // a string_view for search with cursors retrieve string_view of matches
    struct search_view
    {
        using size_type = std::string_view::size_type;
        template<std::size_t N>
        constexpr search_view(const char (&_literal)[N]) : view_{ _literal, N-1 } {}
        constexpr bool empty() const { return match_start_index_+match_lenght_>=view_.size(); }
        constexpr auto front() const { return view_[match_start_index_+match_lenght_]; }
        constexpr void absorb() { ++match_lenght_; }
        constexpr void advance_start() { ++match_start_index_; match_lenght_ = 0; }
        constexpr auto match() const { return view_.substr(match_start_index_, match_lenght_); }

        template<typename OUTPUT_STREAM_TYPE>
        friend OUTPUT_STREAM_TYPE& operator<<(OUTPUT_STREAM_TYPE& _os, search_view _sv)
        {
            _os << _sv.view_;
            return _os;
        }
    private:
        const std::string_view view_;
        size_type match_start_index_ = 0;
        size_type match_lenght_ = 0;
    };

// terminals
    template<char C, char... Cs>
    struct char_among
    {
        template<typename FUNCTOR>
        constexpr bool operator()(const search_view& _sview, FUNCTOR yield_return) const
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
                view_copy.absorb();
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
        constexpr bool operator()(const search_view& _sview, FUNCTOR yield_return) const
        {
            auto view_copy = _sview;
            view_copy.absorb();
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
        constexpr bool operator()(const search_view& _sview, FUNCTOR yield_return) const
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
        constexpr bool operator()(const search_view& _sview, FUNCTOR yield_return) const
        {
            if (PREDICATE{}(_sview, [](const auto&){ return false; }))
            {
                return false;
            }
            else
            {
                auto view_copy = _sview;
                view_copy.absorb();
                yield_return(view_copy);
                return true;
            }
        }
    };

    template<std::size_t N, typename PREDICATE>
    struct at_least
    {
        template<typename FUNCTOR>
        constexpr bool operator()(const search_view& _sview, FUNCTOR yield_return) const
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
        constexpr bool operator()(const search_view& _sview, FUNCTOR yield_return) const
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
        constexpr bool operator()(const search_view& _sview, FUNCTOR yield_return) const
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

    // check for an exact match (nor leading neither trailing characters)
    template<typename PATTERN>
    auto match(search_view _sview, Verboseness _verboseness = Verboseness::Silent)
    {
        // maybe yield_callback could return YIELD_CONTINUE or YIELD_BREAK
        std::size_t match_count = 0;
        if (_verboseness==Verboseness::Verbose)
        {
            std::cout << "grammar::match in \"" << _sview << "\":" << std::endl;
        }
        auto yield_callback = [&match_count, &_verboseness](const auto& _trailing_view)
        {
            if (_trailing_view.empty())
                ++match_count;
            if (_verboseness==Verboseness::Verbose)
            {
                std::cout << "--> \"" << _trailing_view.match() << "\"" << std::endl;
            }
        };

        PATTERN{}(_sview, yield_callback);

        if (_verboseness==Verboseness::Verbose)
        {
            std::cout << "... " << match_count << " " << ( match_count > 1 ?"matches":"match") << std::endl;
        }
        return match_count != 0;
    }

    // search the pattern anywhere in the string
    template<typename PATTERN>
    auto search(search_view _sview, Verboseness _verboseness = Verboseness::Silent)
    {
        if (_verboseness==Verboseness::Verbose)
        {
            std::cout << "grammar::search in \"" << _sview << "\":" << std::endl;
        }
        std::size_t match_count = 0;

        while(!_sview.empty())
        {
            // maybe yield_callback could return YIELD_CONTINUE or YIELD_BREAK
            auto yield_callback = [&match_count, &_verboseness](const auto& _trailing_view)
            {
                ++match_count;
                if (_verboseness==Verboseness::Verbose)
                {
                    std::cout << "--> \"" << _trailing_view.match() << "\"" << std::endl;
                }
            };

            PATTERN{}(_sview, yield_callback);

            _sview.advance_start(); // remove the first character and retry
        }
        if (_verboseness==Verboseness::Verbose)
        {
            std::cout << "... " << match_count << " " << ( match_count > 1 ?"matches":"match") << std::endl;
        }
        return match_count!=0;
    }
}

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
    using grammar::match;

template<std::size_t N>
using gr_perf =
    sequence<
        at_least< N, optional< char_among<'a'> > >
        //, at_least< N, char_among<'a'>>
    >;

int main(void)
{
    // CHECK_TRUE(search<gr_perf<29>>("aaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
    // CHECK_TRUE(search<gr_perf<9>>("aaaaaaaaa"));
    CHECK_TRUE(search<gr_perf<5>>("aaaaa"));
    //CHECK_TRUE(search<gr_perf<2>>("aa"));
    using gr_perf_x =
        sequence<
            optional< char_among<'a'> >
            , optional< char_among<'a'> >
            , optional< char_among<'a'> >
            , optional< char_among<'a'> >
            , optional< char_among<'a'> >
            , char_among<'a'>
            , char_among<'a'>
            , char_among<'a'>
            , char_among<'a'>
            , char_among<'a'>
        >;
    CHECK_TRUE(search<gr_perf_x>("aaaaa", Verboseness::Verbose));

    using gr_blood_group =
        sequence<
            any_of<
                sequence< char_among<'A'>, optional< char_among<'B'> > >,
                char_among< 'B' >,
                char_among< 'O' >
            >,
            any_of<char_among<'+'>,char_among<'-'>>
        >;

    CHECK_TRUE(search<gr_blood_group>("AB+")); // match AB+ or B+
    CHECK_TRUE(search<gr_blood_group>("A+"));
    CHECK_TRUE(search<gr_blood_group>("O- ")); // trailing characters should not interfere the search result
    CHECK_FALSE(match<gr_blood_group>("O- ")); // trailing characters not match exactly the pattern
    CHECK_TRUE(search<gr_blood_group>("AAA+++")); // search should search the pattern anywhere in the string
    CHECK_FALSE(search<gr_blood_group>("BC+"));
    CHECK_TRUE(search<gr_blood_group>("BA+")); // match A+ (leading B is OK with search algorithm)
    CHECK_FALSE(match<gr_blood_group>("BA+"));

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

    CHECK_TRUE(search<gr_filename>("allo.wed.ext")); // OK: dots in filename allowed, extension should be "d"
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