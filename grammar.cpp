#include "tdd/simple_tests.hpp"

// for tests
#include <iostream>
#include <string>

// for lib
#include <cstddef>
#include <string_view>
#include <vector>

// some references
// https://swtch.com/~rsc/regexp/               (Implementing Regular Expressions)
// https://swtch.com/~rsc/regexp/regexp1.html   (Regular Expression Matching Can Be Simple And Fast)

namespace grammar
{
    struct result_t
    {
        std::vector<std::string_view>   string_views;
        decltype(string_views.size()) size() const { return string_views.size(); }
        decltype(string_views.empty()) empty() const { return string_views.empty(); }
        std::string_view& operator[](std::size_t _index) { return string_views[_index]; }

        operator bool() const { return !string_views.empty(); }
    };
// terminals
    template<char C>
    struct is_char
    {
        result_t operator()(std::string_view _sview) const
        {
            if (_sview.front() == C)
            {
                _sview.remove_prefix(1);
                return { { _sview } };
            }
            else
                return { { } };
        }
    };

    struct is_space
    {
        result_t operator()(std::string_view _sview) const
        {
            char c = _sview.front();
            if (c==' ' || c=='\t' || c=='\r' || c=='\n' || c=='\0')
            {
                _sview.remove_prefix(1);
                return { { _sview } };
            }
            else
                return { { } };
        }
    };
// compositors
    template<typename HEAD, typename... TAIL>
    struct any_of
    {
        result_t operator()(std::string_view _sview) const
        {
            auto result = HEAD{}(_sview);
            if (result)
                return result;
            if constexpr (sizeof...(TAIL)==0)
                return { { } };
            else
                return any_of<TAIL...>{}(_sview);
        }
    };

    template<typename PREDICATE>
    struct is_not
    {
        result_t operator()(std::string_view _sview) const
        {
            auto result = PREDICATE{}(_sview);
            if (result)
                return { { } };
            else
            {
                _sview.remove_prefix(1); // the HARDCODED count 1 should depend on the underlying operators
                return { { _sview } };
            }
        }
    };

    template<std::size_t N, typename PREDICATE>
    struct at_least
    {
        result_t operator()(std::string_view _sview) const
        {
            PREDICATE checker{};
            std::size_t matches_count = 0;
            auto work_sview = _sview;
            std::vector<std::string_view> candidates;
            while(true)
            {
                if (work_sview.empty())
                    break;
                auto result = checker(work_sview);
                if (!result)
                    break;
                ++matches_count;
                {
                    if (result.string_views.size()!=1)
                        std::cout << "Toh!!" <<std::endl;
                }
                work_sview = result.string_views[0];
                if (matches_count>=N)
                {
                    candidates.push_back(work_sview);
                }
            }
            if (matches_count < N)
            {
                return { { } };
            }
            else
            {
                return { std::move(candidates) };
            }
        }
    };

    template<typename HEAD, typename... TAIL>
    struct sequence // aka concatenate
    {
        result_t operator()(const std::string_view _sview) const
        {
            if(auto results = HEAD{}(_sview))
            {
                std::cout << "(" << results.string_views.size() << ")" << std::endl;
                if constexpr(sizeof...(TAIL) == 0)
                {
                    return results;
                }
                else if (!results.string_views.empty())
                {
                    // in fact, we want to filter results
                    std::vector<std::string_view> candidates;
                    for(auto& result_view: results.string_views)
                    {
                        std::cout << "-> " << result_view << std::endl;
                        if (sequence<TAIL...>{}(result_view))
                        {
                            candidates.push_back(result_view);
                        }
                    }
                    std::cout << "-> (" << candidates.size() << ")" << std::endl;
                    return { std::move(candidates) };
                }
            }
            // if it fails we should try other matching results of the 
            return { { } };
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
    using filename_t = sequence<at_least<1, filename_allowed_characters_t>, is_char<'.'>, at_least<1, filename_allowed_characters_t>>;

    filename_t checker{};
    // CHECK_TRUE(checker("allowed&.-_=.")); // NOT OK: trailing dot not allowed
    CHECK_TRUE(checker("allowed.cpp")); // OK
    //CHECK_TRUE(checker("allowed")); // OK no extension
    // CHECK_FALSE(checker(".ext")); // NOT OK: extension only not allowed for filenames (OK for folder names)
    // CHECK_FALSE(checker("")); // NOT OK: empty
    // CHECK_FALSE(checker("<notallowed")); // NOT OK, leading and trailing forbidden characters
    // CHECK_FALSE(checker("notallowed>")); // NOT OK, leading and trailing forbidden characters
    // CHECK_TRUE(checker("seperated. names")); // OK: matches "seperated"
    tdd::PrintTestResults([](const char* line){ std::cout << line << std::endl; } );
}