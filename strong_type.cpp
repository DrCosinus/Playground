#include <memory>

template<typename UNDERLYING_TYPE, typename, typename... MODIFIER_TYPES>
struct strong_type
{
    strong_type(UNDERLYING_TYPE _value) : value_(std::move(_value)) {} // implicit construction from UNDERLYING_TYPE
    template<typename ANOTHER_UNDERLYING_TYPE, typename ANOTHER_TAG_TYPE>
    strong_type(const strong_type<ANOTHER_UNDERLYING_TYPE, ANOTHER_TAG_TYPE>&) = delete; // conversion from another strong_type

    // bool operator==(const strong_type& _rhs) const { return value_ == _rhs.value_; } // non extensible way of doing it
    bool operator==(const strong_type& _rhs) const
    {
        auto result = false;
        {
            using pack_expander = int[];
            [[maybe_unused]]pack_expander _{ (result |= MODIFIER_TYPES::Compare(value_, _rhs.value_) , 0)..., 0 };
        }
        return result;
    }

    bool operator!=(const strong_type& _rhs) const { return !(*this == _rhs); }
    bool operator<(const strong_type& _rhs) const { return value_ < _rhs.value_; }
private:
    UNDERLYING_TYPE value_;
};

struct comparable
{
    static bool Compare(float _lhs, float _rhs) { return _lhs == _rhs; }
};

struct oderable
{
    static bool Less(float _lhs, float _rhs) { return _lhs < _rhs; }
};

template<typename...>
struct commutative_addition
{
};

template<typename...>
struct divisible
{

};

// ------------------------

#include <iostream>

// a 3d vector without any semantic
template<typename T>
struct vector3
{
    T x, y, z;
};

// safe length unit => underlying value is in meter (should be carved in the code, not in a comment)
struct length_unit : strong_type<
    float           // underlying type
    , length_unit   // unique tag (thank to CRTP, it is the type itself)
    , comparable
    //, oderable
    >
{
    using strong_type::strong_type;
    length_unit() : strong_type{ 0 } {}
};

/*
struct position;

struct timespan;

struct velocity;

using displacement = strong_type
    <
    vector3<float>, // underlying type
    struct string_int_tag, // unique tag
    comparable, // displacement can use the underlying type comparison operators
    commutative_addition<position, position>, // displacement + position = position + displacement => give a position
    divisible<timespan, velocity> // displacement divide by timespan gives velocity
    >;
*/

int main()
{
    using namespace std;
    length_unit v1, v2;

    cout << boolalpha;

    cout << "(v1 == v2) is " << (v1 == v2) << endl;

    cout << boolalpha << is_same_v<decltype(v1), decltype(v2)> << noboolalpha << endl;
}