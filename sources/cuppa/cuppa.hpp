#pragma once

namespace cuppa
{
    struct pixels
    {
        int value;
    };

    template<typename UNIT>
    struct point2d
    {
        UNIT w, d;
    };

    template<typename UNIT>
    struct size2d
    {
        UNIT w, d;
    };

    template<typename UNIT>
    struct rect
    {
        point2d<UNIT> center;
        size2d<UNIT> size;
    };

    template<typename UNIT>
    struct ellipse
    {
        point2d<UNIT> center;
        size2d<UNIT> size;
    };

    template<typename UNIT>
    struct line2d
    {
        point2d<UNIT> start, end;
    };

    struct text
    {
        //...
    };

    struct image
    {
        //...
    };

    template<typename T>
    T clamp(T v, T min, T max)
    {
        if (v<min)
            return min;
        if (v>max)
            return max;
        return v;
    }

    struct color
    {
        using value_type = unsigned char;
        value_type r, g, b;
        // discreet 8bits components 0 to 255
        color(value_type _red, value_type _green, value_type _blue) : r{ _red }, g{ _green }, b { _blue } {}
        color(value_type _gray) : r{ _gray }, g{ _gray }, b { _gray } {}
        // continuous value default visible range is 0.0 to 1.0
        color(float _intensity) : color{ clamp( static_cast<value_type>( _intensity * 255.999f ), value_type{ 0 }, value_type{ 255 }) } {}
    };

    class app
    {
    public:
      app() = default;
      virtual ~app() = default;

      virtual void setup() {}
      virtual void update() {}
      virtual void draw() {}
  
      void run();

    protected:
        void size(unsigned int _width, unsigned int _height);
    // does it make sense to make these member functions public?
        template<typename UNIT>
        void setWindow(size2d<UNIT> /*_size*/)
        {
        // set the size of the window
        }

        template<typename UNIT>
        void draw(rect<UNIT> /*_rect*/)
        {
        // draw a rectangle...
        }

        template<typename UNIT>
        void draw(ellipse<UNIT> /*_ellipse*/)
        {
        //...
        }

        template<typename UNIT>
        void draw(point2d<UNIT> /*_point*/)
        {
        //...
        }

        template<typename UNIT>
        void draw(line2d<UNIT> /*_line*/)
        {
        //...
        }

        template<typename UNIT>
        void draw(text /*_text*/, point2d<UNIT> /*_pos*/)
        {
        //...
        }

        template<typename UNIT>
        void draw(image /*_image*/, point2d<UNIT> /*_pos*/, size2d<UNIT> /*_size*/)
        {
        //...
        }

        template<typename ANY>
        void draw(ANY)
        {
        // unknow type...
        }
    };
} // namespace wit