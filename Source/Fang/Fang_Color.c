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

typedef struct Fang_Color {
    uint8_t r, g, b, a;
} Fang_Color;

const Fang_Color FANG_RED    = {255,   0,   0, 255};
const Fang_Color FANG_ORANGE = {255, 128,   0, 255};
const Fang_Color FANG_YELLOW = {255, 255,   0, 255};
const Fang_Color FANG_GREEN  = {  0, 255,   0, 255};
const Fang_Color FANG_BLUE   = {  0,   0, 255, 255};
const Fang_Color FANG_PURPLE = {128,   0, 255, 255};
const Fang_Color FANG_WHITE  = {255, 255, 255, 255};
const Fang_Color FANG_GREY   = {128, 128, 128, 255};
const Fang_Color FANG_BLACK  = {  0,   0,   0, 255};

/**
 * Maps RGBA components of a Fang_Color to a 32-bit, unsigned integer.
**/
static inline uint32_t
Fang_MapColor(
    const Fang_Color * const color)
{
    assert(color);

    return (
        (uint32_t)color->r << 24
      | (uint32_t)color->g << 16
      | (uint32_t)color->b << 8
      | (uint32_t)color->a
    );
}
