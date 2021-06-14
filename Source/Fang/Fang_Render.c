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
 * Draws a line in the framebuffer using Bresenham's Algorithm.
 *
 * The target framebuffer must have a valid color image.
**/
static void
Fang_DrawLine(
          Fang_Framebuffer * const framebuf,
    const Fang_Point       * const start,
    const Fang_Point       * const end,
    const Fang_Color       * const color)
{
    assert(framebuf);
    assert(start);
    assert(end);
    assert(color);

    const Fang_Point delta = {
        .x =  abs(end->x - start->x),
        .y = -abs(end->y - start->y),
    };

    const Fang_Point step = {
        .x = (start->x < end->x) ? 1 : (start->x > end->x) ? -1 : 0,
        .y = (start->y < end->y) ? 1 : (start->y > end->y) ? -1 : 0,
    };

    Fang_Point point = *start;
    int        err   = delta.x + delta.y;

    while (true)
    {
        Fang_FramebufferPutPixel(framebuf, &point, color);

        if ((err * 2) >= delta.y)
        {
            if (point.x == end->x)
                break;

            err     += delta.y;
            point.x += step.x;
        }

        if ((err * 2) <= delta.x)
        {
            if (point.y == end->y)
                break;

            err     += delta.x;
            point.y += step.y;
        }
    }
}

/**
 * Draws a 1px thick outline of a rectangle in the framebuffer.
 *
 * The target framebuffer must have a valid color image.
**/
static void
Fang_DrawRect(
          Fang_Framebuffer * const framebuf,
    const Fang_Rect        * const rect,
    const Fang_Color       * const color)
{
    assert(framebuf);
    assert(rect);
    assert(color);

    for (int h = 0; h < rect->h; h += rect->h - 1)
    {
        for (int w = 0; w < rect->w; ++w)
        {
            Fang_FramebufferPutPixel(
                framebuf, &(Fang_Point){rect->x + w, rect->y + h}, color
            );
        }
    }

    for (int h = 0; h < rect->h; ++h)
    {
        for (int w = 0; w < rect->w; w += rect->w - 1)
        {
            Fang_FramebufferPutPixel(
                framebuf, &(Fang_Point){rect->x + w, rect->y + h}, color
            );
        }
    }
}

/**
 * Draws a solid rectangle in the framebuffer.
 *
 * The target framebuffer must have a valid color image.
**/
static void
Fang_FillRect(
          Fang_Framebuffer * const framebuf,
    const Fang_Rect        * const rect,
    const Fang_Color       * const color)
{
    assert(framebuf);
    assert(rect);
    assert(color);

    for (int h = 0; h < rect->h; ++h)
    {
        for (int w = 0; w < rect->w; ++w)
        {
            Fang_FramebufferPutPixel(
                framebuf, &(Fang_Point){rect->x + w, rect->y + h}, color
            );
        }
    }
}

/**
 * Draws an image (or subsection) to the given area in the framebuffer.
 *
 * If the source is NULL, the image size is used with an origin of 0, 0. If the
 * destination is NULL, the framebuffer color image size is used with an origin
 * of 0, 0.
 *
 * If the sizes of the source and destination rectangles do not match, the image
 * will be scaled to fit the destination rectangle. This scaling is linear, no
 * resampling is performed.
**/
static void
Fang_DrawImage(
          Fang_Framebuffer * const framebuf,
    const Fang_Image       * const image,
    const Fang_Rect        * const source,
    const Fang_Rect        * const dest)
{
    assert(framebuf);
    assert(image);
    assert(image->pixels);

    assert(
        image->stride == 1
     || image->stride == 3
     || image->stride == 4
    );

    assert(framebuf->color.stride == 4);

    Fang_Rect source_area = (source)
        ? *source
        : (Fang_Rect){.w = image->width, .h = image->height};

    source_area = Fang_RectClip(
        &source_area,
        &(Fang_Rect){
            .w = image->width,
            .h = image->height,
        }
    );

    Fang_Rect dest_area = (dest)
        ? *dest
        : (Fang_Rect){.w = framebuf->color.width, .h = framebuf->color.height};

    for (int x = dest_area.x; x < dest_area.x + dest_area.w; ++x)
    {
        if (x < 0 || x >= framebuf->color.width)
            continue;

        for (int y = dest_area.y; y < dest_area.y + dest_area.h; ++y)
        {
            if (y < 0 || y >= framebuf->color.height)
                continue;

            float r_x = (float)(x - dest_area.x)
                      / (float)(dest_area.x + dest_area.w - dest_area.x);

            float r_y = (float)(y - dest_area.y)
                      / (float)(dest_area.y + dest_area.h - dest_area.y);

            r_x = max(min(r_x, 1.0f), 0.0f);
            r_y = max(min(r_y, 1.0f), 0.0f);

            const int t_x = (int)((r_x * source_area.w) + source_area.x);
            const int t_y = (int)((r_y * source_area.h) + source_area.y);

            uint32_t pixel = 0;

            for (int p = 0; p < image->stride; ++p)
            {
                pixel |= *(
                    image->pixels + p
                  + (t_x * image->stride)
                  + (t_y * image->pitch)
                );

                if (p < image->stride - 1)
                    pixel <<= 8;
            }

            for (int p = image->stride; p < 4; ++p)
            {
                pixel <<= 8;
                pixel |= 0x000000FF;
            }

            const Fang_Color dest_color = Fang_ColorFromRGBA(pixel);

            Fang_FramebufferPutPixel(
                framebuf,
                &(Fang_Point){x, y},
                &dest_color
            );
        }
    }
}


/**
 * Draws a line of text into the framebuffer using the given font type.
**/
static void
Fang_DrawText(
          Fang_Framebuffer * const framebuf,
    const char             *       text,
    const Fang_FontType            type,
    const int                      fontheight,
    const Fang_Point       * const origin)
{
    assert(framebuf);
    assert(text);

    Fang_Point position = (origin) ? *origin : (Fang_Point){0, 0};

    const float ratio = (float)fontheight / (float)FANG_FONTAREA_HEIGHT;

    while (*text)
    {
        if (*text == ' ')
        {
            text++;
            position.x += (int)((FANG_FONTAREA_WIDTH + 1) * ratio);
            continue;
        }

        const Fang_Rect character = Fang_FontGetCharPosition(*text);

        Fang_DrawImage(
            framebuf,
            Fang_FontGet(type),
            &character,
            &(Fang_Rect){
                .x = position.x,
                .y = position.y,
                .w = (int)(character.w * ratio),
                .h = (int)(character.h * ratio),
            }
        );

        text++;
        position.x += (int)((FANG_FONTAREA_WIDTH + 1) * ratio);
    }
}
