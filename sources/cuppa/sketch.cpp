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
    struct MouseTracker
    {
        static constexpr std::size_t capacity = 15;
        void push(Point pt)
        {
            history[nextIndex] = pt;
            nextIndex = (nextIndex+1) % capacity;
            if (count+1 < capacity)
                count++;
        }
        void draw(sketch& s) const
        {
            for(std::size_t i = 0; i+1 < count; ++i)
            {
                s.stroke(s.remap<Pixel, Color>(i, { 0, count-1 }, { 5_px, 2_px }, { Yellow, Red.ModulateAlpha(64) }));
                // auto[pixel, color] = s.remap<Pixel, Color>(i, { 0, count-1 }, { 5_px, 2_px }, { Yellow, Red.ModulateAlpha(64) });
                // s.stroke(pixel, color);
                //s.stroke(s.remap<Pixel>(i, { 0, count-1 }, { 5_px, 2_px }), s.remap<Color>(i, { 0, count-1 }, { Yellow, Red.ModulateAlpha(64) }));
                s.line(history[(nextIndex-i-1+capacity*2)%capacity], history[(nextIndex-i-2+capacity*2)%capacity]);
            }
        }
    private:
        Point history[capacity];
        std::size_t nextIndex = 0;
        std::size_t count = 0;
    } mouseTracker;
    void draw() override
    {
        background(Black);
        stroke(5_px, White);
        line(previousMousePosition, mousePosition); // TODO: handle off screen positions
        mouseTracker.push(mousePosition);
        mouseTracker.draw(*this);

        auto& gp = gamepad(0);
        if (gp.connected())
        {
            translate({ 550_px, 350_px });
            fill(Color{ 51 });
            stroke(Appliance::DISABLED);
            ellipse( { 0_px, 0_px }, 80_px);
            stroke(White, 5_px);
            line( {0_px, 0_px}, gp.leftStick().flipY() * 40 );
            resetMatrix();

            translate({ 650_px, 350_px });
            fill(Color{ 51 });
            stroke(Appliance::DISABLED);
            ellipse( { 0_px, 0_px }, 80_px);
            stroke(White, 5_px);
            line( {0_px, 0_px}, gp.rightStick().flipY() * 40 );
            resetMatrix();

            translate({ 750_px, 350_px });
            stroke(1_px, White.ModulateAlpha(160));
            fill(gp.buttonA()?Color{0,192,0}:Color{0,96,0});
            ellipse( {0_px, 20_px}, 20_px);
            fill(White.ModulateAlpha(128));
            text("A", { -6_px, 20_px-8_px});

            fill(gp.buttonB()?Color{192,0,0}:Color{96,0,0});
            ellipse( {20_px, 0_px}, 20_px);
            fill(White.ModulateAlpha(128));
            text("B", { 20_px-6_px, -8_px});

            fill(gp.buttonY()?Color{192,192,0}:Color{96,96,0});
            ellipse( {0_px, -20_px}, 20_px);
            fill(White.ModulateAlpha(128));
            text("Y", { -6_px, -20_px-8_px});

            fill(gp.buttonX()?Color{32,32,224}:Color{0,0,96});
            ellipse( {-20_px, 0_px}, 20_px);
            fill(White.ModulateAlpha(128));
            text("X", { -20_px-6_px, -8_px});

            resetMatrix();
        }

        constexpr Direction translation = {180_px, 400_px};
        angle += 2_deg;
        translate({-148_px, -107_px}); // translate(image.getSize()*-0.5f)
        rotate(angle);
        scale(1+cos(angle*5)*0.1f);
        translate(translation);
        image(imgArrow, {0_px, 0_px});
        stroke(Magenta, 5_px);
        fill(Magenta.ModulateAlpha(64));
        rect({148_px, 107_px},{296_px, 214_px}); // translate(image.getSize()*0.5f, image.getSize())
        resetMatrix();

        rotate(angle);
        scale(1+cos(angle*5)*0.1f);
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
        stroke(White.ModulateAlpha(128), 2_px);
        ellipse({x,150_px},16_px);

        fill(Green);
        ellipse({0_px,100_px},16_px);
        ellipse({getWidth(),100_px},16_px);
        fill( Red );
        ellipse({getWidth()-21_px,100_px},16_px);

        // random
        {
            stroke(Appliance::DISABLED);
            static std::size_t pots[100] = {0};
            static float max = 0.0f;
            static float min = 1000.0f;
            static std::size_t count = 0;
            static std::size_t out_of_range_count = 0;

            pushMatrix();
            resetMatrix();
            translate({ 150_px, 120_px});
            constexpr auto alphaStep = 1.5_deg;
            for (auto alpha = 0.0_deg; alpha < 360_deg - alphaStep; alpha += alphaStep)
            {
                float r = randomGaussian()/3.5*50+50;
                //float r = random(100);
                min = std::min(min, r);
                max = std::max(max, r);
                count++;
                if (r<0 || r>=100)
                {
                    out_of_range_count++;
                    fill(Red);
                    r = 100;
                }
                else
                {
                    fill(White);
                    pots[(int)r]++;
                }
                arc({0_px, 0_px}, { Pixel(r*2),  Pixel(r*2) }, alpha, alpha + alphaStep, ArcMode::PIE);
            }
            popMatrix();

            resetMatrix();
            stroke(Appliance::DISABLED);
            fill(Yellow);
            translate({80_px, 220_px});
            auto pot_max = *std::max_element(std::begin(pots), std::end(pots));
            for (int i = 0; i < 100; ++i)
            {
                auto v = 200.0f * pots[i] / pot_max + 1.0f;
                rect({i*7_px, Pixel(-v*0.5f)}, {7_px, Pixel(v)});
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