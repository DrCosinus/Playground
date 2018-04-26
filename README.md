# Playground

## strong type lib // WIP //

Modifiers:

**st** : strong_type, **ut** : underlying type

| | modifier                               | defined operators                   | needed operators of ut |
|-| -------------------------------------- | ----------------------------------- | ---------------------- |
|x| equalable                              | `==` `!=`                           | `==`                   |
|x| comparable                             | `==` `!=` `<` `>` `<=` `>=`         | `==` & `<`             |
|x| explicitly_convertible_to<T>::modifier | explicit convert to T               |                        |
|x| self_addable                           | st = st `+` st                      | `+`                    |
|x| self_substractable                     | st = st `-` st                      | `-`                    |
|x| self_multipliable                      | st = st `*` st                      | `*`                    |
|x| self_dividable                         | st = st `/` st                      | `/`                    |
|x| stringable                             | st.to_string() & std::to_string(st) |                        |
|x| incrementable                          | st++, ++st & st += st;              | ++ut & ut+=ut          |
|x| decrementable                          |                                     |                        |
| | invokable                              | st()                                |                        |
| | serializable                           |                                     |                        |
| | hashable                               |                                     |                        |
| | addable_to<OPERAND_T, RESULT_T>        | res = st + op = op + st             |                        |

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