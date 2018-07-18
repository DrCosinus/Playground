#pragma once

#include "angle.hpp"
#include "image.hpp"
#include "unit.hpp"

#include <string_view>

namespace cuppa
{
    class app;
    enum struct ArcMode{ PIE, OPEN, CHORD };
    enum struct Appliance{ ENABLED, DISABLED };
    enum struct TextHAlign{ LEFT, CENTER, RIGHT };
    enum struct TextVAlign{ TOP, MIDDLE, BOTTOM };

    struct graphicsDriverInterface
    {
        virtual ~graphicsDriverInterface() = default;

        virtual void setup() = 0;
        virtual void draw(app& _app, DeviceContext _dc) = 0;

        virtual void background(Color color) = 0;
        virtual void stroke(Appliance) = 0;
        virtual void stroke(Color color) = 0;
        virtual void stroke(Pixel _thickness) = 0;
        virtual void noFill() = 0;
        virtual void fill(Color color) = 0;
        virtual void point(Point pt) = 0;
        virtual void line(Point pt1, Point pt2) = 0;
        virtual void rect(Point center, Direction size) = 0;
        virtual void ellipse(Point center, Direction size) = 0;
        virtual void arc(Point center, Direction size, Angle start, Angle end, ArcMode mode) = 0;
        virtual void quad(Point pt1, Point pt2, Point pt3, Point pt4) = 0;
        virtual void triangle(Point pt1, Point pt2, Point pt3) = 0;
        virtual void text(std::string_view txt, Point pt, TextHAlign halign = TextHAlign::LEFT, TextVAlign valign = TextVAlign::TOP) = 0;

        virtual void resetMatrix() = 0;
        virtual void pushMatrix() = 0;
        virtual bool popMatrix() = 0;
        virtual void translate(Direction translation) = 0;
        virtual void rotate(Angle angle) = 0;
        virtual void scale(float xscale, float yscale, float) = 0;
        virtual void shearX(float slope) = 0;
        virtual void shearY(float slope) = 0;

        virtual Image loadImage(std::string_view filename) = 0;
        virtual void image(const Image& img, Point pt) = 0;
    };
}