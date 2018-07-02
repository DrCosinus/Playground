#include "cuppa.hpp"

#include <iostream>

struct sketch : cuppa::app
{
    using cuppa::app::app;

    //Image image;
    Meter x = 0_m;

    void setup() override
    {
        size(800_m, 600_m);
        //image = loadImage();
    }
    void update() override
    {
    }
    void draw() override
    {
        background(Black);

        if (x < getWidth())
        {
            x = x + 4_m;
        }
        fill(Color{101});
        stroke(White);
        ellipse({x,150_m},16_m);
    }
};

int main(void)
{
    auto app = sketch{};
    app.run();
}