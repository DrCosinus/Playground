#include "cuppa.hpp"

#include <algorithm>
#include <memory>
#include <string>

namespace
{
    struct demo : cuppa::app
    {
        using cuppa::app::app;

        Image imgArrow;
        Image imgDots;
        Pixel x = -8_px;
        Angle angle = 0_deg;
        std::size_t clickCount = 0;

        demo() : gamepadTracker{ *this } {}

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
            void draw(demo& s) const
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

        struct Gamepad
        {
            demo& s;
            explicit Gamepad(demo& _s) : s{ _s } {}

            void drawStick(Direction centerPosition, Direction stickDirection, bool isDown) const
            {
                s.pushMatrix();
                s.translate(centerPosition);
                s.fill(isDown ? Color{128} : Color{ 51 });
                s.stroke(Appliance::DISABLED);
                s.ellipse( { }, 80_px);
                s.stroke(White, 5_px);
                s.line( {0_px, 0_px}, stickDirection.flipY() * 40 );
                s.popMatrix();
            }
            void drawRoundButton(Direction centerPosition, bool isDown, std::string_view name, Color upColor, Color downColor) const
            {
                s.pushMatrix();
                s.translate(centerPosition);
                s.stroke(1_px, White.ModulateAlpha(160));
                s.fill(isDown?downColor:upColor);
                s.ellipse( { }, 20_px);
                s.fill(White.ModulateAlpha(128));
                s.text(name, {}, TextHAlign::CENTER, TextVAlign::MIDDLE);
                s.popMatrix();
            }
            void drawSquareButton(Direction centerPosition, bool isDown, std::string_view name, Color upColor, Color downColor) const
            {
                s.pushMatrix();
                s.translate(centerPosition);
                s.stroke(1_px, White.ModulateAlpha(160));
                s.fill(isDown?downColor:upColor);
                s.rect( { }, { 25_px, 15_px });
                s.fill(White.ModulateAlpha(128));
                s.text(name, {}, TextHAlign::CENTER, TextVAlign::MIDDLE);
                s.popMatrix();
            }
            void drawGauge(Direction centerPosition, float gauge, std::string_view name, Color upColor, Color downColor) const
            {
                s.pushMatrix();
                s.translate(centerPosition);
                s.stroke(1_px, White.ModulateAlpha(160));
                s.fill(upColor);
                s.rect( { }, { 25_px, 15_px });
                s.fill(downColor);
                s.stroke(Appliance::ENABLED);
                s.rect( { }, { Pixel{25*gauge}, 15_px });
                s.fill(White.ModulateAlpha(128));
                s.text(name, {}, TextHAlign::CENTER, TextVAlign::MIDDLE);
                s.popMatrix();
            }

            void draw() const
            {
                auto& gp = s.gamepad(0);
                if (gp.connected())
                {
                    s.pushMatrix();
                    s.translate({480_px, 230_px});
                    drawStick({10_px, 70_px}, gp.leftStick(), gp.buttonLeftThumb());
                    drawStick({ 170_px, 120_px }, gp.rightStick(), gp.buttonRightThumb());

                    s.pushMatrix();
                    s.translate({230_px, 70_px});
                    drawRoundButton({0_px, 20_px}, gp.buttonA(), "A", Color{0,96,0}, Color{0,192,0});
                    drawRoundButton({20_px, 0_px}, gp.buttonB(), "B", Color{96,0,0}, Color{192,0,0});
                    drawRoundButton({-20_px, 0_px}, gp.buttonX(), "X", Color{0,0,96}, Color{32,32,224});
                    drawRoundButton({0_px, -20_px}, gp.buttonY(), "Y", Color{96,96,0}, Color{192,192,0});
                    s.popMatrix();

                    s.pushMatrix();
                    s.translate({ 70_px, 120_px });
                    drawRoundButton({0_px, 20_px}, gp.buttonDigitalDown(), "V", Color{32}, Color{128});
                    drawRoundButton({20_px, 0_px}, gp.buttonDigitalRight(), ">", Color{32}, Color{128});
                    drawRoundButton({-20_px, 0_px}, gp.buttonDigitalLeft(), "<", Color{32}, Color{128});
                    drawRoundButton({0_px, -20_px}, gp.buttonDigitalUp(), "^", Color{32}, Color{128});
                    s.popMatrix();

                    drawSquareButton({ 90_px, 60_px}, gp.buttonBack(), "<-", Color{32}, Color{128} );
                    drawSquareButton({ 150_px, 60_px}, gp.buttonStart(), "->", Color{32}, Color{128} );

                    drawSquareButton({ 70_px, 28_px}, gp.buttonLeftShoulder(), "LB", Color{32}, Color{128} );
                    drawSquareButton({ 170_px, 28_px}, gp.buttonRightShoulder(), "RB", Color{32}, Color{128} );

                    drawGauge({ 70_px, 10_px}, gp.leftTrigger(), "LT", Color{32}, Color{128} );
                    drawGauge({ 170_px, 10_px}, gp.rightTrigger(), "RT", Color{32}, Color{128} );
                    s.popMatrix();
                }
            }
        } gamepadTracker;

        void draw() override
        {
            background(Black);
            stroke(5_px, White);
            line(previousMousePosition, mousePosition); // TODO: handle off screen positions
            mouseTracker.push(mousePosition);
            mouseTracker.draw(*this);

            gamepadTracker.draw();

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
                    float r = randomGaussian()/3.5f*50+50;
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
} // anonymous namespace

std::unique_ptr<cuppa::app> createDemo()
{
    return std::make_unique<demo>();
}