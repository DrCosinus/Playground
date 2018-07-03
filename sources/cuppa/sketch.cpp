#include "cuppa.hpp"

#include <iostream>

struct sketch : cuppa::app
{
    using cuppa::app::app;

    Image image;
    Pixel x = 0_px;

    void setup() override
    {
        size(800_px, 600_px);
        //image = loadImage("test.png");
    }
    void update() override
    {
    }
    void draw() override
    {
        background(Black);

        if (x < getWidth())
        {
            x = x + 4_px;
        }
        fill(Color{101});
        stroke(White);
        ellipse({x,150_px},16_px);

        fill(Green);
        ellipse({0_px,100_px},16_px);
        ellipse({getWidth(),100_px},16_px);
        fill( Red );
        ellipse({getWidth()-21_px,100_px},16_px);
    }
};

int main(void)
{
    auto app = sketch{};
    app.run();
}