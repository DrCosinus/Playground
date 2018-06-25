#include "cuppa.hpp"

#include <iostream>

struct sketch : cuppa::app
{
    using cuppa::app::app;

    void setup() override
    {
        size(800, 600);
    }
    void update() override
    {
    }
    void draw() override
    {
        //stroke(thickness{2_px}, color{0});
        //fill(color{255});
        //draw(line{point{0,0}, point{100,100}});
    }
};

int main(void)
{
    auto app = sketch{};
    app.run();
}