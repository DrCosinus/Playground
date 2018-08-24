#include "proto.hpp"

#include "console.hpp"
#include "os.hpp"

#include <cuppa/cuppa.hpp>

#include <string>
#include <iostream>
#include <queue>

using namespace cuppa;

namespace proto
{
    namespace CT
    {
        // Requirements:
        // - T{1} gives the neutral element of multiplication of T
        template <std::size_t N, typename T>
        constexpr auto power(T value)
        {
            if constexpr (N == 0)
            {    (void)value;   return T{1};    }
            else if constexpr (N == 1)
                return value;
            else
                return value * power<N-1>(value);
        }
    } // namespace CT

    struct proto : app
    {
        Console console;
        OS fos;
        Font bigFont;
        Image perlinImg;

        void setup() override
        {
            size( 800_px, 600_px );
            console.setup(fos);
            fos.setup(console);
            bigFont = loadFont("Arial", 48);
            perlinImg = createImage({800_px, 600_px});
        }

        void draw() override
        {
            stroke(Appliance::DISABLED);
            scale( 780 / 640.f, 580 / 480.f );
            console.ref( 72, 0) = gamepad(0).connected() ? '0' : '-';
            console.ref( 73, 0) = gamepad(1).connected() ? '1' : '-';
            console.ref( 74, 0) = gamepad(2).connected() ? '2' : '-';
            console.ref( 75, 0) = gamepad(3).connected() ? '3' : '-';
            translate({ 10_px, 10_px });
            background(Black);
            //console.draw();

            rect(pos, { 10_px, 10_px });

            auto& pad = gamepad(0);
            if (pad.connected())
            {
                pos += pad.leftStick().flipY() * 10;
                if (pos.x<-5_px) pos.x += 810_px;
                if (pos.x>805_px) pos.x -= 810_px;
                if (pos.y<-5_px) pos.y += 610_px;
                if (pos.y>605_px) pos.y -= 610_px;
            }
            if (pad.buttonBack())
            {
                quit();
            }
            {
                perlinTest2();
                //perlinTest();
            }
        }

        void perlinTest()
        {
            resetMatrix();
            static auto offset = 0.0f;
            constexpr auto scale = 32.0f;
            stroke(Red, 3_px);
            noFill();
            auto min = 5.0f, max = -5.0f;
            beginShape();
            constexpr auto bucketCount = 50;
            int buckets[bucketCount] = { 0 };
            int fullerBucketSize = 0;
            int fullerBucketIndex = 0;
            for (auto x = 0_px; x < getWidth(); x+=0.3_px)
            {
                auto fy = octave(x / getWidth() * scale + offset);
                auto y = fy * getHeight();
                if ( fy < min ) min = fy;
                if ( fy > max ) max = fy;
                vertex(Point{x, y});
                auto bucketIndex = remap<int>(fy, { 0.0f, 1.0f }, { 0u, bucketCount });
                auto bucketSize = ++buckets[bucketIndex];
                if (bucketSize > fullerBucketSize)
                {
                    fullerBucketIndex = bucketIndex;
                    fullerBucketSize = bucketSize;
                }
            }
            endShape();
            fill(Red);

            textFont(bigFont);
            text(std::to_string(min), { 10_px, 10_px });
            text(std::to_string(max), { 10_px, 60_px });
            offset += 0.01f;
            stroke(Blue);
            fill(Blue.ModulateAlpha(127));
            for(auto bucketIndex = 0; bucketIndex < bucketCount; ++bucketIndex)
            {
                auto x = remap<Pixel>(bucketIndex, {0u, bucketCount}, {0_px, getWidth()});
                auto w = getWidth() / bucketCount;
                auto h = remap<Pixel>(buckets[bucketIndex], { 0, fullerBucketSize}, { 0_px, getHeight() });
                auto y = getHeight() - h;
                rect({x+w/2, y+h/2}, {w, h});
            }
        }

        void perlinTest2()
        {
            resetMatrix();
            constexpr auto scale = 20.0f;
            constexpr auto step = 1_px;
            stroke(Appliance::DISABLED);
            auto min = 100.0f;
            auto max = -100.0f;
            static auto z = 0.0f;
            loadPixels(perlinImg);
            auto pixels = getPixels();
            for (auto y = 0_px; y < getHeight(); y += step)
            {
                for (auto x = 0_px; x < getWidth(); x += step)
                {
                    auto px = x / getWidth() * scale;
                    auto py = y / getHeight() * scale;
                    auto value = noise(px, py, z)
                     + noise(px * 2, py * 2, z * 2) / 2
                     + noise(px * 4, py * 4, z * 4) / 4
                     + noise(px * 8, py * 8, z * 8) / 8;
                    if (value>max) max = value;
                    if (value<min) min = value;
                    if (value>0.99f) value = 0.99f;
                    else if (value<-1) value = -1;
                    auto intensity = remap<int>( value, {-1.0, 1.0f}, {0, 256});
                    auto col = Color{intensity};
                    *(pixels++) = col;
                }
            }
            updatePixels(perlinImg);
            image(perlinImg, {0_px, 0_px});
            z += 0.07f;
            fill(Blue);
            textFont(bigFont);
            text(std::to_string(min), { 10_px, 110_px });
            text(std::to_string(max), { 10_px, 160_px });
        }

        template<std::size_t N>
        float octave_helper(float x, float partial_sum = 0.0f)
        {
            if constexpr(N == 0)
                return partial_sum + noise(x);
            else
                return octave_helper<N-1>( x, partial_sum + noise( CT::power<N>(2) * x) / CT::power<N>(2)); 
        }

        float octave(float x)
        {
            constexpr auto rank = 4u;
            // float result = noise(x);
            // auto factor = 2.0f;
            // for(auto i = 0u; i < rank; ++i, factor *= 2)
            //     result += noise(x*factor)/factor; 
            auto result = octave_helper<rank>(x);
            return result/2 +0.5f;
        }

        void keyDown(short keyCode) override
        {
            console.pushKeyCode(keyCode);
        }

        void keyChar(char c) override
        {
            console.pushChar(c);
        }

        Point pos = { 400_px, 300_px };
    };
} // anonymous namespace

std::unique_ptr<app> createProto()
{
    return std::make_unique<proto::proto>();
}