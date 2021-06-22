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
 * A structure used for rendering to the screen.
 *
 * Framebuffers consist of two images:
 * - An RGBA color image whose result is drawn to the screen
 * - A stencil buffer used internally to discard pixels
 *
 * @see Fang_FramebufferPutPixel()
**/
typedef struct Fang_Framebuffer
{
    Fang_Image  color;
    Fang_Image  stencil;
    bool        enable_stencil;
    Fang_Mat3x3 transform;
} Fang_Framebuffer;

/**
 * Writes a pixel of a given color to the framebuffer.
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
    assert(framebuf->color.stride == 4);

    const Fang_Point trans_point = Fang_MatMult(framebuf->transform, *point);

    if (trans_point.x < 0 || trans_point.x >= framebuf->color.width)
        return false;

    if (trans_point.y < 0 || trans_point.y >= framebuf->color.height)
        return false;

    bool write = true;

    if (framebuf->enable_stencil)
    {
        assert(framebuf->stencil.pixels);
        assert(framebuf->stencil.width  == framebuf->color.width);
        assert(framebuf->stencil.height == framebuf->color.height);
        assert(framebuf->stencil.stride == 1);

        const uint8_t pixel  = UINT8_MAX;
        const int     offset = (
            trans_point.y * framebuf->stencil.pitch
          + trans_point.x * framebuf->stencil.stride
        );

        if (*(framebuf->stencil.pixels + offset))
            write = false;
        else
            *(framebuf->stencil.pixels + offset) = pixel;
    }

    if (write)
    {
        uint32_t * dst = (uint32_t*)(
            framebuf->color.pixels
          + (trans_point.y * framebuf->color.pitch
          +  trans_point.x * framebuf->color.stride)
        );

        Fang_Color dst_color = Fang_ColorFromRGBA(*dst);

        const float src_r = color->r / 255.0f,
                    src_g = color->g / 255.0f,
                    src_b = color->b / 255.0f,
                    src_a = color->a / 255.0f;

        float dst_r = dst_color.r / 255.0f,
              dst_g = dst_color.g / 255.0f,
              dst_b = dst_color.b / 255.0f,
              dst_a = dst_color.a / 255.0f;

        dst_r = (src_r * src_a) + (dst_r * (1.0f - src_a));
        dst_g = (src_g * src_a) + (dst_g * (1.0f - src_a));
        dst_b = (src_b * src_a) + (dst_b * (1.0f - src_a));
        dst_a = src_a + (dst_a * (1.0f - src_a));

        dst_r = min(max(dst_r, 0.0f), 1.0f);
        dst_g = min(max(dst_g, 0.0f), 1.0f);
        dst_b = min(max(dst_b, 0.0f), 1.0f);
        dst_a = min(max(dst_a, 0.0f), 1.0f);

        *dst = Fang_ColorToRGBA(
            &(Fang_Color){
                .r = (uint8_t)(dst_r * 255.0f),
                .g = (uint8_t)(dst_g * 255.0f),
                .b = (uint8_t)(dst_b * 255.0f),
                .a = (uint8_t)(dst_a * 255.0f),
            }
        );
    }

    return write;
}

/**
 * Clear's the framebuffer's color and stencil images.
 *
 * If the framebuffer does not have a stencil image, only the color is cleared.
 * The framebuffer must have a color image.
**/
static inline void
Fang_FramebufferClear(
    Fang_Framebuffer * const framebuf)
{
    assert(framebuf);
    assert(framebuf->color.pixels);

    Fang_ImageClear(&framebuf->color);

    if (framebuf->stencil.pixels)
        Fang_ImageClear(&framebuf->stencil);
}

/**
 * Get the rectangle representing the framebuffer's viewport (0, 0, w, h).
 *
 * This does not take the framebuffer's transform into account.
**/
static inline Fang_Rect
Fang_FramebufferGetViewport(
    Fang_Framebuffer * const framebuf)
{
    assert(framebuf);

    return (Fang_Rect){
        .x = 0,
        .y = 0,
        .w = framebuf->color.width,
        .h = framebuf->color.height,
    };
}

/**
 * Sets the bounds with which the viewport should draw into.
 *
 * Because this is a viewport transform and not a clip region, the framebuffer
 * will still attempt to draw all contents within the new area.
**/
static inline void
Fang_FramebufferSetViewport(
          Fang_Framebuffer * const framebuf,
    const Fang_Rect        * const viewport)
{
    assert(framebuf);
    assert(viewport);

    if (viewport->w == 0 || viewport->h == 0)
    {
        memset(&framebuf->transform, 0, sizeof(Fang_Mat3x3));
        return;
    }

    const Fang_Rect bounds = Fang_FramebufferGetViewport(framebuf);

    const Fang_Mat3x3 translate = Fang_MatMult(
        framebuf->transform,
        Fang_Mat3x3Translate(viewport->x, viewport->y)
    );

    framebuf->transform = Fang_MatMult(
        translate,
        Fang_Mat3x3Scale(
            (float)viewport->w / (float)bounds.w,
            (float)viewport->h / (float)bounds.h
        )
    );
}
