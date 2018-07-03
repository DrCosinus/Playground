#pragma once

#include "unit.hpp"
#include "image.hpp"

#include <string_view>

namespace cuppa
{
    class app
    {
    public:
        using Color = cuppa::Color;
        using Pixel = cuppa::Pixel;
        using Image = cuppa::Image;

        enum struct ArcMode{ PIE, OPEN, CHORD };

        app() = default;
        virtual ~app() = default;

        virtual void setup() {}
        virtual void update() {}
        virtual void draw() {}

        void run();

    protected:
        static constexpr float PI = 3.1415926535897932384626433832795f;

        static constexpr Color White{ 255 };
        static constexpr Color Black{ 0 };
        static constexpr Color Red{ 255, 0, 0 };
        static constexpr Color Green{ 0, 255, 0 };
        static constexpr Color Blue{ 0, 0, 255 };
        static constexpr Color Magenta{ 255, 0, 255 };
        static constexpr Color Cyan{ 0, 255, 255 };
        static constexpr Color Yellow{ 255, 255, 0 };
        static constexpr Color Orange{ 255, 170, 85 };

    // environment
        void size(Pixel _width, Pixel _height);
        Pixel getWidth() const;
        Pixel getHeight() const;

    // colors: settings
        void background(Color color);

        void noStroke();
        void stroke(Color color);
        void stroke(Pixel _thickness);

        void noFill();
        void fill(Color color);

    // shapes: 2D primitives
        void point(Point2D pt);
        void line(Point2D pt1, Point2D pt2);
        void rect(Point2D center, Move2D size);
        void ellipse(Point2D center, Move2D size) const;
        void ellipse(Point2D center, Pixel diameter)        {   ellipse( center, { diameter, diameter } ); }
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

    // image
        Image loadImage(std::string_view filename);
    };
} // namespace wit