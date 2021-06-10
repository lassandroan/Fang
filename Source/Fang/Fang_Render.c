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
 * @brief Draws a line in the framebuffer using Bresenham's Algorithm.
 *
 * @param framebuf the target framebuffer, must have color image
 * @param start    the beginning point of the line
 * @param end      the ending point of the line
 * @param color    the color of the line
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
 * @brief Draws a 1px thick outline of a rectangle in the framebuffer.
 *
 * @param framebuf the target framebuffer, must have color image
 * @param rect     the rectangle to draw
 * @param color    the color of the rectangle
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

/** @brief Draws a solid rectangle in the framebuffer.
 *
 * @param framebuf the target framebuffer, must have color image
 * @param rect     the rectangle to draw
 * @param color    the color of the rectangle
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
