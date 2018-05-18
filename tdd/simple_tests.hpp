#pragma once
#include <cstddef>
#include <iostream>

inline std::size_t FailureCount = 0;
inline std::size_t CheckCount = 0;

#define __CHECK_OP(__E,__V,__OP,__FILE,__LINE) \
{ \
    ++CheckCount; \
    if (!(__E __OP __V)) \
    { \
        ++FailureCount; \
        std::cout << __FILE << '(' << __LINE << "): FAILURE: " << #__E << #__OP << #__V << " aka " << __E << #__OP << __V  << std::endl; \
    } \
}

#define CHECK_EQ(__E,__V) __CHECK_OP(__E,__V,== ,__FILE__,__LINE__)
#define CHECK_NE(__E,__V) __CHECK_OP(__E,__V,!= ,__FILE__,__LINE__)
#define FAIL() cout << __FILE__ << '(' << __LINE__ << "): FAILURE: always" << endl;

#define PRINT_TEST_RESULTS() std::cout << "Tests: " << (CheckCount-FailureCount) << "/" << CheckCount << std::endl
