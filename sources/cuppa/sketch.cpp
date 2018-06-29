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
        background(255);
        scale(6);

        stroke(0);
        strokeWeight(5);
        fill(255,0,0);
        rect({ 30_m, 30_m }, { 50_m, 50_m });

        strokeWeight(2);
        stroke(0,0,255);
        fill(255,255,0, 127);
        ellipse({ 55_m, 55_m }, 20_m);

        fill(0);
        text("plop",95,2);
        char buff[] = { '0', '\0' };
        static int count = 0;
        buff[0] = count % 10 + '0';
        ++count;
        text(buff,2,62);

        strokeWeight(0.2f);
        stroke(255,0,255);
        line({ 5_m, 5_m}, { 55_m,55_m});
        line({ 5_m, 55_m}, { 55_m, 5_m});

        stroke(0);
        strokeWeight(8);
        point({30_m, 30_m});

        strokeWeight(1);
        fill(255,0,255,64);
        arc({ 50_m, 75_m }, { 25_m, 25_m }, 0_turn, 1.25_turn, ArcMode::OPEN);
        arc({ 80_m, 75_m }, { 25_m, 25_m }, 0_turn, 1.25_turn, ArcMode::CHORD);
        arc({110_m, 75_m }, { 25_m, 25_m }, 0_turn, 1.25_turn, ArcMode::PIE);

        noFill();
        quad(78, 31, 126, 20, 109, 53, 70, 66);

        fill(255,170,85,64);
        noStroke();
        fill(255,170,85,64);
        triangle(30, 75, 58, 20, 86, 75);
/*
        stroke(0);
        fill(255);
        rect(30+25, 20+25, 50, 50);
        scale(0.5);
        rect(30+25, 20+25, 50, 50);

        static float angle = 0.0f;
        shearY(QUARTER_PI);
        rotate(angle);
        angle += 0.03f;
        translate(300,300);
        rect(0,0,100,100);*/
    }
};

int main(void)
{
    auto app = sketch{};
    app.run();
}