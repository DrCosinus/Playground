#pragma once

namespace cuppa
{
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

        void stroke(unsigned int _red, unsigned int _green, unsigned int _blue, unsigned int _alpha);
        void stroke(unsigned int _red, unsigned int _green, unsigned int _blue)     {   stroke(_red, _green, _blue, 255);   }
        void stroke(unsigned int _gray)                                             {   stroke(_gray, _gray, _gray, 255);   }

        void strokeWeight(float _thickness);

        void fill(unsigned int _red, unsigned int _green, unsigned int _blue, unsigned int _alpha);
        void fill(unsigned int _red, unsigned int _green, unsigned int _blue)   {   fill(_red, _green, _blue, 255); }
        void fill(unsigned int _gray)                                           {   fill(_gray, _gray, _gray, 255); }

        void rectangle(int _centerX, int _centerY, unsigned int _width, unsigned int _height);

        void ellipse(int _centerX, int _centerY, unsigned int _width, unsigned int _height) const;
        void ellipse(int _centerX, int _centerY, unsigned int _diameter)        {   ellipse( _centerX, _centerY, _diameter, _diameter ); }

        void text(const char* c, int x, int y);

        void point(int x, int y);

        void line(int x1, int y1, int x2, int y2);
    };
} // namespace wit