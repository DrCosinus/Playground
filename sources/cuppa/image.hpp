#pragma once

#include <any>
#include <utility>

namespace cuppa
{
    struct Image
    {
        Image() = default;
        Image(const Image&) = default;
        Image(Image&&) = default;
        Image& operator=(const Image&) = default;
        Image& operator=(Image&&) = default;
        template<typename T>
        explicit Image(T&& nativeImage) { image = nativeImage; }
        template<typename T>
        T getNativeAs() const { return std::any_cast<T>(image); }
    private:
        std::any image;
    };
} // namespace cuppa