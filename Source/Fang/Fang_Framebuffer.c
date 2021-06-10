// Copyright (C) 2021 Antonio Lassandro

// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.

// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public
// License for more details.

// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/**
 * A structure used for rendering to the screen.
 *
 * Framebuffers consist of two images:
 * - A color image whose result is drawn to the screen
 * - A stencil buffer used internally to discard pixels
 *
 * @see Fang_FramebufferPutPixel()
**/
typedef struct Fang_Framebuffer
{
    Fang_Image color;
    Fang_Image stencil;
    bool       enable_stencil;
} Fang_Framebuffer;

/**
 * @brief Writes a pixel of a given color to the framebuffer.
 *
 * This routine utilizes the framebuffer's stencil when placing pixels. If the
 * stencil is enabled, its buffer is checked to see if a value has already been
 * written in this position.
 *
 * If a value has been written, we do not write the new pixel into the color
 * image. If a value has not been written then the stencil is written to and the
 * color is written into the color image. If the point lies outside the
 * framebuffer bounds this function does nothing.
 *
 * The framebuffer must have a color image.
**/
static inline bool
Fang_FramebufferPutPixel(
    const Fang_Framebuffer * const framebuf,
    const Fang_Point       * const point,
    const Fang_Color       * const color)
{
    assert(framebuf);
    assert(framebuf->color.pixels);

    if (point->x < 0 || point->x >= framebuf->color.width)
        return false;

    if (point->y < 0 || point->y >= framebuf->color.height)
        return false;

    bool write = true;

    if (framebuf->enable_stencil)
    {
        assert(framebuf->stencil.pixels);
        assert(framebuf->stencil.width  == framebuf->color.width);
        assert(framebuf->stencil.height == framebuf->color.height);
        assert(framebuf->stencil.pitch  == framebuf->stencil.width);

        const uint8_t pixel  = UINT8_MAX;
        const int     offset = (
            point->y * framebuf->stencil.pitch
          + point->x * framebuf->stencil.stride
        );

        if (*(framebuf->stencil.pixels + offset))
            write = false;
        else
            *(framebuf->stencil.pixels + offset) = pixel;
    }

    if (write)
    {
        const uint32_t pixel  = Fang_MapColor(color);
        const int      offset = (
            point->y * framebuf->color.pitch
          + point->x * framebuf->color.stride
        );

        *(uint32_t*)(framebuf->color.pixels + offset) = pixel;
    }

    return write;
}

/**
 * @brief Clear's the framebuffer's color and stencil images.
 *
 * If the framebuffer does not have a stencil image, only the color is cleared.
 * The framebuffer must have a color image.
**/
static inline void
Fang_FramebufferClear(
    Fang_Framebuffer * const framebuf)
{
    assert(framebuf);
    assert(framebuf.color->pixels);

    Fang_ImageClear(&framebuf->color);

    if (framebuf->stencil.pixels)
        Fang_ImageClear(&framebuf->stencil);
}
