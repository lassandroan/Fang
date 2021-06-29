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
    int size;

    Fang_Tile * tiles;

    Fang_Texture skybox;
    Fang_Texture floor;

    Fang_Color fog;
    float      fog_distance;
} Fang_Map;

static inline const Fang_Tile *
Fang_MapQuery(
    const Fang_Map * const map,
    const int              x,
    const int              y)
{
    assert(map);

    /* NOTE: Temporary */
    assert(map->size == 8);

    static const Fang_Tile solid_tile = {
        .texture = FANG_TEXTURE_TILE,
        .y = 0.0f,
        .h = 0.1f,
    };

    static const Fang_Tile floating_tile = {
        .texture = FANG_TEXTURE_TILE,
        .y = 0.0f,
        .h = 0.2f,
    };

    if (x < 0 || x >= map->size)
        return NULL;

    if (y < 0 || y >= map->size)
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

static inline Fang_Color
Fang_MapGetFog(
    const Fang_Map   * const map,
    const float              dist)
{
    assert(map);

    if (map->fog_distance != 0.0f)
    {
        return (Fang_Color){
            .r = map->fog.r,
            .g = map->fog.g,
            .b = map->fog.b,
            .a = (uint8_t)(
                clamp(dist / map->fog_distance, 0.0f, 1.0f) * 255.0f
            ),
        };
    }

    return (Fang_Color){0, 0, 0, 0};
}

static inline Fang_Color
Fang_MapBlendFog(
    const Fang_Map   * const map,
    const Fang_Color * const color,
    const float              dist)
{
    assert(map);
    assert(color);

    if (map->fog_distance != 0.0f)
    {
        const Fang_Color fog = Fang_MapGetFog(map, dist);
        return Fang_ColorBlend(&fog, color);
    }

    return *color;
}
