#pragma once

#include <any>

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

// TODO: must be moved elsewhere
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

// TODO: must be moved elsewhere
    struct DeviceContext
    {
        DeviceContext() = default;
        DeviceContext(const DeviceContext&) = default;
        DeviceContext(DeviceContext&&) = default;
        DeviceContext& operator=(const DeviceContext&) = default;
        DeviceContext& operator=(DeviceContext&&) = default;

        template<typename T>
        explicit DeviceContext(T&& native) { data = native; }
        template<typename T>
        T getNativeAs() const { return std::any_cast<T>(data); }
    private:
        std::any data;
    };
} // namespace cuppa