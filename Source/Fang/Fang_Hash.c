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

static inline uint16_t
Fang_MortonIndex(
    const int8_t x,
    const int8_t y)
{
    static uint64_t lookup[1 << 4] = {
        [ 0] = 0b00000000,
        [ 1] = 0b00000001,
        [ 2] = 0b00000100,
        [ 3] = 0b00000101,
        [ 4] = 0b00010000,
        [ 5] = 0b00010001,
        [ 6] = 0b00010100,
        [ 7] = 0b00010101,
        [ 8] = 0b01000000,
        [ 9] = 0b01000001,
        [10] = 0b01000100,
        [11] = 0b01000101,
        [12] = 0b01010000,
        [13] = 0b01010001,
        [14] = 0b01010100,
        [15] = 0b01010101,
    };

    uint16_t result = 0;
    for (uint8_t i = 0; i < 32; i += 4)
    {
        const uint8_t xbits = (x >> i) & 0b1111;
        const uint8_t ybits = (y >> i) & 0b1111;
        result |= (lookup[xbits] | (lookup[ybits] << 1)) << (i * 2);
    }

    return result;
}
