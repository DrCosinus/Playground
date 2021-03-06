#pragma once

#include <type_traits>

namespace cuppa
{
    template<typename T>
    struct ColorComponentTraits;

    template<> struct ColorComponentTraits<unsigned char>  { using diff_type = short; };
    template<> struct ColorComponentTraits<unsigned short> { using diff_type = int; };
    template<> struct ColorComponentTraits<unsigned int>   { using diff_type = long long; };
    template<> struct ColorComponentTraits<char>  { using diff_type = short; };
    template<> struct ColorComponentTraits<short> { using diff_type = int; };
    template<> struct ColorComponentTraits<int>   { using diff_type = long long; };

    template<typename COMPONENT, typename = std::enable_if_t<std::is_arithmetic_v<COMPONENT>&&!std::is_same_v<bool,COMPONENT>>>
    struct ColorT
    {
        using DIFF_COMPONENT = typename ColorComponentTraits<COMPONENT>::diff_type;
        using diff_type = ColorT<DIFF_COMPONENT>;
        ColorT() = default;
        explicit constexpr ColorT(COMPONENT _red, COMPONENT _green, COMPONENT _blue, COMPONENT _alpha=255)
        : blue{ _blue }, green{ _green }, red{ _red }, alpha{ _alpha }
        {}
        explicit constexpr ColorT(COMPONENT _gray, COMPONENT _alpha=255)
        : ColorT{ _gray, _gray, _gray, _alpha }
        {}
        template<typename T, typename = std::enable_if_t<std::is_convertible_v<T, COMPONENT>>>
        explicit constexpr ColorT(T _gray, COMPONENT _alpha=255)
        : ColorT{ static_cast<COMPONENT>(_gray), _alpha }
        {}

        constexpr ColorT ModulateAlpha(COMPONENT _alpha) const
        {
            return ColorT{ red, green, blue, COMPONENT(alpha * _alpha / 255) };
        }

        constexpr auto operator-(const ColorT& rhs) const { return diff_type{ GetRed<DIFF_COMPONENT>() - rhs.GetRed<DIFF_COMPONENT>(), GetGreen<DIFF_COMPONENT>() - rhs.GetGreen<DIFF_COMPONENT>(), GetBlue<DIFF_COMPONENT>() - rhs.GetBlue<DIFF_COMPONENT>(), GetAlpha<DIFF_COMPONENT>() - rhs.GetAlpha<DIFF_COMPONENT>() }; }
        constexpr auto operator*(float scale) const { return ColorT{ static_cast<COMPONENT>(scale*red), static_cast<COMPONENT>(scale*green), static_cast<COMPONENT>(scale*blue), static_cast<COMPONENT>(scale*alpha) }; }
        constexpr auto operator+(const diff_type& rhs) const 
        {
            return ColorT{
                static_cast<COMPONENT>(red + rhs.template GetRed<DIFF_COMPONENT>()),
                static_cast<COMPONENT>(green + rhs.template GetGreen<DIFF_COMPONENT>()),
                static_cast<COMPONENT>(blue + rhs.template GetBlue<DIFF_COMPONENT>()),
                static_cast<COMPONENT>(alpha + rhs.template GetAlpha<DIFF_COMPONENT>())};
        }
        // static constexpr auto FromGray(U8 gray) { return ColorT{ gray, 255 }; }
        // static constexpr auto FromRGB(U8 red, U8 green, U8 blue) { return ColorT{ red, green, blue, 255 }; }
        // static constexpr auto FromRGBA(U8 red, U8 green, U8 blue, U8 alpha) { return ColorT{ red, green, blue, alpha }; }
        //static constexpr auto FromHSV() { return ColorT{}; }
        //static constexpr auto FromHSB() { return ColorT{}; }
        constexpr unsigned int ToARGB32() const { return ((alpha & 255) << 24) | ((red & 255) << 16) | ((green & 255) << 8) | (blue & 255); }
        template<typename T>
        T GetRed() const { return static_cast<T>(red); }
        template<typename T>
        T GetGreen() const { return static_cast<T>(green); }
        template<typename T>
        T GetBlue() const { return static_cast<T>(blue); }
        template<typename T>
        T GetAlpha() const { return static_cast<T>(alpha); }
    private:
        COMPONENT blue = 255, green = 255, red = 255, alpha = 255;
    };

    using Color = ColorT<unsigned char>;

    // PREDEFINED COLORS (https://en.wikipedia.org/wiki/Web_colors#X11_color_names)
    inline constexpr Color White{ 255 };
    inline constexpr Color LightGray{ 211 };
    inline constexpr Color Silver{ 192 };
    inline constexpr Color DarkGray{ 169 };
    inline constexpr Color Gray{ 128 };
    inline constexpr Color DimGray{ 105 };
    inline constexpr Color Black{ 0 };
    // PINK COLORS
    inline constexpr Color Pink{ 255, 192, 203 };
    // RED COLORS
    inline constexpr Color Red{ 255, 0, 0 };
    inline constexpr Color DarkRed{ 139, 0, 0 };
    // ORANGE COLORS
    inline constexpr Color OrangeRed{ 255, 69, 0 };
    inline constexpr Color Coral{ 255, 127, 80 };
    inline constexpr Color DarkOrange{ 255, 140, 0 };
    inline constexpr Color Orange{ 255, 165, 0 };
    // YELLOW COLORS
    inline constexpr Color Yellow{ 255, 255, 0 };
    inline constexpr Color Gold{ 255, 215, 0 };
    // BROWN COLORS
    inline constexpr Color Goldenrod{ 218, 165, 32 };
    inline constexpr Color DarkGoldenrod{ 184, 134, 11 };
    inline constexpr Color SaddleBrown{ 139, 69, 19 };
    inline constexpr Color Sienna{ 160, 82, 45 };
    inline constexpr Color Brown{ 165, 42, 42 };
    // GREEN COLORS
    inline constexpr Color Lime{ 0, 255, 0 };
    inline constexpr Color Green{ 0, 128, 0 };
    inline constexpr Color DarkGreen{ 0, 100, 0 };
    // CYAN COLORS
    inline constexpr Color Cyan{ 0, 255, 255 };
    inline constexpr Color Aqua{ 0, 255, 255 };
    // BLUE COLORS
    inline constexpr Color CornflowerBlue{ 100, 149, 237 };
    inline constexpr Color Blue{ 0, 0, 255 };
    inline constexpr Color MediumBlue{ 0, 0, 205 };
    inline constexpr Color DarkBlue{ 0, 0, 139 };
    inline constexpr Color Navy{ 0, 0, 128 };
    // PURPLE, VIOLET AND MAGENTA COLORS
    inline constexpr Color Magenta{ 255, 0, 255 };
    inline constexpr Color Fuchsia{ 255, 0, 255 };
    inline constexpr Color DarkViolet{ 148, 0, 211 };
    inline constexpr Color Purple{ 128, 0, 128 };
    inline constexpr Color Indigo{ 75, 0, 130 };
}