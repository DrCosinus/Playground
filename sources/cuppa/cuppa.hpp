#pragma once

#include "unit.hpp"
#include "image.hpp"

#include "graphicsDriverInterface.hpp"
#include "platformDriverInterface.hpp"

#include <string_view>

namespace cuppa
{
    void Assert(bool _condition, const char* _message);
    inline void Assert(bool _condition) { Assert(_condition, "Assert without description"); }

    template<typename T>
    struct range
    {
        T min;
        T max;
        auto width() const { return max - min; }
    };

    class app
    {
    public:
        using Color = cuppa::Color;
        using Pixel = cuppa::Pixel;
        using Angle = cuppa::Angle;
        using Image = cuppa::Image;
        using Point2D = cuppa::Point2D;
        using Move2D = cuppa::Move2D;

        app() = default;
        app(app&&) = default;
        virtual ~app() = default;

        virtual void setup() {}
        virtual void update() {}
        virtual void draw() {}

        void run();

    // environment
        Pixel getWidth() const { return platformDriver->getWidth(); }
        Pixel getHeight() const { return platformDriver->getHeight(); }

        std::unique_ptr<graphicsDriverInterface> graphicsDriver;
        std::unique_ptr<platformDriverInterface> platformDriver;
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
        void size(Pixel _width, Pixel _height)  const   { platformDriver->size(_width, _height); }

    // calculation
        static float remap(float value, range<float> from, range<float> to)
        {
            return (value - from.min) * to.width() / from.width() + to.min;
        }

    // random
        // returns a flot between 0 and 1 (1 not included)
        float random()                      const;
        float random(float high)            const   { return high * random(); }
        float random(float low, float high) const   { return low + ( high - low ) * random(); }

        // returns a float from a series of number having a mean of 0 and standard deviation of 1
        // - 68%   of numbers are between -1 and 1
        // - 95%   of numbers are between -2 and 2
        // - 99.8% of numbers are between -3 and 3
        float randomGaussian() const;

    // colors: settings
        void background(Color color)                const   {   graphicsDriver->background(color);  }

        void noStroke()                             const   {   graphicsDriver->noStroke();         }
        void stroke(Color color)                    const   {   graphicsDriver->stroke(color);      }
        void stroke(Pixel _thickness)               const   {   graphicsDriver->stroke(_thickness); }
        //void pushStroke()
        //void popStroke()
        //void resetStroke()

        void noFill()                               const   {   graphicsDriver->noFill();           }
        void fill(Color color)                      const   {   graphicsDriver->fill(color);        }
        //void pushFill()
        //void popFill()
        //void resetFill()

    // shapes: 2D primitives
        void point(Point2D pt)                                                      const   {   graphicsDriver->point(pt);                              }
        void line(Point2D pt1, Point2D pt2)                                         const   {   graphicsDriver->line(pt1, pt2);                         }
        void rect(Point2D center, Move2D size)                                      const   {   graphicsDriver->rect(center, size);                     }
        void ellipse(Point2D center, Move2D size)                                   const   {   graphicsDriver->ellipse( center, size );                }
        void ellipse(Point2D center, Pixel diameter)                                const   {   ellipse( center, { diameter, diameter } );              }
        void arc(Point2D center, Move2D size, Angle start, Angle end, ArcMode mode) const   {   graphicsDriver->arc( center, size, start, end, mode);   }
        void arc(Point2D center, Move2D size, Angle start, Angle end)               const   {   arc(center, size, start, end, ArcMode::OPEN);           }
        void quad(Point2D pt1, Point2D pt2, Point2D pt3, Point2D pt4)               const   {   graphicsDriver->quad( pt1, pt2, pt3, pt4);              }
        void triangle(Point2D pt1, Point2D pt2, Point2D pt3)                        const   {   graphicsDriver->triangle( pt1, pt2, pt3);               }
        void text(const char* c, Point2D pt)                                        const   {   graphicsDriver->text( c, pt);                           }

    // transforms
        void pushMatrix()   const   { graphicsDriver->pushMatrix();  }
        void popMatrix()    const   { graphicsDriver->popMatrix();   }
        void resetMatrix()  const   { graphicsDriver->resetMatrix(); }

        void translate(Move2D translation)  const   {   graphicsDriver->translate(translation); }

        void scale(float xscale, float yscale, float zscale)    const   {   graphicsDriver->scale(xscale, yscale, zscale);  }
        void scale(float xscale, float yscale)                  const   {   scale(xscale, yscale, 1.0f);                    }
        void scale(float _scale)                                const   {   scale(_scale, _scale, _scale);                  }

        void rotate(Angle angle)    const   {   graphicsDriver->rotate(angle);  }

        void shearX(float slope)    const   { graphicsDriver->shearX(slope);    }
        void shearY(float slope)    const   { graphicsDriver->shearY(slope);    }

    // image
        Image loadImage(std::string_view filename)  const   {   return graphicsDriver->loadImage(filename); }
        void image(const Image& img, Point2D pt)    const   {   graphicsDriver->image(img, pt);             }
    };
} // namespace wit