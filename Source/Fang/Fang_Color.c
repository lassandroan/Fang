// Copyright (C) 2021  Antonio Lassandro

// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option)
// any later version.

// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.

// You should have received a copy of the GNU General Public License along
// with this program.  If not, see <http://www.gnu.org/licenses/>.

typedef struct Fang_Color {
    uint8_t r, g, b, a;
} Fang_Color;

#define FANG_RED          (Fang_Color){255,   0,   0, 255}
#define FANG_ORANGE       (Fang_Color){255, 128,   0, 255}
#define FANG_YELLOW       (Fang_Color){255, 255,   0, 255}
#define FANG_GREEN        (Fang_Color){  0, 255,   0, 255}
#define FANG_BLUE         (Fang_Color){  0,   0, 255, 255}
#define FANG_PURPLE       (Fang_Color){128,   0, 255, 255}
#define FANG_WHITE        (Fang_Color){255, 255, 255, 255}
#define FANG_GREY         (Fang_Color){128, 128, 128, 255}
#define FANG_BLACK        (Fang_Color){  0,   0,   0, 255}
#define FANG_TRANSPARENT  (Fang_Color){  0,   0,   0,   0}

/**
 * Maps RGBA components of a Fang_Color to a 32-bit, unsigned integer.
**/
static inline uint32_t
Fang_MapColor(
    const Fang_Color * const color)
{
    assert(color);

    return (
        (uint32_t)color->r << 24
      | (uint32_t)color->g << 16
      | (uint32_t)color->b << 8
      | (uint32_t)color->a
    );
}

/**
 * Maps a 32-bit, unsigned integer to RGBA components of a Fang_Color.
**/
static inline Fang_Color
Fang_GetColor(
    uint32_t color)
{
    Fang_Color result;
    result.a = color & 0x000000FF; color >>= 8;
    result.b = color & 0x000000FF; color >>= 8;
    result.g = color & 0x000000FF; color >>= 8;
    result.r = color & 0x000000FF;
    return result;
}

/**
 * Performs alpha blending on two colors.
**/
static inline Fang_Color
Fang_BlendColor(
    const Fang_Color * const source,
    const Fang_Color * const dest)
{
    assert(source);
    assert(dest);

    const float source_r = source->r / 255.0f,
                source_g = source->g / 255.0f,
                source_b = source->b / 255.0f,
                source_a = source->a / 255.0f;

    float dest_r = dest->r / 255.0f,
          dest_g = dest->g / 255.0f,
          dest_b = dest->b / 255.0f,
          dest_a = dest->a / 255.0f;

    dest_r = (source_r * source_a) + (dest_r * (1.0f - source_a));
    dest_g = (source_g * source_a) + (dest_g * (1.0f - source_a));
    dest_b = (source_b * source_a) + (dest_b * (1.0f - source_a));
    dest_a = source_a + (dest_a * (1.0f - source_a));

    dest_r = min(max(dest_r, 0.0f), 1.0f);
    dest_g = min(max(dest_g, 0.0f), 1.0f);
    dest_b = min(max(dest_b, 0.0f), 1.0f);
    dest_a = min(max(dest_a, 0.0f), 1.0f);

    return (Fang_Color){
        .r = (uint8_t)(dest_r * 255.0f),
        .g = (uint8_t)(dest_g * 255.0f),
        .b = (uint8_t)(dest_b * 255.0f),
        .a = (uint8_t)(dest_a * 255.0f),
    };
}
