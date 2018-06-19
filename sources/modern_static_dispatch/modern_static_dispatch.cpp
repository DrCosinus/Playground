#include "simple_tests.hpp"
#include <iostream>

template<typename... Ts>
struct overloaded_lambda : Ts...
{
    using Ts::operator()...;
};

// deduction guide
template<typename... Ts>
overloaded_lambda(Ts...) -> overloaded_lambda<Ts...>;

int main(void)
{
    auto visit = overloaded_lambda{
        [](int){ std::cout << "integer" << std::endl; }
        , [](float){ std::cout << "float" << std::endl; }
        , [](double){ std::cout << "double" << std::endl; }
        , [](const char*){ std::cout << "const char*" << std::endl; }
        , [](auto){ std::cout << "unknown" << std::endl; }
    };
    visit(9);
    visit(9.0f);
    visit(9.0);
    visit("9");
    visit(9ul);

}
    // tdd::PrintTestResults([](const char* line){ std::cout << line << std::endl; } );