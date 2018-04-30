# Playground

## strong type lib // WIP //

### Type safety

Wikipedia/Wiktionary definition:

In computer science, type safety is the extent to which a programming language **discourages** or **prevents** type errors.

At compiletime, check for:

- use of one type when another is intended
- operations that do not make sense
- use of values outside the defined space

#### Affine space

...

| measure    | difference   |
| ---------- | ------------ |
| position   | displacement |
| angle      | arc          |
| time_stamp | time_span    |
| pointer    | offset       |
| treasury   | cost         |

### Implementation

```cpp
namespace wit {
template<
  // the type in which the value is effectively stored
  typename UNDERLYING_TYPE,
  // ensure the unicity of the whole strong type... 
  // typically it could be the inherithing type itself
  typename TAG_TYPE,
  // a set of modifiers. 
  // the strong type will inherit from them
  template<typename> class... MODIFIER_TYPES 
  >
struct strong_type : MODIFIER_TYPES<strong_type<UNDERLYING_TYPE, TAG_TYPE, MODIFIER_TYPES...>>...
{
  explicit constexpr strong_type(UNDERLYING_TYPE _value)
  : value_(std::move(_value))
  {}
  // ...
private:
  UNDERLYING_TYPE value_;
}
} // namespace wit
```

- No implicit construction from underlying type
- No implicit copy-assignment

#### List of built-in modifiers

**st** : strong_type, **ut** : underlying type, **dt** difference type

| | modifier                                | defined<br>operators                | required<br>operators of ut |
|-| --------------------------------------- | ----------------------------------- | ---------------------- |
|x| equalable                               | `==` `!=`                           | `==`                   |
|x| comparable                              | `==` `!=`<br>`<` `>` `<=` `>=`      | `==` &<br>`<`          |
|x| explicitly_convertible_to\<T>::modifier | T(st)                               |                        |
|x| self_addable                            | st = st `+` st                      | `+`                    |
|x| self_substractable                      | st = st `-` st                      | `-`                    |
|x| self_multipliable                       | st = st `*` st                      | `*`                    |
|x| self_dividable                          | st = st `/` st                      | `/`                    |
|x| stringable                              | st.to_string() &<br>std::to_string(st) |                     |
|x| incrementable                           | st++, ++st &<br>st += st;           | ++ut &<br>ut+=ut       |
|x| decrementable                           |                                     |                        |
| | invokable                               | st()                                |                        |
| | serializable                            |                                     |                        |
| | hashable                                |                                     |                        |
| | addable_to<OPERAND_T, RESULT_T>         | res = st + op<br> res = op + st     |                        |
| | define_difference_type\<dt>             | st = st + dt<br>st = dt + st<br>dt = st - st | st and df have<br>the same ut |

```cpp
#include <wit/strong_type.hpp>

struct length_unit : wit::strong_type<
    float           // underlying type
    , length_unit   // unique tag (thank to CRTP, it is the type itself)
    , wit::comparable
    , wit::explicitly_convertible_to<float>::modifier
    , wit::self_addable
    , wit::stringable
    >
{
    using strong_type::strong_type;
    length_unit() : strong_type{ 0 } {}
};

struct time_unit : wit::strong_type<
    long long
    , time_unit
    , wit::comparable
    , wit::incrementable
    >
{
    using strong_type::strong_type;
    time_unit() : strong_type{ 0 } {}
};
```

### manage enum class as bool

```cpp
enum class Visibility { Visible, Hidden };
std::string std::to_string(cosnt Visibility& _v) { return { _v==Visibility::Visible?"Visible":"Hidden" }; }
```

### manage enum serialization by name

### handle constraints

### compose strong_type as aggregate of strong_type / static array of strong_type

row-major/column-major order

```cpp
struct vector3 : wit::aggregate<
    std::tuple<length_type,length_type,length_type>
    , vector3
    , wit::equalable
    >
{
    using aggregate::aggregate;
};
```