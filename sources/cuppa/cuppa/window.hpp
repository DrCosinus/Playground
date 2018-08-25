#pragma once

#include <any>

namespace cuppa
{
    struct Window
    {
        Window() = default;
        Window(const Window&) = default;
        Window(Window&&) = default;
        Window& operator=(const Window&) = default;
        Window& operator=(Window&&) = default;

        template<typename T>
        explicit Window(T&& native) { data = native; }
        template<typename T>
        T getNativeAs() const { return std::any_cast<T>(data); }
    private:
        std::any data;
    };
} //    namespace cuppa
