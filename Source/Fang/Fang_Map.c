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

typedef struct Fang_Map {
    Fang_TextureId skybox;
    Fang_TextureId floor;
    Fang_Color     fog;
    float          fog_distance;
    Fang_ChunkSet  chunks;
} Fang_Map;

static inline const Fang_Tile *
Fang_MapQuery(
    const Fang_Map * const map,
    const int              x,
    const int              y)
{
    assert(map);

    static const Fang_Tile solid_tile = {
        .texture = FANG_TEXTURE_TILE,
        .offset = 0.0f,
        .height = 0.1f,
    };

    static const Fang_Tile floating_tile = {
        .texture = FANG_TEXTURE_TILE,
        .offset = 0.0f,
        .height = 0.2f,
    };

    if (x < 0 || x >= 8)
        return NULL;

    if (y < 0 || y >= 8)
        return NULL;

    static const Fang_TileType temp_tiles[8 * 8] = {
        1, 1, 1, 0, 0, 0, 1, 1,
        1, 0, 0, 0, 0, 0, 2, 1,
        1, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 1,
        1, 2, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 1, 0, 1,
        1, 1, 0, 0, 0, 0, 0, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
    };

    const Fang_TileType type = temp_tiles[y * 8 + x];

    switch (type)
    {
        case FANG_TILETYPE_SOLID:
            return &solid_tile;

        case FANG_TILETYPE_FLOATING:
            return &floating_tile;

        default:
            return NULL;
    }
}
