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

typedef struct Fang_FrameState {
    bool        enable_depth;
    float       current_depth;
    Fang_Matrix transform;
} Fang_FrameState;

/**
 * A structure used for rendering to the screen.
 *
 * Framebuffers consist of two images:
 * - An RGBA color image whose result is drawn to the screen
 * - A depth buffer used internally to discard fragments
**/
typedef struct Fang_Framebuffer
{
    Fang_Image      color;
    Fang_Image      depth;
    Fang_FrameState state;
} Fang_Framebuffer;

/**
 * Writes a fragment of a given color to the framebuffer.
 *
 * This routine utilizes the framebuffer's depth when placing fragments. If the
 * depth buffer is enabled, its buffer is checked to see if a value has already
 * been written in this position.
 *
 * If a value has been written *and* is lower than our supplied depth, we do not
 * write the new fragment into the color image. If a value has not been written
 * then the depth is written to and the color is written into the color image.
 * If the point lies outside the framebuffer bounds this function does nothing.
 *
 * The framebuffer must have a color image.
**/
static inline bool
Fang_SetFragment(
    const Fang_Framebuffer * const framebuf,
    const Fang_Point       * const point,
    const Fang_Color       * const color)
{
    assert(framebuf);
    assert(Fang_ImageValid(&framebuf->color));
    assert(framebuf->color.stride == 4);

    const Fang_Point trans_point = Fang_MultMatrix(
        framebuf->state.transform, *point
    );

    if (trans_point.x < 0 || trans_point.x >= framebuf->color.width)
        return false;

    if (trans_point.y < 0 || trans_point.y >= framebuf->color.height)
        return false;

    if (!color->a)
        return false;

    bool write = true;

    if (framebuf->state.enable_depth)
    {
        assert(Fang_ImageValid(&framebuf->depth));

        assert(framebuf->depth.width  == framebuf->color.width);
        assert(framebuf->depth.height == framebuf->color.height);
        assert(framebuf->depth.stride == 4);

        float * const dest = (float*)(
            framebuf->depth.pixels
          + trans_point.y * framebuf->depth.pitch
          + trans_point.x * framebuf->depth.stride
        );

        if (*dest < framebuf->state.current_depth)
            write = false;
        else if (*dest == FLT_MAX || color->a == UINT8_MAX)
            *dest = framebuf->state.current_depth;
    }

    if (write)
    {
        uint32_t * const dest = (uint32_t*)(
            framebuf->color.pixels
          + trans_point.y * framebuf->color.pitch
          + trans_point.x * framebuf->color.stride
        );

        Fang_Color dest_color    = Fang_GetColor(*dest);
        Fang_Color blended_color = Fang_BlendColor(color, &dest_color);
        *dest = Fang_MapColor(&blended_color);
    }

    return write;
}

/**
 * Calculates a shade using the current depth buffer and blends the result into
 * the framebuffer's color image.
 *
 * This is utilized for drawing fog at the end of the frame.
**/
static inline void
Fang_ShadeFramebuffer(
          Fang_Framebuffer * const framebuf,
    const Fang_Color       * const color,
    const float                    dist)
{
    assert(framebuf);
    assert(Fang_ImageValid(&framebuf->color));
    assert(Fang_ImageValid(&framebuf->depth));
    assert(framebuf->color.width  == framebuf->depth.width);
    assert(framebuf->color.height == framebuf->depth.height);
    assert(framebuf->color.stride == framebuf->depth.stride);
    assert(framebuf->color.stride == 4);
    assert(color);

    if (dist == 0.0f)
        return;

    for (int x = 0; x < framebuf->color.width; ++x)
    {
        for (int y = 0; y < framebuf->color.height; ++y)
        {
            float depth = *(float*)(
                framebuf->depth.pixels
              + y * framebuf->depth.pitch
              + x * framebuf->depth.stride
            );

            if (depth == FLT_MAX)
                continue;

            depth = clamp(depth / dist, 0.0f, 1.0f);

            uint32_t * const dest = (uint32_t*)(
                framebuf->color.pixels
              + y * framebuf->color.pitch
              + x * framebuf->color.stride
            );

            Fang_Color dest_color    = Fang_GetColor(*dest);
            Fang_Color blended_color = Fang_BlendColor(
                &(Fang_Color){
                    .r = color->r,
                    .g = color->g,
                    .b = color->b,
                    .a = (uint8_t)(depth * 255.0f),
                },
                &dest_color
            );

            *dest = Fang_MapColor(&blended_color);
        }
    }
}

/**
 * Get the rectangle representing the framebuffer's viewport (0, 0, w, h).
 *
 * This does not take the framebuffer's transform into account.
**/
static inline Fang_Rect
Fang_GetViewport(
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
 *
 * This returns the previous state of the framebuffer, so that it can easily be
 * restored later.
**/
static inline Fang_FrameState
Fang_SetViewport(
          Fang_Framebuffer * const framebuf,
    const Fang_Rect        * const viewport)
{
    assert(framebuf);
    assert(viewport);

    Fang_FrameState state = framebuf->state;

    if (viewport->w == 0 || viewport->h == 0)
    {
        assert(0);
        memset(&framebuf->state.transform, 0, sizeof(Fang_Matrix));
        return state;
    }

    const Fang_Rect bounds = Fang_GetViewport(framebuf);

    const Fang_Matrix translate = Fang_MultMatrix(
        framebuf->state.transform,
        Fang_TranslateMatrix(viewport->x, viewport->y)
    );

    framebuf->state.transform = Fang_MultMatrix(
        translate,
        Fang_ScaleMatrix(
            (float)viewport->w / (float)bounds.w,
            (float)viewport->h / (float)bounds.h
        )
    );

    return state;
}
