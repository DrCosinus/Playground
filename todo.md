# TODO

## unmutable string view

- constructible from const char*, literal string, std::string, custom strings, ... (expendability)
- cheap to convert to const char*, std::string, ...

## array view

## determinism helpers

## query selector

auto AllProps = Entities.Select([](auto& s){ return s.IsProp() });

## automation

## GENERIC NODE GRAPH EDITOR

### commandline

Scenario:

```cpp
using wit::Commandline::CommandlineWrapper;

auto cmdlineWrapper = new CommandlineWrapper{};
cmdlineWrapper.parseArguments(argc, argv);

// is any argument matches -force, /force, -force=yes, -force:1, /force=0, or /force:"true"
if (cmdlineWrapper.isDefined("force"))
{

}

// is any argument matches -value, /value, -value=yes, -value:1, /value=0, or /value:"true"
// 
if (auto [isDefined, value] cmdlineWrapper.TryGet("value"); isDefined)
{
  //  value is a std::string_view
}
```

one function to rule them all.
compile-time visitor

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

[Back to README](README.md)