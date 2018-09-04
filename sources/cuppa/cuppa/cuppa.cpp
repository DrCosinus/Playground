#include "cuppa.hpp"

#include <random>

namespace cuppa
{
    extern std::unique_ptr<platformDriverInterface> createPlatformDriverMSWindows();
    extern std::unique_ptr<graphicsDriverInterface> createGraphicsDriverGdiplus();
    extern void assertionError(const char*);

    void Assert(bool _condition, const char* _message)
    {
        if(!_condition)
        {
            assertionError( _message );
            exit(1);
            return;
        }
    }

    void app::onBeginFrame()
    {
        frameStartTime = std::chrono::high_resolution_clock::now();
    }

    void app::onEndFrame()
    {
        using namespace std::chrono;
        fps =  1000.0f / duration_cast<milliseconds>(high_resolution_clock::now() - frameStartTime).count();
    }

    void app::run()
    {
        // Commandline: __argc, __argv
        markTime();

        platformDriver = createPlatformDriverMSWindows();

        graphicsDriver = createGraphicsDriverGdiplus();
        graphicsDriver->setup(*this);

        setup();
        platformDriver->setup(*this);
        platformDriver->run();
        platformDriver = nullptr;
        graphicsDriver = nullptr;
    }

    std::default_random_engine engine{ std::random_device()() };
    std::uniform_real_distribution<float> uniform_distribution{ 0.0f, 1.0f };
    std::normal_distribution<float> normal_distribution{ };

    float random() { return uniform_distribution(engine); }
    float randomGaussian() { return normal_distribution(engine); }

    struct OriginalPerlinNoiseImplementation
    {
        // reference: http://flafla2.github.io/2014/08/09/perlinnoise.html
        static float noise(float x)
        {
            auto xi = static_cast<int>(x) & 255;

            auto xf = x - static_cast<float>(static_cast<int>(x));

            auto u = fade(xf);

            auto a = permutation[xi];
            auto b = permutation[(xi + 1) & 255];
            return lerp(grad(a, xf), grad(b, xf - 1), u);
        }

        static float noise(float x, float y)
        {
            auto xi = static_cast<int>(x) & 255;
            auto yi = static_cast<int>(y) & 255;

            auto xf = x - static_cast<float>(static_cast<int>(x));
            auto yf = y - static_cast<float>(static_cast<int>(y));

            auto u = fade(xf);
            auto v = fade(yf);

            auto aa = permutation[permutation[xi] + yi];
            auto ab = permutation[permutation[xi] + yi + 1];
            auto ba = permutation[permutation[xi + 1] + yi];
            auto bb = permutation[permutation[xi + 1] + yi + 1];

            // The gradient function calculates the dot product between a pseudorandom
            // gradient vector and the vector from the input coordinate to the 8
            // surrounding points in its unit cube.
            // This is all then lerped together as a sort of weighted average based on the faded (u,v,w)
            // values we made earlier.
            auto x1 = lerp(grad(aa, xf, yf), grad(ba, xf - 1, yf),  u);
            auto x2 = lerp(grad(ab, xf, yf - 1), grad(bb, xf - 1, yf - 1), u);
            return lerp(x1, x2, v);
        }

        static float noise(float x, float y, float z)
        {
            auto xi = static_cast<int>(x) & 255;
            auto yi = static_cast<int>(y) & 255;
            auto zi = static_cast<int>(z) & 255;

            auto xf = x - static_cast<float>(static_cast<int>(x));
            auto yf = y - static_cast<float>(static_cast<int>(y));
            auto zf = z - static_cast<float>(static_cast<int>(z));

            auto u = fade(xf);
            auto v = fade(yf);
            auto w = fade(zf);

            auto aaa = permutation[permutation[permutation[xi] + yi] + zi];
            auto aba = permutation[permutation[permutation[xi] + yi + 1] + zi];
            auto baa = permutation[permutation[permutation[xi + 1] + yi] + zi];
            auto bba = permutation[permutation[permutation[xi + 1] + yi + 1] + zi];
            auto aab = permutation[permutation[permutation[xi] + yi] + zi + 1];
            auto abb = permutation[permutation[permutation[xi] + yi + 1] + zi + 1];
            auto bab = permutation[permutation[permutation[xi + 1] + yi] + zi + 1];
            auto bbb = permutation[permutation[permutation[xi + 1] + yi + 1] + zi + 1];

            // The gradient function calculates the dot product between a pseudorandom
            // gradient vector and the vector from the input coordinate to the 8
            // surrounding points in its unit cube.
            // This is all then lerped together as a sort of weighted average based on the faded (u,v,w)
            // values we made earlier.
            auto x1 = lerp(grad(aaa, xf, yf, zf), grad(baa, xf - 1, yf, zf), u);
            auto x2 = lerp(grad(aba, xf, yf - 1, zf), grad(bba, xf - 1, yf - 1, zf), u);
            auto y1 = lerp(x1, x2, v);
            x1 = lerp(grad(aab, xf, yf, zf - 1), grad(bab, xf - 1, yf, zf - 1), u);
            x2 = lerp(grad(abb, xf, yf - 1, zf - 1), grad(bbb, xf - 1, yf - 1, zf - 1), u);
            auto y2 = lerp(x1, x2, v);
            return lerp(y1, y2, w);
        }

      private:
        // Hash lookup table as defined by Ken Perlin.  This is a randomly arranged array of all numbers
        // from 0-255 inclusive.
        // we duplicatede it to avoid overflow and not to have to perform maybe modulo 256 during computation...
        static constexpr int permutation[] =
        {
            151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,
            10,23,190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,88,237,
            149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,77,146,158,231,83,111,
            229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,102,143,54, 65,25,63,161, 1,216,80,
            73,209,76,132,187,208, 89,18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,
            64,52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,
            182,189,28,42,223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
            129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,251,34,242,
            193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,49,192,214, 31,181,199,
            106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,138,236,205,93,222,114,67,29,24,72,
            243,141,128,195,78,66,215,61,156,180,
            151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,
            10,23,190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,88,237,
            149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,77,146,158,231,83,111,
            229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,102,143,54, 65,25,63,161, 1,216,80,
            73,209,76,132,187,208, 89,18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,
            64,52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,
            182,189,28,42,223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
            129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,251,34,242,
            193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,49,192,214, 31,181,199,
            106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,138,236,205,93,222,114,67,29,24,72,
            243,141,128,195,78,66,215,61,156,180
        };

        // Fade function as defined by Ken Perlin.  This eases coordinate values so that they will ease towards integral values.
        // This ends up smoothing the final output: 6t^5 - 15t^4 + 10t^3
        static float fade(float t)
        {
            return t * t * t * ( 10 + t * (-15 + t * 6));
        }

        static float grad(int hash, float x, float y = 0.0f, float z = 0.0f)
        {
            switch (hash & 0xF)
            {
            case 0x0: return x + y;
            case 0x1: return -x + y;
            case 0x2: return x - y;
            case 0x3: return -x - y;
            case 0x4: return x + z;
            case 0x5: return -x + z;
            case 0x6: return x - z;
            case 0x7: return -x - z;
            case 0x8: return y + z;
            case 0x9: return -y + z;
            case 0xA: return y - z;
            case 0xB: return -y - z;
            case 0xC: return y + x;
            case 0xD: return -y + z;
            case 0xE: return y - x;
            case 0xF: return -y - z;
            default:  return 0; // never happens
            }
        }
    };

    struct WikipediaPerlinNoiseImplementation
    {

        // reference: https://en.wikipedia.org/wiki/Perlin_noise
        static float noise(float x, float y)
        {
            auto x0 = static_cast<int>(x);
            auto x1 = x0 + 1;
            auto y0 = static_cast<int>(y);
            auto y1 = y0 + 1;

            auto sx = x - static_cast<float>(x0);
            auto sy = y - static_cast<float>(y0);

            auto n00 = dotGridGradient(x0, y0, x, y);
            auto n10 = dotGridGradient(x1, y0, x, y);
            auto ix0 = lerp(n00, n10, sx);
            auto n01 = dotGridGradient(x0, y1, x, y);
            auto n11 = dotGridGradient(x1, y1, x, y);
            auto ix1 = lerp(n01, n11, sx);
            return lerp(ix0, ix1, sy);
        }

    private:
        // static constexpr auto IXMAX = 8;
        // static constexpr auto IYMAX = 8;

        static float dotGridGradient(int /*ix*/, int /*iy*/, float /*x*/, float /*y*/)
        {
            // extern float Gradient[IYMAX][IXMAX][2];
            // auto dx = x - static_cast<float>(ix);
            // auto dy = y - static_cast<float>(iy);
            // return dx*Gradient[iy][ix][0] + dy*Gradient[iy][ix][1];
            return 0.0f;
        }
    };

    using PerlinNoiseImplementation = OriginalPerlinNoiseImplementation;

    float noise(float x)
    {
        return PerlinNoiseImplementation::noise(x);
    }

    float noise(float x, float y)
    {
        return PerlinNoiseImplementation::noise(x, y);
    }

    float noise(float x, float y, float z)
    {
        return PerlinNoiseImplementation::noise(x, y, z);
    }
} // namespace cuppa