#include "simple_tests.hpp"

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

    enum class verboseness { silent, verbose };

    template<verboseness V>
    struct line_logger
    {
        struct scoped_indent final
        {
            explicit scoped_indent(line_logger& _logger)
            : logger_{_logger}
            {
                logger_.push_indent();
            }
            ~scoped_indent() { logger_.pop_indent(); }
            line_logger& logger_;
        };
        auto make_scoped_indent() { return scoped_indent{ *this }; }

        using write_line_delegate = void(*)(std::size_t, std::string_view);
        line_logger(write_line_delegate _write_line_delegate) : write_line_delegate_{_write_line_delegate} { }
        line_logger(const line_logger&) = delete; // a priori not desirable
        line_logger(line_logger&&) = delete;      // a priori not desirable

        template <typename T> auto& operator<<(T&& _value) { ss_ << _value; return *this; }

        auto& operator<<(const std::nullptr_t&) { ss_ << "(null)"; return *this; }

        auto& operator<<(bool _boolean) { ss_ << (_boolean ? "true" : "false"); return *this; }

        auto& operator<<(line_logger&(*manipulator)(line_logger&)) { return manipulator(*this); }

        auto flush_line()
        {
            ss_.flush();
            write_line_delegate_(indent_, ss_.get_view());
            ss_.reset();
        }

    private:
        auto push_indent() { ++indent_; }
        auto pop_indent() { --indent_; }

        struct viewable_output_stringstream : std::stringstream
        {
            struct viewable_output_stringbuf : std::stringbuf
            {
                viewable_output_stringbuf() : std::stringbuf{ std::ios_base::out } {}
                auto get_view() const { return std::string_view{ pbase(), static_cast<std::size_t>(pptr() - pbase()) }; }
            };
            viewable_output_stringstream() { set_rdbuf(&buff_); }
            auto get_view() const { return buff_.get_view(); }
            void reset() { buff_.str(""); }

            viewable_output_stringbuf buff_;
        };
        std::size_t indent_ = 0;
        viewable_output_stringstream    ss_;
        write_line_delegate             write_line_delegate_;
    };

    template<verboseness V>
    line_logger<V>& push_line(line_logger<V>& _logger)
    {
        _logger.flush_line();
        return _logger;
    }

    template<typename T>
    inline constexpr bool is_logger_v = false;

    template<verboseness V>
    inline constexpr bool is_logger_v<line_logger<V>> = true;

    template<>
    struct line_logger<verboseness::silent>
    {
        template <typename T> auto operator<<(T&&) { return *this; }
        template <typename T, std::size_t N> auto operator<<(T(&)[N]) { return *this; }
        auto& operator<<(line_logger&(*)(line_logger&)) { return *this; }
        auto flush_line() const {}
        struct scoped_indent final
        { ~scoped_indent() {} }; // explicit non trivial destructor to avoir unused variable warning
        scoped_indent make_scoped_indent() { return {}; }
    };

    // a string_view for search with cursors retrieve string_view of matches
    struct search_view
    {
        using size_type = std::string_view::size_type;
        constexpr search_view() = default;
        template<std::size_t N>
        constexpr search_view(const char (&_literal)[N]) : view_{ _literal, N-1 } {}

        constexpr search_view& operator=(const search_view&) = default;

        constexpr bool empty() const { return match_start_index_+match_length_>=view_.size(); }
        constexpr auto front() const { return view_[match_start_index_+match_length_]; }
        constexpr void absorb() { ++match_length_; }
        constexpr void advance_start() { ++match_start_index_; match_length_ = 0; }
        constexpr auto match() const { return view_.substr(match_start_index_, match_length_); }

        template<typename OUTPUT_STREAM_TYPE, typename = std::enable_if_t<!is_logger_v<OUTPUT_STREAM_TYPE>>>
        friend OUTPUT_STREAM_TYPE& operator<<(OUTPUT_STREAM_TYPE& _os, search_view _sv)
        {
            _os //<< _sv.view_.substr(0, _sv.match_start_index_) << "…"
                << _sv.view_.substr(_sv.match_start_index_, _sv.match_length_)
                << "[_]" << _sv.view_.substr(_sv.match_start_index_+_sv.match_length_);
            return _os;
        }
    private:
        std::string_view view_;
        size_type match_start_index_ = 0;
        size_type match_length_ = 0;
    };

// terminals
    template<char... Cs>
    struct char_among
    {
        static_assert(sizeof...(Cs)>0);

        template<typename FUNCTOR, typename LOGGER>
        static constexpr bool parse(const search_view& _sview, FUNCTOR yield_return, [[maybe_unused]] LOGGER& _logger)
        {
            _logger << "char_among = \"" << char_among{} << "\", string = \"" << _sview << "\"" << push_line;

            bool success = false;
            (..., [&success, &_sview, &yield_return](char _c)
            {
                if (!success)
                {
                    if (_c=='\0')
                    {
                        if (_sview.empty())
                        {   // special handling for empty view when _c is '\0'
                            yield_return( _sview );
                            success = true;
                        }
                    }
                    if (!_sview.empty() && _sview.front() == _c)
                    {
                        auto view_copy = _sview;
                        view_copy.absorb();
                        yield_return( view_copy );
                        success = true;
                    }
                }
            }(Cs));
            if (!success)
                _logger << "->  FAIL" << push_line;
            return success;
        }

        template<typename OUTPUT_STREAM_TYPE, typename = std::enable_if_t<!is_logger_v<OUTPUT_STREAM_TYPE>>>
        friend OUTPUT_STREAM_TYPE& operator<<(OUTPUT_STREAM_TYPE& _os, char_among)
        {
            if constexpr (sizeof...(Cs)!=1)
            {
                _os << '[';
                (_os << ... << detail::to_escaped(Cs)) << ']';
            }
            else
            {
                _os << detail::to_escaped(Cs...);
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
        static constexpr bool parse(const search_view& _sview, FUNCTOR yield_return, LOGGER& _logger)
        {
            _logger << "any_char = \"" << any_char{} << "\", string = \"" << _sview << "\"" << push_line;
            if (!_sview.empty() && _sview.front()!='\0')
            {
                auto view_copy = _sview;
                view_copy.absorb();
                yield_return( view_copy );
                return true;
            }
            else
            {
                _logger << "->  FAIL" << push_line;
                return false;
            }
        }

        template<typename OUTPUT_STREAM_TYPE, typename = std::enable_if_t<!is_logger_v<OUTPUT_STREAM_TYPE>>>
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
    template<typename... PREDICATES>
    struct any_of
    {
        static_assert(sizeof...(PREDICATES)>0);

        template<typename FUNCTOR, typename LOGGER>
        static constexpr bool parse(const search_view& _sview, FUNCTOR yield_return, LOGGER& _logger)
        {
            _logger << "any_of = \"" << any_of{} << "\", string = \"" << _sview << "\"" << push_line;
            auto scoped_indent = _logger.make_scoped_indent();

            bool success = false;
            (..., [&success, &_sview, &yield_return, &_logger](auto _predicate)
            {
                if (!success)
                {
                    if (_predicate.parse(_sview, [&yield_return](const auto& _trailing_sview){ return yield_return(_trailing_sview); }, _logger ))
                    {
                        success = true;
                    }
                }
            }(PREDICATES{}));
            if (!success)
                _logger << "->  FAIL" << push_line;
            return success;
        }

        template<typename OUTPUT_STREAM_TYPE, typename = std::enable_if_t<!is_logger_v<OUTPUT_STREAM_TYPE>>>
        friend OUTPUT_STREAM_TYPE& operator<<(OUTPUT_STREAM_TYPE& _os, any_of)
        {
            _os << '(';
            // cannot inline the lambda in the fold expression because it expand each parameter independently, so first would not.
            auto lambda = [&_os, first = true](const auto& arg) mutable { if (first) { first = false; } else { _os << '|'; } _os << arg; };
            (..., lambda(PREDICATES{}) ) ;
            _os << ')';
            return _os;
        }

        static constexpr std::size_t min_size()
        {
            return std::min({ PREDICATES::min_size()... });
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

        template<typename OUTPUT_STREAM_TYPE, typename = std::enable_if_t<!is_logger_v<OUTPUT_STREAM_TYPE>>>
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
            std::size_t occurence_count = 0;
            auto view = _sview;
            while(true)
            {
                if (view.empty())
                    break;
                if (search_view trailing_view; PREDICATE::parse(view, [&trailing_view](const auto& _trailing_view)
                {
                    trailing_view = _trailing_view;
                }, _logger))
                {
                    view = trailing_view;
                    ++occurence_count;
                    if (occurence_count>=N)
                        yield_return(view);
                }
                else
                    break;
            }
            return occurence_count>=N;
        }

        template<typename OUTPUT_STREAM_TYPE, typename = std::enable_if_t<!is_logger_v<OUTPUT_STREAM_TYPE>>>
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

#define NEW_SEQUENCE_IMPLEMENTATION 0
#if NEW_SEQUENCE_IMPLEMENTATION
    template<typename... PREDICATES>
    struct sequence // aka concatenate
    {
        template<typename FUNCTOR, typename LOGGER>
        static constexpr bool parse(const search_view& _sview, FUNCTOR yield_return, LOGGER& _logger)
        {
            _logger << "sequence = \"" << sequence{} << "\", string = \"" << _sview << "\"" << push_line;
            auto scoped_indent = _logger.make_scoped_indent();
            auto view = _sview;
            auto lambda = [success = true, &view, &_logger](auto _predicate) mutable
            {
                if (success)
                {
                    success = _predicate.parse(view, [&view](auto _trailing_view)
                    {
                        view = _trailing_view;
                    }, _logger);
                }
                return success;
            };
            auto success = (..., lambda(PREDICATES{}));
            if (success)
                yield_return(view);
            return success;
        }

        template<typename OUTPUT_STREAM_TYPE, typename = std::enable_if_t<!is_logger_v<OUTPUT_STREAM_TYPE>>>
        friend OUTPUT_STREAM_TYPE& operator<<(OUTPUT_STREAM_TYPE& _os, sequence)
        {
            _os << '(';
            (_os << ... << PREDICATES{}) << ')';
            return _os;
        }

        static constexpr std::size_t min_size()
        {
            return (... + PREDICATES::min_size());
        }
    };
#else
    template<typename HEAD, typename... TAIL>
    struct sequence // aka concatenate
    {
        template<typename FUNCTOR, typename LOGGER>
        static constexpr bool parse(const search_view& _sview, FUNCTOR yield_return, LOGGER& _logger)
        {
            _logger << "sequence = \"" << sequence{} << "\", string = \"" << _sview << "\"" << push_line;
            auto scoped_indent = _logger.make_scoped_indent();
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

        template<typename OUTPUT_STREAM_TYPE, typename = std::enable_if_t<!is_logger_v<OUTPUT_STREAM_TYPE>>>
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
#endif

    template<typename PREDICATE>
    struct optional // aka one_or_zero aka at_most<1, PREDICATE>   x{,1}
    {
        template<typename FUNCTOR, typename LOGGER>
        static constexpr bool parse(const search_view& _sview, FUNCTOR yield_return, LOGGER& _logger)
        {
            _logger << "optional = \"" << optional{} << "\", string = \"" << _sview << "\"" << push_line;

            {
                auto scoped_indent = _logger.make_scoped_indent();
                PREDICATE::parse(_sview, [&yield_return](const auto& trailing_view)
                {
                    yield_return(trailing_view);
                }, _logger);
            }
            yield_return(_sview);
            return true;
        }

        template<typename OUTPUT_STREAM_TYPE, typename = std::enable_if_t<!is_logger_v<OUTPUT_STREAM_TYPE>>>
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
        _logger << "grammar::match in \"" << _sview << "\":" << push_line;

        // maybe yield_callback could return YIELD_CONTINUE or YIELD_BREAK
        auto yield_callback = [&match_count, &_logger](const auto& _trailing_view)
        {
            if (_trailing_view.empty())
                ++match_count;
            _logger << "--> \"" << _trailing_view.match() << "\"" << push_line;
        };

        PATTERN::parse(_sview, yield_callback, _logger);

        _logger << "... " << match_count << " " << ( match_count > 1 ?"matches":"match") << push_line;
        return match_count != 0;
    }

    template<typename PATTERN>
    auto match(search_view _sview)
    {
        auto logger = line_logger<verboseness::silent>{};
        return match<PATTERN>(_sview, logger);
    }

    // search the pattern anywhere in the string
    template<typename PATTERN, typename LOGGER>
    bool search(search_view _sview, LOGGER& _logger)
    {
        _logger << "grammar::search \"" << PATTERN{} << "\" in \"" << _sview << "\":" << push_line;

        std::size_t match_count = 0;

        while(!_sview.empty())
        {
            _logger << "- sub-search in \"" << _sview << "\":" << push_line;
            // maybe yield_callback could return YIELD_CONTINUE or YIELD_BREAK
            auto yield_callback = [&match_count, &_logger](const auto& _trailing_view)
            {
                ++match_count;
                _logger << "->  MATCH FOUND \"" << _trailing_view.match() << "\"" << push_line;
            };

            PATTERN::parse(_sview, yield_callback, _logger);

            _sview.advance_start(); // remove the first character and retry
        }
        _logger << "... " << match_count << " " << ( match_count > 1 ?"matches":"match") << push_line;
        return match_count!=0;
    }

    template<typename PATTERN>
    auto search(search_view _sview)
    {
        auto logger = line_logger<verboseness::silent>{};
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
            , char_among<'b','a'>
            , char_among<'a'>
        >;

    CHECK_EQ(gr_perf_x::min_size(),2);
    grammar::line_logger<grammar::verboseness::verbose> verbose{ [](auto indent_count, auto _view)
    {
        for (std::size_t i = 0; i<indent_count; ++i)
            std::cout << "    ";
        std::cout << _view << std::endl;
    } };
    CHECK_TRUE(search<gr_perf_x>("aa"));

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
            // gr_extension_allowed_character,
            optional<
                sequence<
                    char_among<'.'>,
                    at_least<1, gr_extension_allowed_character>
                >
            >
        >;

    CHECK_EQ(gr_filename::min_size(),1);

    // search will TRUE most of the time (FALSE for empty string).
    CHECK_FALSE(search<gr_filename>("")); // NOT OK: empty
    CHECK_TRUE(search<gr_filename>("<a.b.c>")); // OK: so many valid file name in this string
    CHECK_TRUE(search<gr_filename>("trailing.dot.")); // OK: "allowed&.-_=" among others is OK
    CHECK_TRUE(search<gr_filename>("allowed")); // OK no extension is allowed
    CHECK_TRUE(search<gr_filename>(".ext")); // OK: "ext" among others is OK
    CHECK_TRUE(search<gr_filename>("seperated.cpp names")); // OK: matches "seperated"

    CHECK_TRUE(match<gr_filename>("allo.wed.ext")); // OK: dots in filename allowed,
    CHECK_TRUE(match<gr_filename>("allowed&.-_=.")); // NOT OK: trailing dot not allowed (no extension)... but the current expression matches if the extension is empty
    CHECK_TRUE(match<gr_filename>("allowed.cpp")); // OK
    CHECK_TRUE(match<gr_filename>("allowed..cpp")); // OK
    CHECK_TRUE(match<gr_filename>(".ext")); // OK: extension only allowed but not for the good reason .ext filename without extension
    CHECK_FALSE(match<gr_filename>("<notallowed")); // NOT OK, leading and trailing forbidden characters
    CHECK_FALSE(match<gr_filename>("notallowed>")); // NOT OK, leading and trailing forbidden characters

    std::cout << "gr_blood_group: \"" << gr_blood_group{} << "\"" << std::endl;
    std::cout << "gr_filename: \"" << gr_filename{} << "\"" << std::endl;
    std::cout << "gr_perf_x: \"" << gr_perf_x{} << "\"" << std::endl;
    std::cout << "gr_perf<5>: \"" << gr_perf<5>{} << "\"" << std::endl;

    tdd::PrintTestResults([](const char* line){ std::cout << line << std::endl; } );
}