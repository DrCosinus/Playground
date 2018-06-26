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
        stroke(0);
        strokeWeight(5);
        fill(255,0,0);
        rectangle(50,50,50,50);

        strokeWeight(2);
        stroke(0,0,255);
        fill(255,255,0, 127);
        ellipse(75,75,20,20);

        fill(0);
        text("plop",200,10);
    }
};

int main(void)
{
    auto app = sketch{};
    app.run();
}