#include "cuppa.hpp"

#include <algorithm>
#include <string>

#pragma warning(disable: 4244)

struct sketch : cuppa::app
{
    using cuppa::app::app;

    Image imgArrow;
    Image imgDots;
    Pixel x = -8_px;
    Angle angle = 0_deg;
    std::size_t clickCount = 0;

    void setup() override
    {
        size(800_px, 600_px);
        imgArrow = loadImage("data/arrow.png");
        imgDots = loadImage("data/dots.png");
    }
    void update() override
    {
    }
    void mouseClick(Point) override
    {
        ++clickCount;
    }
    void draw() override
    {
        constexpr Direction translation = {180_px, 400_px};
        angle += 2_deg;
        background(Black);
        resetMatrix();
        translate({-148_px, -107_px}); // translate(image.getSize()*-0.5f)
        rotate(angle);
        scale(1+cosf(angle.ToRadian()*5)*0.1f);
        translate(translation);
        image(imgArrow, {0_px, 0_px});
        stroke(Magenta); // stroke(Magenta, 5_px);
        stroke(5_px);
        fill(Magenta.ModulateAlpha(64));
        rect({148_px, 107_px},{296_px, 214_px}); // translate(image.getSize()*0.5f, image.getSize())
        resetMatrix();

        rotate(angle);
        scale(1+cosf(angle.ToRadian()*5)*0.1f); // scale(1+cos(angle*5)*0.1f)
        translate(translation);
        point({0_px, 0_px});
        resetMatrix();

        image(imgDots, { 600_px, 400_px});

        if (x < getWidth()+8_px)
        {
            x = x + 4_px;
        }
        else
        {
            x = -8_px;
        }
        fill(Color{101});
        stroke(White);
        ellipse({x,150_px},16_px);

        fill(Green);
        ellipse({0_px,100_px},16_px);
        ellipse({getWidth(),100_px},16_px);
        fill( Red );
        ellipse({getWidth()-21_px,100_px},16_px);

        // random
        {
            stroke(1_px);
            static std::size_t pots[100] = {0};
            translate({ 250_px, 0_px});
            static float max = 0.0f;
            static float min = 1000.0f;
            static std::size_t count = 0;
            static std::size_t out_of_range_count = 0;

            pushMatrix();
            for (auto alpha = 0.0_deg; alpha < 360_deg; alpha += 2_deg)
            {
                float r = randomGaussian()/3.5*50+50;
                //float r = random(100);
                min = std::min(min, r);
                max = std::max(max, r);
                count++;
                resetMatrix();
                rotate(alpha);
                translate({ 150_px, 120_px});
                if (r<0 || r>=100)
                {
                    out_of_range_count++;
                    stroke(Red);
                    r = 200;
                }
                else
                {
                    stroke(White);
                    pots[(int)r]++;
                }
                line({ 0_px, 0_px }, { Pixel(r), 0_px });
            }
            popMatrix();

            resetMatrix();
            noStroke(); // stroke(false);
            fill(Yellow);
            translate({350_px, 250_px});
            auto pot_max = *std::max_element(std::begin(pots), std::end(pots));
            for (int i = 0; i < 100; ++i)
            {
                auto v = 200.0f * pots[i] / pot_max + 1.0f;
                rect({i*4_px, Pixel(-v*0.5f)}, {4_px, Pixel(v)});
            }
            text(std::to_string(min), { 0_px, 0_px });
            text(std::to_string(max), { 0_px, 20_px });
            text(std::to_string(out_of_range_count*100.0f/count), { 0_px, 40_px });
        }

        resetMatrix();
        fill(Yellow);
        text(std::to_string(clickCount), { 0_px, 40_px });
    }
};

int main(void)
{
    auto app = sketch{};
    app.run();
}