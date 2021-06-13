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

typedef struct Fang_Point {
    int x, y;
} Fang_Point;

typedef struct Fang_Rect {
    int x, y, w, h;
} Fang_Rect;

/**
 * Returns the source rectangle, clipped to fit within the bounds of the
 * destination rectangle.
**/
static inline Fang_Rect
Fang_RectClip(
    const Fang_Rect * const source,
    const Fang_Rect * const dest)
{
    assert(source);
    assert(dest);

    Fang_Rect result;
    result.x = max(source->x, dest->x);
    result.w = min(source->x + source->w, dest->x + dest->w) - result.x;
    result.y = max(source->y, dest->y);
    result.h = min(source->y + source->h, dest->y + dest->h) - result.y;
    return result;
}

/**
 * Returns whether or not a point lies within a given area.
**/
static inline bool
Fang_RectContains(
    const Fang_Rect  * const rect,
    const Fang_Point * const point)
{
    assert(rect);
    assert(point);

    return (
        point->x >= rect->x
     && point->x <= rect->x + rect->w
     && point->y >= rect->y
     && point->y <= rect->y + rect->h
    );
}

/**
 * Grows or shrinks a rectangle in each dimension by a given pixel amount.
**/
static inline Fang_Rect
Fang_RectResize(
    const Fang_Rect * const rect,
    const int               x,
    const int               y)
{
    return (Fang_Rect){
        .x = rect->x - (x / 2),
        .y = rect->y - (y / 2),
        .w = rect->w + x,
        .h = rect->h + y,
    };
}
