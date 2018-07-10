#pragma once

#include "angle.hpp"
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
        using Point = cuppa::Point;
        using Direction = cuppa::Direction;
        using Appliance = cuppa::Appliance;
        using ArcMode = cuppa::ArcMode;

        app() = default;
        app(app&&) = default;
        virtual ~app() = default;

        virtual void setup() {}
        virtual void update() {}
        virtual void draw() {}
        virtual void mouseClick(Point) {}

        void setMousePosition(Point position) { previousMousePosition = mousePosition; mousePosition = position; }
        Point mousePosition;
        Point previousMousePosition;
        void run();

    // environment
        Pixel getWidth() const { return width; }
        Pixel getHeight() const { return height; }
        void setSize(Direction newSize)
        {
            width = newSize.width;
            height = newSize.height;
        }

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
        void size(Pixel _width, Pixel _height)
        {
            width = _width;
            height = _height;
            platformDriver->size(_width, _height);
        }

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

        void stroke(Appliance appliance)            const   {   graphicsDriver->stroke(appliance);  }
        void stroke(Color color)                    const   {   graphicsDriver->stroke(color);      }
        void stroke(Pixel thickness)                const   {   graphicsDriver->stroke(thickness);  }
        template<typename HEAD_TYPE, typename... TAIL_TYPES>
        void stroke(HEAD_TYPE first, TAIL_TYPES... others)  const
        {
            stroke(first);
            if constexpr(sizeof...(TAIL_TYPES)!=0)
            {
                stroke(others...);
            }
        }

        //void pushStroke()
        //void popStroke()
        //void resetStroke()

        void noFill()                               const   {   graphicsDriver->noFill();           }
        void fill(Color color)                      const   {   graphicsDriver->fill(color);        }
        //void pushFill()
        //void popFill()
        //void resetFill()

    // shapes: 2D primitives
        void point(Point pt)                                                        const   {   graphicsDriver->point(pt);                              }
        void line(Point pt1, Point pt2)                                             const   {   graphicsDriver->line(pt1, pt2);                         }
        void rect(Point center, Direction size)                                     const   {   graphicsDriver->rect(center, size);                     }
        void ellipse(Point center, Direction size)                                  const   {   graphicsDriver->ellipse( center, size );                }
        void ellipse(Point center, Pixel diameter)                                  const   {   ellipse( center, { diameter, diameter } );              }
        void arc(Point center, Direction size, Angle start, Angle end, ArcMode mode)const   {   graphicsDriver->arc( center, size, start, end, mode);   }
        void arc(Point center, Direction size, Angle start, Angle end)              const   {   arc(center, size, start, end, ArcMode::OPEN);           }
        void quad(Point pt1, Point pt2, Point pt3, Point pt4)                       const   {   graphicsDriver->quad( pt1, pt2, pt3, pt4);              }
        void triangle(Point pt1, Point pt2, Point pt3)                              const   {   graphicsDriver->triangle( pt1, pt2, pt3);               }
        void text(std::string_view txt, Point pt)                                   const   {   graphicsDriver->text( txt, pt);                         }

    // transforms
        void pushMatrix()   const   { graphicsDriver->pushMatrix();  }
        void popMatrix()    const   { graphicsDriver->popMatrix();   }
        void resetMatrix()  const   { graphicsDriver->resetMatrix(); }

        void translate(Direction translation)   const   {   graphicsDriver->translate(translation); }

        void scale(float xscale, float yscale, float zscale)    const   {   graphicsDriver->scale(xscale, yscale, zscale);  }
        void scale(float xscale, float yscale)                  const   {   scale(xscale, yscale, 1.0f);                    }
        void scale(float _scale)                                const   {   scale(_scale, _scale, _scale);                  }

        void rotate(Angle angle)    const   {   graphicsDriver->rotate(angle);  }

        void shearX(float slope)    const   { graphicsDriver->shearX(slope);    }
        void shearY(float slope)    const   { graphicsDriver->shearY(slope);    }

    // image
        Image loadImage(std::string_view filename)  const   {   return graphicsDriver->loadImage(filename); }
        void image(const Image& img, Point pt)      const   {   graphicsDriver->image(img, pt);             }
    private:
        Pixel width = 240_px, height = 120_px;
    };
} // namespace wit