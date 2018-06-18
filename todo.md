# TODO

## unmutable string view

- constructible from const char*, literal string, std::string, custom strings, ... (expendability)
- cheap to convert to const char*, std::string, ...

## array view

## determinism helpers

## query selector

auto AllProps = Entities.Select([](auto& s){ return s.IsProp() });

## automation

## WIT

### commandline

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

### text

loadjson
loadxml

### math

Vector2d, Vector3d

### minimal media interface (graphics, inputs,...)

```cpp
namespace wit
{
  class Application
  {
    Application();
    virtual ~Application() = default;

    setSize();
    draw rect
    draw ellipse
    draw text
    draw line
    draw point
  };
} // namespace wit
```
