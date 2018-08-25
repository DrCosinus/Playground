#pragma once

#include <any>

namespace cuppa
{
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