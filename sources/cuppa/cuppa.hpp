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