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
    struct result_t
    {
        bool                success;
        std::string_view    string_view;

        constexpr operator bool() const { return success; }
    };
// terminals
    template<char C>
    struct is_char
    {
        constexpr result_t operator()(std::string_view _sview) const
        {
            if (_sview.front() == C)
            {
                _sview.remove_prefix(1);
                return { true,  _sview };
            }
            else
                return { false, _sview };
        }
    };

    struct is_space
    {
        constexpr result_t operator()(std::string_view _sview) const
        {
            char c = _sview.front();
            if (c==' ' || c=='\t' || c=='\r' || c=='\n' || c=='\0')
            {
                _sview.remove_prefix(1);
                return { true, _sview };
            }
            else
                return { false, _sview };
        }
    };
// compositors
    template<typename HEAD, typename... TAIL>
    struct any_of
    {
        constexpr result_t operator()(std::string_view _sview) const
        {
            auto result = HEAD{}(_sview);
            if (result.success)
                return result;
            if constexpr (sizeof...(TAIL)==0)
                return { false, _sview };
            else
                return any_of<TAIL...>{}(_sview);
        }
    };

    template<typename T>
    struct is_not
    {
        constexpr result_t operator()(std::string_view _sview) const
        {
            auto result = T{}(_sview);
            if (result.success)
                return { false, _sview };
            else
            {
                _sview.remove_prefix(1); // the HARDCODED count 1 should depend on the underlying operators
                return { true, _sview };
            }
        }
    };

    template<std::size_t N, typename T>
    struct at_least
    {
        constexpr result_t operator()(std::string_view _sview) const
        {
            T checker{};
            std::size_t matches_count = 0;
            auto work_sview = _sview;
            while(true)
            {
                if (work_sview.empty())
                    break;
                auto result = checker(work_sview);
                if (!result.success)
                    break;
                ++matches_count;
                //std::cout << work_sview << std::endl;
                work_sview = result.string_view;
            }
            if (matches_count < N)
            {
                return { false, _sview };
            }
            else
            {
                return { true, work_sview };
            }
        }
    };

    template<typename HEAD, typename... TAIL>
    struct sequence // aka concatenate
    {
        constexpr result_t operator()(const std::string_view _sview) const
        {
            if(auto result = HEAD{}(_sview); result.success)
            {
                if constexpr(sizeof...(TAIL) == 0)
                {
                    return result;
                }
                else if (!result.string_view.empty())
                {
                    return sequence<TAIL...>{}(result.string_view);
                }
            }
            // if it fails we should try other matching results of the 
            return { false, _sview };
        }
    };
}

int main(void)
{
    using grammar::any_of;
    using grammar::is_char;
    using grammar::is_space;
    using grammar::is_not;
    using grammar::at_least;
    using grammar::sequence;

    using filename_allowed_characters_t = is_not<any_of<is_char<'\\'>,is_char<'/'>,is_char<'*'>,is_char<'\"'>,is_char<'*'>,is_char<':'>,is_char<'<'>,is_char<'>'>,is_char<'|'>,is_char<'?'>,is_space>>;
    using filename_t = sequence<at_least<1, filename_allowed_characters_t>, is_char<'.'>>;

    filename_t checker{};
    CHECK_FALSE(checker("allowed&.-_=.")); // NOT OK: trailing dot not allowed
    CHECK_TRUE(checker("allowed.cpp")); // OK
    CHECK_TRUE(checker("allowed")); // OK no extension
    CHECK_FALSE(checker(".ext")); // NOT OK: extension only not allowed for filenames (OK for folder names)
    CHECK_FALSE(checker("")); // NOT OK: empty
    CHECK_FALSE(checker("<notallowed")); // NOT OK, leading and trailing forbidden characters
    CHECK_FALSE(checker("notallowed>")); // NOT OK, leading and trailing forbidden characters
    CHECK_TRUE(checker("seperated. names")); // OK: matches "seperated"
    tdd::PrintTestResults([](const char* line){ std::cout << line << std::endl; } );
}