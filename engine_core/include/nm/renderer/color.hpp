#pragma once

#include "nm/core/types.hpp"

namespace nm::renderer
{

struct Color
{
    u8 r;
    u8 g;
    u8 b;
    u8 a;

    constexpr Color()
        : r(255), g(255), b(255), a(255)
    {
    }

    constexpr Color(u8 red, u8 green, u8 blue, u8 alpha = 255)
        : r(red), g(green), b(blue), a(alpha)
    {
    }

    static constexpr Color fromRGBA(u32 rgba)
    {
        return Color(
            static_cast<u8>((rgba >> 24) & 0xFF),
            static_cast<u8>((rgba >> 16) & 0xFF),
            static_cast<u8>((rgba >> 8) & 0xFF),
            static_cast<u8>(rgba & 0xFF)
        );
    }

    [[nodiscard]] constexpr u32 toRGBA() const
    {
        return (static_cast<u32>(r) << 24) |
               (static_cast<u32>(g) << 16) |
               (static_cast<u32>(b) << 8) |
               static_cast<u32>(a);
    }

    static const Color White;
    static const Color Black;
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Yellow;
    static const Color Cyan;
    static const Color Magenta;
    static const Color Transparent;
};

inline constexpr Color Color::White{255, 255, 255, 255};
inline constexpr Color Color::Black{0, 0, 0, 255};
inline constexpr Color Color::Red{255, 0, 0, 255};
inline constexpr Color Color::Green{0, 255, 0, 255};
inline constexpr Color Color::Blue{0, 0, 255, 255};
inline constexpr Color Color::Yellow{255, 255, 0, 255};
inline constexpr Color Color::Cyan{0, 255, 255, 255};
inline constexpr Color Color::Magenta{255, 0, 255, 255};
inline constexpr Color Color::Transparent{0, 0, 0, 0};

} // namespace nm::renderer
