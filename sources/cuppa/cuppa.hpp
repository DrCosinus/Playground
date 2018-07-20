#pragma once

#include "angle.hpp"
#include "unit.hpp"
#include "image.hpp"
#include "font.hpp"

#include "graphicsDriverInterface.hpp"
#include "platformDriverInterface.hpp"

#include <string_view>
#include <tuple>

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

    template<typename T>
    range(T min, T max) -> range<T>;

    class app
    {
    public:
        using Color = cuppa::Color;
        using Pixel = cuppa::Pixel;
        using Angle = cuppa::Angle;
        using Image = cuppa::Image;
        using Font = cuppa::Font;
        using Point = cuppa::Point;
        using Direction = cuppa::Direction;
        using Appliance = cuppa::Appliance;
        using ArcMode = cuppa::ArcMode;
        using TextHAlign = cuppa::TextHAlign;
        using TextVAlign = cuppa::TextVAlign;

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
        DeviceContext getDeviceContext() const { return platformDriver->getDeviceContext(); }
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

    // PREDEFINED COLORS (https://en.wikipedia.org/wiki/Web_colors#X11_color_names)
        static constexpr Color White{ 255 };
        static constexpr Color LightGray{ 211 };
        static constexpr Color Silver{ 192 };
        static constexpr Color DarkGray{ 169 };
        static constexpr Color Gray{ 128 };
        static constexpr Color DimGray{ 105 };
        static constexpr Color Black{ 0 };
        // PINK COLORS
        static constexpr Color Pink{ 255, 192, 203 };
        // RED COLORS
        static constexpr Color Red{ 255, 0, 0 };
        static constexpr Color DarkRed{ 139, 0, 0 };
        // ORANGE COLORS
        static constexpr Color OrangeRed{ 255, 69, 0 };
        static constexpr Color Coral{ 255, 127, 80 };
        static constexpr Color DarkOrange{ 255, 140, 0 };
        static constexpr Color Orange{ 255, 165, 0 };
        // YELLOW COLORS
        static constexpr Color Yellow{ 255, 255, 0 };
        static constexpr Color Gold{ 255, 215, 0 };
        // BROWN COLORS
        static constexpr Color Goldenrod{ 218, 165, 32 };
        static constexpr Color DarkGoldenrod{ 184, 134, 11 };
        static constexpr Color SaddleBrown{ 139, 69, 19 };
        static constexpr Color Sienna{ 160, 82, 45 };
        static constexpr Color Brown{ 165, 42, 42 };
        // GREEN COLORS
        static constexpr Color Lime{ 0, 255, 0 };
        static constexpr Color Green{ 0, 128, 0 };
        static constexpr Color DarkGreen{ 0, 100, 0 };
        // CYAN COLORS
        static constexpr Color Cyan{ 0, 255, 255 };
        static constexpr Color Aqua{ 0, 255, 255 };
        // BLUE COLORS
        static constexpr Color CornflowerBlue{ 100, 149, 237 };
        static constexpr Color Blue{ 0, 0, 255 };
        static constexpr Color MediumBlue{ 0, 0, 205 };
        static constexpr Color DarkBlue{ 0, 0, 139 };
        static constexpr Color Navy{ 0, 0, 128 };
        // PURPLE, VIOLET AND MAGENTA COLORS
        static constexpr Color Magenta{ 255, 0, 255 };
        static constexpr Color Fuchsia{ 255, 0, 255 };
        static constexpr Color DarkViolet{ 148, 0, 211 };
        static constexpr Color Purple{ 128, 0, 128 };
        static constexpr Color Indigo{ 75, 0, 130 };

    // environment
        void quit() const { platformDriver->quit(); }
        void size(Pixel _width, Pixel _height)
        {
            width = _width;
            height = _height;
            platformDriver->size(_width, _height);
        }

        const gamepadInterface& gamepad(std::size_t padIndex) const { return platformDriver->gamepad(padIndex); }

    // calculation
        // remap
        // Requirements:
        // - we must be able to substract a T from another T, it returns a DIFF_T
        // - a DIFF_T must be dividable by another DIFF_T and return a scalar (no unit)
        // - we must be able to substract a U from another U, it returns a DIFF_U
        // - a DIFF_U times a scalar must return a value to which be could add U and return a U
        template<typename U, typename T>
        static U remap(T value, range<T> from, range<U> to)
        {
            if constexpr (std::is_integral_v<T>)
                return to.min + to.width() * (static_cast<float>(value - from.min) / from.width());
            else
                return to.min + to.width() * ((value - from.min) / from.width());
        }
        template<typename T>
        static float remap(T value, range<T> from, range<float> to)
        {
            if constexpr (std::is_integral_v<T>)
                return to.min + to.width() * (static_cast<float>(value - from.min) / from.width());
            else
                return to.min + to.width() * ((value - from.min) / from.width());
        }
        template<typename... Us, typename T>
        static std::tuple<Us...> remap(T value, range<T> from, range<Us>... to)
        {
            auto f = (static_cast<float>(value - from.min) / from.width());
            auto g = [f](auto r) { return r.min + r.width() * f; };
            return { g(to)... };
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
        template<typename TUPLE, std::size_t... INDICES>
        void stroke_helper(TUPLE&& tuple, std::index_sequence<INDICES...>) const { stroke( std::get<INDICES>(std::forward<TUPLE>(tuple))... ); }
        template<typename TUPLE>
        void stroke(TUPLE&& tuple) const       {   stroke_helper( std::forward<TUPLE>(tuple), std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<TUPLE>>>{} ); }

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
        void line(Point pt, Direction dir)                                          const   {   graphicsDriver->line(pt, pt+dir);                       }
        void rect(Point center, Direction size)                                     const   {   graphicsDriver->rect(center, size);                     }
        void ellipse(Point center, Direction size)                                  const   {   graphicsDriver->ellipse( center, size );                }
        void ellipse(Point center, Pixel diameter)                                  const   {   ellipse( center, { diameter, diameter } );              }
        void arc(Point center, Direction size, Angle start, Angle end, ArcMode mode)const   {   graphicsDriver->arc( center, size, start, end, mode);   }
        void arc(Point center, Direction size, Angle start, Angle end)              const   {   arc(center, size, start, end, ArcMode::OPEN);           }
        void quad(Point pt1, Point pt2, Point pt3, Point pt4)                       const   {   graphicsDriver->quad( pt1, pt2, pt3, pt4);              }
        void triangle(Point pt1, Point pt2, Point pt3)                              const   {   graphicsDriver->triangle( pt1, pt2, pt3);               }

    // typography
        Font loadFont(std::string_view name, std::size_t size) const {  return graphicsDriver->loadFont(name, size);    }
        void textFont(const Font& font) const                        {  graphicsDriver->textFont(font);                 }
        Direction textSize(std::string_view txt) const               {  return graphicsDriver->textSize(txt);           }
        void text(std::string_view txt, Point pt, TextHAlign halign = TextHAlign::LEFT, TextVAlign valign = TextVAlign::TOP) const
        {   graphicsDriver->text( txt, pt, halign, valign); }

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