#include "cuppa.hpp"

#include <iostream>

struct sketch : cuppa::app
{
    using cuppa::app::app;

    Image imgArrow;
    Image imgDots;
    Pixel x = 0_px;
    Angle angle = 0_deg;

    void setup() override
    {
        size(800_px, 600_px);
        imgArrow = loadImage("data/arrow.png");
        imgDots = loadImage("data/dots.png");
    }
    void update() override
    {
    }
    void draw() override
    {
        constexpr Move2D position = {150_px, 350_px};
        angle += 2_deg;
        background(Black);
        resetMatrix();
        translate({-148_px, -107_px});
        rotate(angle);
        scale(1+cosf(angle.ToRadian()*5)*0.1f);
        translate(position);
        image(imgArrow, {0_px, 0_px});
        stroke(Magenta);
        stroke(5_px);
        fill(Magenta.ModulateAlpha(64));
        rect({148_px, 107_px},{296_px, 214_px});
        resetMatrix();

        rotate(angle);
        scale(1+cosf(angle.ToRadian()*5)*0.1f);
        translate(position);
        point({0_px, 0_px});
        resetMatrix();

        image(imgDots, { 600_px, 400_px});
        image(imgDots, { 0_px, 0_px});

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

        stroke(1_px);
        translate({ 250_px, 0_px});
        for (auto y = 0_px; y < 100_px; ++y)
        {
            float r = random(50);
            line({ 0_px, y }, { Pixel(r), y });
        }
    }
};

int main(void)
{
    auto app = sketch{};
    app.run();
}