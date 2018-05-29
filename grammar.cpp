#include "tdd/simple_tests.hpp"

// for tests
#include <iostream>
#include <string>

// for lib
#include <cstddef>
#include <string_view>
#include <algorithm>

// some references
// https://swtch.com/~rsc/regexp/               (Implementing Regular Expressions)
// https://swtch.com/~rsc/regexp/regexp1.html   (Regular Expression Matching Can Be Simple And Fast)

// lazy vs greedy

namespace grammar
{
    namespace detail
    {
        static constexpr std::string_view _escaped_names_[] =
        {
            R"(\0)", R"(\x01)", R"(\x02)", R"(\x03)", R"(\x04)", R"(\x05)", R"(\x06)", R"(\a)"
            , R"(\b)", R"(\t)", R"(\n)", R"(\v)", R"(\f)", R"(\r)", R"(\x0E)", R"(\x0F)"
            , R"(\x10)", R"(\x11)", R"(\x12)", R"(\x13)", R"(\x14)", R"(\x15)", R"(\x16)", R"(\x17)"
            , R"(\x18)", R"(\x19)", R"(\x1a)", R"(\x1b)", R"(\x1c)", R"(\x1d)", R"(\x1e)", R"(\x1f)"
            , " ", "!", R"('\")", "#", "$", "%", "&", "'"
            , "(", ")", "*", R"(\+)", ",", R"(\-)", R"(\.)", "/"
            , "0", "1", "2", "3", "4", "5", "6", "7"
            , "8", "9", ":", ";", "<", "=", ">", R"(\?)"
            , "@", "A", "B", "C", "D", "E", "F", "G"
            , "H", "I", "J", "K", "L", "M", "N", "O"
            , "P", "Q", "R", "S", "T", "U", "V", "W"
            , "X", "Y", "Z", "[", R"(\\)", R"(\])", R"(\^)", "_"
            , "`", "a", "b", "c", "d", "e", "f", "g"
            , "h", "i", "j", "k", "l", "m", "n", "o"
            , "p", "q", "r", "s", "t", "u", "v", "w"
            , "x", "y", "z", "{", "|", "}", "~", R"(\x7F)"
        };
        static constexpr std::string_view _escaped_names_unknown = { "■", 1 };
        static constexpr auto to_escaped(char _c) { return (((unsigned char)_c)<128) ? _escaped_names_[((unsigned char)_c)] : _escaped_names_unknown; }
    }

    struct search_view;

    struct silent_logger
    {
        // T should not match 'search_view' to avoid ambiguity
        template<typename T>
        silent_logger& operator<<(const T&)
        {
            return *this;
        }
        silent_logger& operator<<(std::ostream& (*)(std::ostream&))
        {
            return *this;
        }
    };

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

        // constexpr auto get_length() const { return match_lenght_; }
        // constexpr auto store_lenght() { if (match_lenght_stored_ != invalid_match_length_) throw; match_lenght_stored_ = match_lenght_; }
        // constexpr auto restore_lenght() { if (match_lenght_stored_ == invalid_match_length_) throw; match_lenght_ = match_lenght_stored_; match_lenght_stored_ = invalid_match_length_; }

        template<typename OUTPUT_STREAM_TYPE, typename = std::enable_if_t<!std::is_same_v<OUTPUT_STREAM_TYPE, silent_logger>>>
        friend OUTPUT_STREAM_TYPE& operator<<(OUTPUT_STREAM_TYPE& _os, search_view _sv)
        {
            _os << _sv.view_;
            return _os;
        }
    private:
        const std::string_view view_;
        size_type match_start_index_ = 0;
        size_type match_lenght_ = 0;
        //static constexpr size_type invalid_match_length_ = std::numeric_limits<size_type>::max();
        //size_type match_lenght_stored_ = invalid_match_length_;
    };

// terminals
    template<char C, char... Cs>
    struct char_among
    {
        template<typename FUNCTOR, typename LOGGER>
        static constexpr bool parse(const search_view& _sview, FUNCTOR yield_return, [[maybe_unused]] LOGGER& _logger)
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
                return char_among<Cs...>::parse(_sview, yield_return, _logger);
            else
                return false;
        }

        template<typename OUTPUT_STREAM_TYPE, typename = std::enable_if_t<!std::is_same_v<OUTPUT_STREAM_TYPE, silent_logger>>>
        friend OUTPUT_STREAM_TYPE& operator<<(OUTPUT_STREAM_TYPE& _os, char_among)
        {
            if constexpr (sizeof...(Cs)!=0)
            {
                _os << '[' << detail::to_escaped(C);
                (_os << ... << detail::to_escaped(Cs)) << ']';
            }
            else
            {
                _os << detail::to_escaped(C);
            }
            return _os;
        }

        static constexpr std::size_t min_size()
        {
            return 1;
        }
    };

    using whitespace = char_among<' ', '\t', '\r', '\n'>;

    struct any_char
    {
        template<typename FUNCTOR, typename LOGGER>
        static constexpr bool parse(const search_view& _sview, FUNCTOR yield_return, [[maybe_unused]]LOGGER& _logger)
        {
            auto view_copy = _sview;
            view_copy.absorb();
            yield_return( view_copy );
            return true;
        }

        template<typename OUTPUT_STREAM_TYPE, typename = std::enable_if_t<!std::is_same_v<OUTPUT_STREAM_TYPE, silent_logger>>>
        friend OUTPUT_STREAM_TYPE& operator<<(OUTPUT_STREAM_TYPE& _os, any_char)
        {
            _os << '.';
            return _os;
        }

        static constexpr std::size_t min_size()
        {
            return 1;
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

    // boundary aka \b

// compositors
    template<typename HEAD, typename... TAIL>
    struct any_of
    {
        template<typename FUNCTOR, typename LOGGER>
        static constexpr bool parse(const search_view& _sview, FUNCTOR yield_return, LOGGER& _logger)
        {
            if (HEAD::parse(_sview, [&yield_return](const auto& _trailing_sview){ return yield_return(_trailing_sview); }, _logger ))
            {
                return true;
            }
            else
            {
                if constexpr (sizeof...(TAIL)==0)
                    return false;
                else
                    return any_of<TAIL...>::parse(_sview, yield_return, _logger);
            }
        }

        template<typename OUTPUT_STREAM_TYPE, typename = std::enable_if_t<!std::is_same_v<OUTPUT_STREAM_TYPE, silent_logger>>>
        friend OUTPUT_STREAM_TYPE& operator<<(OUTPUT_STREAM_TYPE& _os, any_of)
        {
            _os << '(' << HEAD{};
            (..., [&_os](const auto& arg){ _os << '|' << arg; }(TAIL{}) ) ;
            _os << ')';
            return _os;
        }

        static constexpr std::size_t min_size()
        {
            return std::min({ HEAD::min_size(), TAIL::min_size()... });
        }
    };

    // TODO: should I replace the generic "is_not" unary by the specific "char_not_among" terminal
    template<typename PREDICATE>
    struct is_not
    {
        template<typename FUNCTOR, typename LOGGER>
        static constexpr bool parse(const search_view& _sview, FUNCTOR yield_return, LOGGER& _logger)
        {
            if (PREDICATE::parse(_sview, [](const auto&){ return false; }, _logger))
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

        template<typename OUTPUT_STREAM_TYPE, typename = std::enable_if_t<!std::is_same_v<OUTPUT_STREAM_TYPE, silent_logger>>>
        friend OUTPUT_STREAM_TYPE& operator<<(OUTPUT_STREAM_TYPE& _os, is_not)
        {
            _os << '^' << PREDICATE{};
            return _os;
        }

        static constexpr std::size_t min_size()
        {
            return PREDICATE::min_size();
        }
    };

    template<std::size_t N, typename PREDICATE>
    struct at_least
    {
        template<typename FUNCTOR, typename LOGGER>
        static constexpr bool parse(const search_view& _sview, FUNCTOR yield_return, LOGGER& _logger)
        {
            if (!_sview.empty())
            {
                if constexpr(N <= 1)
                {
                    return PREDICATE::parse(_sview, [&yield_return, &_logger](const auto& trailing_view)
                    {
                        yield_return(trailing_view);
                        at_least<0, PREDICATE>::parse(trailing_view, yield_return, _logger);
                    }, _logger) || ( N == 0 );
                }
                else
                {
                    PREDICATE::parse(_sview, [&yield_return, &_logger](const auto& trailing_view)
                    {
                        at_least<N-1, PREDICATE>::parse(trailing_view, yield_return, _logger);
                    });
                    return false;
                }
            }
            return (N == 0);
        }

        template<typename OUTPUT_STREAM_TYPE, typename = std::enable_if_t<!std::is_same_v<OUTPUT_STREAM_TYPE, silent_logger>>>
        friend OUTPUT_STREAM_TYPE& operator<<(OUTPUT_STREAM_TYPE& _os, at_least)
        {
            _os << PREDICATE{};
            if constexpr(N==0)
                _os << '*';
            else if constexpr(N==1)
                _os << '+';
            else
                _os << '{' << N << ",}";
            return _os;
        }

        static constexpr std::size_t min_size()
        {
            return PREDICATE::min_size() * N;
        }
    };

    // TODO: at_most<N, PREDICATE> =>  (0 to N) repititions

    // TODO: times<N, PREDICATE> => (exactly N ) repititions

    template<typename HEAD, typename... TAIL>
    struct sequence // aka concatenate
    {
        template<typename FUNCTOR, typename LOGGER>
        static constexpr bool parse(const search_view& _sview, FUNCTOR yield_return, LOGGER& _logger)
        {
            _logger << "regex = \"" << sequence{} << "\", string = \"" << _sview << "\", check: \"" << HEAD{} << "\"" << std::endl;
            auto success = false;
            HEAD::parse(_sview, [&success, &yield_return, &_logger](auto _trailing_view) // intentional copy (test purpose for now)
            {
                if constexpr(sizeof...(TAIL) == 0)
                {
                    (void)_logger; // for clang (unused lambda capture warning)
                    yield_return(_trailing_view);
                    success = true;
                }
                else
                {
                    success = sequence<TAIL...>::parse(_trailing_view, yield_return, _logger);
                }
            }, _logger);
            return success;
        }

        template<typename OUTPUT_STREAM_TYPE, typename = std::enable_if_t<!std::is_same_v<OUTPUT_STREAM_TYPE, silent_logger>>>
        friend OUTPUT_STREAM_TYPE& operator<<(OUTPUT_STREAM_TYPE& _os, sequence)
        {
            _os << '(' << HEAD{};
            (_os << ... << TAIL{}) << ')';
            return _os;
        }

        static constexpr std::size_t min_size()
        {
            return HEAD::min_size() + (... + TAIL::min_size());
        }
    };

    template<typename PREDICATE>
    struct optional // aka one_or_zero aka at_most<1, PREDICATE>   x{,1}
    {
        template<typename FUNCTOR, typename LOGGER>
        static constexpr bool parse(const search_view& _sview, FUNCTOR yield_return, LOGGER& _logger)
        {
            _logger << "optional..." << std::endl;
            PREDICATE::parse(_sview, [&yield_return, &_logger](const auto& trailing_view)
            {
                _logger << "- match: \"" << trailing_view.match() << "\"" << std::endl;
                yield_return(trailing_view);
            }, _logger);
            yield_return(_sview);
            _logger << std::endl;
            return true;
        }

        template<typename OUTPUT_STREAM_TYPE, typename = std::enable_if_t<!std::is_same_v<OUTPUT_STREAM_TYPE, silent_logger>>>
        friend OUTPUT_STREAM_TYPE& operator<<(OUTPUT_STREAM_TYPE& _os, optional)
        {
            _os << PREDICATE{};
            _os << '?';
            return _os;
        }

        static constexpr std::size_t min_size()
        {
            return 0;
        }
    };

// algorithm

    // check for an exact match (nor leading neither trailing characters)
    template<typename PATTERN, typename LOGGER>
    auto match(search_view _sview, LOGGER& _logger)
    {
        std::size_t match_count = 0;
        _logger << "grammar::match in \"" << _sview << "\":" << std::endl;

        // maybe yield_callback could return YIELD_CONTINUE or YIELD_BREAK
        auto yield_callback = [&match_count, &_logger](const auto& _trailing_view)
        {
            if (_trailing_view.empty())
                ++match_count;
            _logger << "--> \"" << _trailing_view.match() << "\"" << std::endl;
        };

        PATTERN::parse(_sview, yield_callback, _logger);

        _logger << "... " << match_count << " " << ( match_count > 1 ?"matches":"match") << std::endl;
        return match_count != 0;
    }

    template<typename PATTERN>
    auto match(search_view _sview)
    {
        auto logger = silent_logger{};
        return match<PATTERN>(_sview, logger);
    }

    // search the pattern anywhere in the string
    template<typename PATTERN, typename LOGGER>
    bool search(search_view _sview, LOGGER& _logger)
    {
        _logger << "grammar::search \"" << PATTERN{} << "\" in \"" << _sview << "\":" << std::endl;

        std::size_t match_count = 0;

        while(!_sview.empty())
        {
            _logger << "- sub-search in \"" << _sview << "\":" << std::endl;
            // maybe yield_callback could return YIELD_CONTINUE or YIELD_BREAK
            auto yield_callback = [&match_count, &_logger](const auto& _trailing_view)
            {
                ++match_count;
                _logger << "--> \"" << _trailing_view.match() << "\"" << std::endl;
            };

            PATTERN::parse(_sview, yield_callback, _logger);

            _sview.advance_start(); // remove the first character and retry
        }
        _logger << "... " << match_count << " " << ( match_count > 1 ?"matches":"match") << std::endl;
        return match_count!=0;
    }

    template<typename PATTERN>
    auto search(search_view _sview)
    {
        auto logger = silent_logger{};
        return search<PATTERN>(_sview, logger);
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
    // CHECK_TRUE(search<gr_perf<5>>("aaaaa"));
    //CHECK_TRUE(search<gr_perf<2>>("aa"));

    // TO DO: compile computation of the minimal string size to match the pattern

    //  regex = "a?a?aa", string = "aa", check first "a?"
    //  ->  OK  regex = "a?'a?aa", string = "a'a", check second "a?"
    //      ->  OK  regex = "a?a?'aa", string = "aa'", check first "a"
    //          -> NO (need a char "a" but string is empty)
    //      ->  NO  regex = "a?a?'aa", string = "a'a", check first "a"
    //          -> OK   regex = "a?a?a'a", string = "aa'", check second "a"
    //              -> NO (need a char "a" but string is empty)
    //  ->  NO  regex = "a?'a?aa",  string = "aa", check second "a?"
    //      ->  OK  regex = "a?a?'aa", string = "a'a", check first "a"
    //          ->  OK  regex = "a?a?a'a", string = "aa'", check second "a"
    //              ->  NO (need a char "a" but string is empty)
    //      ->  NO  regex = "a?a?'aa", string = "aa", check first "a"
    //          ->  OK  regex = "a?a?a'a", string = "a'a", check second "a"
    //              -> OK   regex = "a?a?aa'", string = "aa'", match (regex fully checked)
    using gr_perf_x =
        sequence<
            optional< char_among<'a'> >
            , optional< char_among<'a'> >
            , char_among<'a'>
            , char_among<'a'>
        >;

    CHECK_EQ(gr_perf_x::min_size(),2);
    CHECK_TRUE(search<gr_perf_x>("aa", std::cout));

    using gr_blood_group =
        sequence<
            any_of<
                sequence< char_among<'A'>, optional< char_among<'B'> > >,
                char_among< 'B' >,
                char_among< 'O' >
            >,
            any_of<char_among<'+'>,char_among<'-'>>
        >;
    CHECK_EQ(gr_blood_group::min_size(),2);
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
            >
        >;

    CHECK_EQ(gr_filename::min_size(),1);
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

    std::cout << "gr_blood_group: \"" << gr_blood_group{} << "\"" << std::endl;
    std::cout << "gr_filename: \"" << gr_filename{} << "\"" << std::endl;
    std::cout << "gr_perf_x: \"" << gr_perf_x{} << "\"" << std::endl;
    std::cout << "gr_perf<5>: \"" << gr_perf<5>{} << "\"" << std::endl;

    tdd::PrintTestResults([](const char* line){ std::cout << line << std::endl; } );
}