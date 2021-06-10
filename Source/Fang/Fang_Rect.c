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
