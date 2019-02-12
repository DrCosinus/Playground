
template <typename T, typename V, const V &(T::*getter)() const, void (T::*setter)(const V &)>
struct Property final
{
    operator V() const { return (((T *)this)->*getter)(); }
    Property &operator=(const V &value)
    {
        (((T *)this)->*setter)(value);
        return *this;
    }
    Property &operator+=(const V &value)
    {
        *this = V{*this} + value;
        return *this;
    }
    template <typename U>
    Property &operator*=(const U &value)
    {
        *this = V{*this} * value;
        return *this;
    }
    /*
  // operator==(const Property<V>&, const V&), operator==(const V&, const
  // Property<V>&), operator==(const Property<V>&, const Property<V>&) should
  // exist if operator==(const V&, const V&)... same for other operators (!=,
  // <, <=, >, >=)
  constexpr bool operator==(const Property& prop) const {
    return this->value == prop.value;
  }
  constexpr bool operator!=(const Property& prop) const {
    return !(*this == prop);
  }
  constexpr bool operator==(const V& value) const {
    return this->value == value;
  }
  friend constexpr bool operator==(const V& value, const Property& prop) {
    return prop == value;
  }
*/
    // Property should accept affectation from types convertible to V
    // (respectively implicitly or explitcitly regarding the conversion from these
    // types to V...)
};