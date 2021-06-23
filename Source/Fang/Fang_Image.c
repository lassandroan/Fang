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

/**
 * A container for image pixel data.
**/
typedef struct Fang_Image {
    uint8_t * pixels;
    int       width;
    int       height;
    int       pitch;
    int       stride;
} Fang_Image;

/**
 * Clears the pixel data for the given image.
 *
 * This resets all values in the pixel buffer to 0, meaning the alpha values are
 * not preserved nor reset to 255 during this operation.
 *
 * The image must have a valid pixel data pointer.
**/
static inline void
Fang_ImageClear(
    Fang_Image * const image)
{
    assert(image);
    assert(image->pixels);

    memset(
        (void*)image->pixels,
        0,
        (size_t)(image->pitch * image->height)
    );
}

/**
 * Query an image for a 32-bit color value.
 *
 * If the image depth is less than 32 bits, the missing channels are defaulted
 * to 255.
**/
static inline Fang_Color
Fang_ImageQuery(
    const Fang_Image * const image,
    const Fang_Point * const point)
{
    assert(image);
    assert(point);

    /* The 'XOR Texture' serves as the default 'missing' texture. */
    if (!image->pixels)
    {
        const uint8_t value = (uint8_t)point->x ^ (uint8_t)point->y;
        return (Fang_Color){value, value, value, 255};
    }

    assert(point->x >= 0 && point->x < image->width);
    assert(point->y >= 0 && point->y < image->height);

    uint32_t pixel = 0;

    for (int p = 0; p < image->stride; ++p)
    {
        pixel |= *(
            image->pixels + p
          + (point->x * image->stride)
          + (point->y * image->pitch)
        );

        if (p < image->stride - 1)
            pixel <<= 8;
    }

    for (int p = image->stride; p < 4; ++p)
    {
        pixel <<= 8;
        pixel |= 0x000000FF;
    }

    return Fang_ColorFromRGBA(pixel);
}
