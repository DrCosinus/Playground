# Cuppa

C++ prototyping facilities (simple media management). Windows specific until further notice.

## Thinkings

### text utilities

loadcsv
loadjson
loadxml

### math utilities

Vector2d, Vector3d
Matrix2d (Rows by Columns)

#### Linear algebra

W * X = Y

[[ w11 w12]    [[x1]     [[w11*x1+w12*x2]    [[y1]
 [ w21 w22]] *  [x2]]  =  [w21*x1+w22*x2]] =  [y2]]

### Neural network Facilities

### minimal media interface (graphics, inputs,...)

### brainstorming a use case

```cpp
namespace cuppa
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

[Back to README](../../README.md)