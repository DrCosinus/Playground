#pragma once

#include "angle.hpp"
#include "unit.hpp"
#include "image.hpp"
#include "font.hpp"

#include "graphicsDriverInterface.hpp"
#include "platformDriverInterface.hpp"

#include <chrono>
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

    inline std::unique_ptr<graphicsDriverInterface> graphicsDriver;
    inline std::unique_ptr<platformDriverInterface> platformDriver;

    // environment
    inline DeviceContext getDeviceContext() { return platformDriver->getDeviceContext(); }
    inline void quit() { platformDriver->quit(); }
    inline const gamepadInterface& gamepad(std::size_t padIndex)  { return platformDriver->gamepad(padIndex); }
    inline bool isDigit(char c) { return c >= '0' && c <= '9'; }
    inline bool isLower(char c) { return ( c >= 'a' && c <= 'z' ); }
    inline bool isUpper(char c) { return ( c >= 'A' && c <= 'Z' ); }
    inline bool isAlpha(char c) { return isLower(c) || isUpper(c); }
    inline bool isAlphaNum(char c) { return isAlpha(c) || isDigit(c); }

    // time & date
    inline std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    inline auto markTime() { startTime = std::chrono::high_resolution_clock::now(); }
    inline auto getMillisFromMark() { return std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now() - startTime).count(); }

    // typography
    inline Font loadFont(std::string_view name, std::size_t size) {  return graphicsDriver->loadFont(name, size);    }
    inline void textFont(const Font& font)                        {  graphicsDriver->textFont(font);                 }
    inline Direction textSize(std::string_view txt)               {  return graphicsDriver->textSize(txt);           }
    inline void text(std::string_view txt, Point pt, TextHAlign halign = TextHAlign::LEFT, TextVAlign valign = TextVAlign::TOP)
    {   graphicsDriver->text( txt, pt, halign, valign); }

    // colors: settings
    inline void background(Color color)                   {   graphicsDriver->background(color);  }

    inline void stroke(Appliance appliance)               {   graphicsDriver->stroke(appliance);  }
    inline void stroke(Color color)                       {   graphicsDriver->stroke(color);      }
    inline void stroke(Pixel thickness)                   {   graphicsDriver->stroke(thickness);  }
    template<typename HEAD_TYPE, typename... TAIL_TYPES>
    inline void stroke(HEAD_TYPE first, TAIL_TYPES... others)  
    {
        stroke(first);
        if constexpr(sizeof...(TAIL_TYPES)!=0)
        {
            stroke(others...);
        }
    }
    template<typename TUPLE, std::size_t... INDICES>
    inline void stroke_helper(TUPLE&& tuple, std::index_sequence<INDICES...>) { stroke( std::get<INDICES>(std::forward<TUPLE>(tuple))... ); }
    template<typename TUPLE>
    inline void stroke(TUPLE&& tuple)       {   stroke_helper( std::forward<TUPLE>(tuple), std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<TUPLE>>>{} ); }

    //void pushStroke()
    //void popStroke()
    //void resetStroke()

    inline void noFill()            {   graphicsDriver->noFill();       }
    inline void fill(Color color)   {   graphicsDriver->fill(color);    }
    //void pushFill()
    //void popFill()
    //void resetFill()

    // shapes: 2D primitives
    inline void point(Point pt)                                                         {   graphicsDriver->point(pt);                              }
    inline void line(Point pt1, Point pt2)                                              {   graphicsDriver->line(pt1, pt2);                         }
    inline void line(Point pt, Direction dir)                                           {   graphicsDriver->line(pt, pt+dir);                       }
    inline void rect(Point center, Direction size)                                      {   graphicsDriver->rect(center, size);                     }
    inline void ellipse(Point center, Direction size)                                   {   graphicsDriver->ellipse( center, size );                }
    inline void ellipse(Point center, Pixel diameter)                                   {   ellipse( center, { diameter, diameter } );              }
    inline void arc(Point center, Direction size, Angle start, Angle end, ArcMode mode) {   graphicsDriver->arc( center, size, start, end, mode);   }
    inline void arc(Point center, Direction size, Angle start, Angle end)               {   arc(center, size, start, end, ArcMode::OPEN);           }
    inline void quad(Point pt1, Point pt2, Point pt3, Point pt4)                        {   graphicsDriver->quad( pt1, pt2, pt3, pt4);              }
    inline void triangle(Point pt1, Point pt2, Point pt3)                               {   graphicsDriver->triangle( pt1, pt2, pt3);               }
    inline void beginShape()                                                            {   graphicsDriver->beginShape();                           }
    inline void endShape()                                                              {   graphicsDriver->endShape();                             }
    inline void vertex(Point pt)                                                        {   graphicsDriver->vertex(pt);                             }

    // transforms
    inline void pushMatrix()    { graphicsDriver->pushMatrix();  }
    inline void popMatrix()     { graphicsDriver->popMatrix();   }
    inline void resetMatrix()   { graphicsDriver->resetMatrix(); }

    inline void translate(Direction translation)    {   graphicsDriver->translate(translation); }

    inline void scale(float xscale, float yscale, float zscale) {   graphicsDriver->scale(xscale, yscale, zscale);  }
    inline void scale(float xscale, float yscale)               {   scale(xscale, yscale, 1.0f);                    }
    inline void scale(float _scale)                             {   scale(_scale, _scale, _scale);                  }

    inline void rotate(Angle angle) {   graphicsDriver->rotate(angle);  }

    inline void shearX(float slope) { graphicsDriver->shearX(slope);    }
    inline void shearY(float slope) { graphicsDriver->shearY(slope);    }

    // random
    // returns a flot between 0 and 1 (1 not included)
    float random();
    inline float random(float high)             { return high * random(); }
    inline float random(float low, float high)  { return low + ( high - low ) * random(); }
    float noise(float x);
    float noise(float x, float y);

    // returns a float from a series of number having a mean of 0 and standard deviation of 1
    // - 68%   of numbers are between -1 and 1
    // - 95%   of numbers are between -2 and 2
    // - 99.8% of numbers are between -3 and 3
    float randomGaussian();

    // image
    inline Image loadImage(std::string_view filename)  {   return graphicsDriver->loadImage(filename); }
    inline void image(const Image& img, Point pt)      {   graphicsDriver->image(img, pt);             }

    // sound
    inline void beep(int frequency, int duration)       { platformDriver->beep(frequency, duration); }
    inline void systemBeep(int type)                    { platformDriver->systemBeep(type);          }

    // calculation
    inline constexpr float PI = 3.1415926535897932384626433832795f;
    // remap
    // Requirements:
    // - we must be able to substract a T from another T, it returns a DIFF_T
    // - a DIFF_T must be dividable by another DIFF_T and return a scalar (no unit)
    // - we must be able to substract a U from another U, it returns a DIFF_U
    // - a DIFF_U times a scalar must return a value to which be could add U and return a U
    template<typename U, typename T>
    inline U remap(T value, range<T> from, range<U> to)
    {
        if constexpr (std::is_integral_v<T>)
            return to.min + to.width() * (static_cast<float>(value - from.min) / from.width());
        else
            return to.min + to.width() * ((value - from.min) / from.width());
    }
    template<typename T>
    inline float remap(T value, range<T> from, range<float> to)
    {
        if constexpr (std::is_integral_v<T>)
            return to.min + to.width() * (static_cast<float>(value - from.min) / from.width());
        else
            return to.min + to.width() * ((value - from.min) / from.width());
    }
    template<typename... Us, typename T>
    inline std::tuple<Us...> remap(T value, range<T> from, range<Us>... to)
    {
        auto f = (static_cast<float>(value - from.min) / from.width());
        auto g = [f](auto r) { return r.min + r.width() * f; };
        return { g(to)... };
    }
    // Requierements:
    // - we must be able to substract a T from another T, it returns a DIFF_T
    // - a DIFF_T times a scalar must return a DIFF_T
    // - a T plus a DIFF_T must return a T
    // - alpha should ne in the reange [0, 1] but it is not checked
    template<typename T>
    inline T lerp(T begin, T end, float alpha)
    {
        return begin + alpha * (end-begin);
    }

    class app
    {
    public:
        app() = default;
        app(app&&) = default;
        virtual ~app() = default;

        virtual void setup() {}
        virtual void update() {}
        virtual void draw() {}
        virtual void mouseClick(Point) {}
        virtual void keyUp(short) {}
        virtual void keyDown(short) {}
        virtual void keyChar(char) {}

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

    protected:

    // environment
        void size(Pixel _width, Pixel _height)
        {
            width = _width;
            height = _height;
            platformDriver->size(_width, _height);
        }
    private:
        Pixel width = 240_px, height = 120_px;
    };
} // namespace wit