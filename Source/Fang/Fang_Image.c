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
