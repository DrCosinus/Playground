#pragma once

namespace wit
{
enum class LengthOrder
{
};
constexpr LengthOrder operator+(LengthOrder _lhs, LengthOrder _rhs)
{
    return LengthOrder{static_cast<int>(_lhs) + static_cast<int>(_rhs)};
}
constexpr LengthOrder operator-(LengthOrder _lhs, LengthOrder _rhs)
{
    return LengthOrder{static_cast<int>(_lhs) - static_cast<int>(_rhs)};
}
constexpr bool operator==(LengthOrder _lhs, int _rhs)
{
    return static_cast<int>(_lhs) == (_rhs);
}

enum class TimeOrder
{
};
constexpr TimeOrder operator+(TimeOrder _lhs, TimeOrder _rhs)
{
    return TimeOrder{static_cast<int>(_lhs) + static_cast<int>(_rhs)};
}
constexpr TimeOrder operator-(TimeOrder _lhs, TimeOrder _rhs)
{
    return TimeOrder{static_cast<int>(_lhs) - static_cast<int>(_rhs)};
}
constexpr bool operator==(TimeOrder _lhs, int _rhs)
{
    return static_cast<int>(_lhs) == (_rhs);
}

enum class MassOrder
{
};
constexpr MassOrder operator+(MassOrder _lhs, MassOrder _rhs)
{
    return MassOrder{static_cast<int>(_lhs) + static_cast<int>(_rhs)};
}
constexpr MassOrder operator-(MassOrder _lhs, MassOrder _rhs)
{
    return MassOrder{static_cast<int>(_lhs) - static_cast<int>(_rhs)};
}
constexpr bool operator==(MassOrder _lhs, int _rhs)
{
    return static_cast<int>(_lhs) == (_rhs);
}

template <LengthOrder LENGTH_ORDER, TimeOrder TIME_ORDER, MassOrder MASS_ORDER>
struct Quantity
{
    explicit constexpr Quantity(float _value) : value_{_value} {}

    constexpr auto operator=(const Quantity &_rhs)
    {
        value_ = _rhs.value_;
        return *this;
    }

    template <LengthOrder OTHER_LENGTH_ORDER, TimeOrder OTHER_TIME_ORDER, MassOrder OTHER_MASS_ORDER>
    constexpr auto operator*(const Quantity<OTHER_LENGTH_ORDER, OTHER_TIME_ORDER, OTHER_MASS_ORDER> &_rhs) const
    {
        return Quantity<LENGTH_ORDER + OTHER_LENGTH_ORDER, TIME_ORDER + OTHER_TIME_ORDER, MASS_ORDER + OTHER_MASS_ORDER>{value_ * _rhs.value_};
    }

    template <LengthOrder OTHER_LENGTH_ORDER, TimeOrder OTHER_TIME_ORDER, MassOrder OTHER_MASS_ORDER>
    constexpr auto operator/(const Quantity<OTHER_LENGTH_ORDER, OTHER_TIME_ORDER, OTHER_MASS_ORDER> &_rhs) const
    {
        return Quantity<LENGTH_ORDER - OTHER_LENGTH_ORDER, TIME_ORDER - OTHER_TIME_ORDER, MASS_ORDER + OTHER_MASS_ORDER>{value_ * _rhs.value_};
    }

    template <LengthOrder OTHER_LENGTH_ORDER, TimeOrder OTHER_TIME_ORDER, MassOrder OTHER_MASS_ORDER>
    constexpr auto operator+(const Quantity<OTHER_LENGTH_ORDER, OTHER_TIME_ORDER, OTHER_MASS_ORDER> &_rhs) const
    {
        static_assert(LENGTH_ORDER == OTHER_LENGTH_ORDER && TIME_ORDER == OTHER_TIME_ORDER && MASS_ORDER == OTHER_MASS_ORDER, "Can not add quantities with mismatching units!");
        return Quantity{value_ + _rhs.value_};
    }

    template <LengthOrder OTHER_LENGTH_ORDER, TimeOrder OTHER_TIME_ORDER, MassOrder OTHER_MASS_ORDER>
    constexpr auto operator-(const Quantity<OTHER_LENGTH_ORDER, OTHER_TIME_ORDER, OTHER_MASS_ORDER> &_rhs) const
    {
        static_assert(LENGTH_ORDER == OTHER_LENGTH_ORDER && TIME_ORDER == OTHER_TIME_ORDER && MASS_ORDER == OTHER_MASS_ORDER, "Can not substract quantities with mismatching units!");
        return Quantity{value_ - _rhs.value_};
    }
    //+= -= *= /= == !=  < > <= >=

    static constexpr auto getLengthOrder() { return LENGTH_ORDER; }
    static constexpr auto getTimeOrder() { return TIME_ORDER; }
    static constexpr auto getMassOrder() { return MASS_ORDER; }
    //private:
    float value_;
};

template <LengthOrder LENGTH_ORDER, TimeOrder TIME_ORDER, MassOrder MASS_ORDER>
constexpr bool IsScalar(const Quantity<LENGTH_ORDER, TIME_ORDER, MASS_ORDER> &)
{
    return LENGTH_ORDER == 0 && TIME_ORDER == 0 && MASS_ORDER == 0;
}

template <LengthOrder LENGTH_ORDER, TimeOrder TIME_ORDER, MassOrder MASS_ORDER>
constexpr bool IsLength(const Quantity<LENGTH_ORDER, TIME_ORDER, MASS_ORDER> &)
{
    return LENGTH_ORDER == 1 && TIME_ORDER == 0 && MASS_ORDER == 0;
}

template <LengthOrder LENGTH_ORDER, TimeOrder TIME_ORDER, MassOrder MASS_ORDER>
constexpr bool IsSpeed(const Quantity<LENGTH_ORDER, TIME_ORDER, MASS_ORDER> &)
{
    return LENGTH_ORDER == 1 && TIME_ORDER == -1 && MASS_ORDER == 0;
}

template <LengthOrder LENGTH_ORDER, TimeOrder TIME_ORDER, MassOrder MASS_ORDER>
constexpr bool IsAcceleration(const Quantity<LENGTH_ORDER, TIME_ORDER, MASS_ORDER> &)
{
    return LENGTH_ORDER == 1 && TIME_ORDER == -2 && MASS_ORDER == 0;
}

template <LengthOrder LENGTH_ORDER, TimeOrder TIME_ORDER, MassOrder MASS_ORDER>
constexpr bool IsForce(const Quantity<LENGTH_ORDER, TIME_ORDER, MASS_ORDER> &)
{
    return LENGTH_ORDER == 1 && TIME_ORDER == -2 && MASS_ORDER == 1;
}

using Meter = Quantity<LengthOrder{1}, TimeOrder{0}, MassOrder{0}>;
using Second = Quantity<LengthOrder{0}, TimeOrder{1}, MassOrder{0}>;

struct Hertz : Quantity<LengthOrder{0}, TimeOrder{-1}, MassOrder{0}>
{
    using Quantity::Quantity;
};

struct Scalar : Quantity<LengthOrder{0}, TimeOrder{0}, MassOrder{0}>
{
    using Quantity::Quantity;
};

} // namespace wit