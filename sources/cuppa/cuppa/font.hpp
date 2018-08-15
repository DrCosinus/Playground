#pragma once

#include <any>
//#include <utility>

namespace cuppa
{
    struct Font
    {
        Font() = default;
        Font(const Font&) = default;
        Font(Font&&) = default;
        Font& operator=(const Font&) = default;
        Font& operator=(Font&&) = default;
        template<typename T>
        explicit Font(T&& nativeData) { data = nativeData; }
        template<typename T>
        T getNativeAs() const { return std::any_cast<T>(data); }
    private:
        std::any data;
    };
} //namespace cuppa