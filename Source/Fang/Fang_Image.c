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

static inline bool
Fang_ImageValid(
    const Fang_Image * const image)
{
    return (
        image
     && image->pixels
     && image->width
     && image->height
     && image->stride
     && image->pitch
    );
}

/**
 * Sets the image attributes and allocates a pixel buffer for the image.
 *
 * The depth should be 8, 24, or 32 bits. This is then used to calculate the
 * stride (bytes per pixel) and set the image pitch appropriately.
**/
static inline int
Fang_AllocImage(
          Fang_Image * const image,
    const int                width,
    const int                height,
    const int                depth)
{
    assert(image);
    assert(!image->pixels);

    image->width  = width;
    image->height = height;
    image->stride = (depth + 7) >> 3;
    image->pitch  = image->stride * width;
    image->pixels = malloc((size_t)(image->pitch * height));

    if (!image->pixels)
    {
        memset(image, 0, sizeof(Fang_Image));
        return 1;
    }

    return 0;
}

/**
 * Frees an image's pixel data and clears the image's attributes.
 *
 * If the image was previously freed or not allocated, this function does
 * nothing.
**/
static inline void
Fang_FreeImage(
    Fang_Image * const image)
{
    if (Fang_ImageValid(image))
    {
        free(image->pixels);
        memset(image, 0, sizeof(Fang_Image));
    }
}

/**
 * Clears the pixel data for the given image.
 *
 * This resets all values in the pixel buffer to 0, meaning the alpha values are
 * not preserved nor reset to 255 during this operation.
 *
 * The image must have a valid pixel data pointer.
**/
static inline void
Fang_ClearImage(
    Fang_Image * const image)
{
    assert(Fang_ImageValid(image));

    memset((void*)image->pixels, 0, (size_t)(image->pitch * image->height));
}

/**
 * Query an image for a 32-bit color value.
 *
 * If the image depth is less than 32 bits, the missing channels are defaulted
 * to 255.
**/
static inline Fang_Color
Fang_GetPixel(
    const Fang_Image * const image,
    const Fang_Point * const point)
{
    assert(point);

    /* The 'XOR Texture' serves as the default 'missing' texture. */
    if (!Fang_ImageValid(image))
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

    return Fang_GetColor(pixel);
}
