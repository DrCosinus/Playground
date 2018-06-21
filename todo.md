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

### text

loadcsv
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

    virtual void setup() {}
    virtual void update() {}
    virtual void draw() {}

    void run();

  protected: // does it make sense to make these member functions public?
    void setWindow(size2d<pixels> _size)
    {
      // set the size of the window
    }

    void draw(rect _rect)
    {
      // draw a rectangle...
    }
    void draw(ellipse _ellipse)
    {
      //...
    }
    void draw(point _point)
    {
      //...
    }
    void draw(line _line)
    {
      //...
    }
    void draw(text _text)
    {
      //...
    }
    void draw(auto)
    {
      // unknow type...
    }
  };
} // namespace wit
```

one function to rule them all.
compile-time visitor

