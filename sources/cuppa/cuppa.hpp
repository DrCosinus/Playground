#pragma once

#include "unit.hpp"

namespace cuppa
{
    class app
    {
    public:
        static constexpr float QUARTER_PI = 0.78539816339744830961566084581988f;
        static constexpr float HALF_PI = 1.5707963267948966192313216916398f;
        static constexpr float PI = 3.1415926535897932384626433832795f;
        static constexpr float TWO_PI = 6.283185307179586476925286766559f;

        enum struct ArcMode{ PIE, OPEN, CHORD };

        using Color = cuppa::Color;

        app() = default;
        virtual ~app() = default;

        virtual void setup() {}
        virtual void update() {}
        virtual void draw() {}

        void run();

    protected:
    // environment
        void size(unsigned int _width, unsigned int _height);

    // colors: settings
        void background(Color color);

        void noStroke();
        void stroke(Color color);
        void stroke(Meter _thickness);

        void noFill();
        void fill(Color color);

    // shapes: 2D primitives
        void point(Point2D pt);
        void line(Point2D pt1, Point2D pt2);
        void rect(Point2D center, Move2D size);
        void ellipse(Point2D center, Move2D size) const;
        void ellipse(Point2D center, Meter diameter)        {   ellipse( center, { diameter, diameter } ); }
        void arc(Point2D center, Move2D size, Angle start, Angle end, ArcMode mode);
        void arc(Point2D center, Move2D size, Angle start, Angle end)   { arc(center, size, start, end, ArcMode::OPEN); }
        void quad(Point2D pt1, Point2D pt2, Point2D pt3, Point2D pt4);
        void triangle(Point2D pt1, Point2D pt2, Point2D pt3);

        void text(const char* c, int x, int y);

    // transforms
        void pushMatrix();
        void popMatrix();
        void resetMatrix();

        void translate(float xmove, float ymove, float zmove);
        void translate(float xscale, float yscale)                  { translate(xscale, yscale, 1.0f); }
        void translate(float _scale)                                { translate(_scale, _scale, _scale); }

        void scale(float xscale, float yscale, float zscale);
        void scale(float xscale, float yscale)                  { scale(xscale, yscale, 1.0f); }
        void scale(float _scale)                                { scale(_scale, _scale, _scale); }

        void rotate(float angle);

        void shearX(float angle);
        void shearY(float angle);
    };
} // namespace wit