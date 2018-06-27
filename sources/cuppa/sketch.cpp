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
        rectangle(30,30,50,50);

        strokeWeight(2);
        stroke(0,0,255);
        fill(255,255,0, 127);
        ellipse(55,55,20,20);

        fill(0);
        text("plop",95,2);

        strokeWeight(0.2f);
        stroke(255,0,255);
        line(5,5,55,55);
        line(5,55,55,5);

        stroke(0);
        strokeWeight(8);
        point(30, 30);

        strokeWeight(1);
        fill(255,0,255,64);
        arc(50,75,25,25, 0, PI + QUARTER_PI, ArcMode::OPEN);
        arc(80,75,25,25, 0, PI + QUARTER_PI, ArcMode::CHORD);
        arc(110,75,25,25, 0, PI + QUARTER_PI, ArcMode::PIE);
    }
};

int main(void)
{
    auto app = sketch{};
    app.run();
}