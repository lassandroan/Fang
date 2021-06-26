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

    Fang_Atlas textures;

    Fang_TileType * tiles;
    Fang_Tile * sizes;

    Fang_Image skybox;
    Fang_Image floor;

    Fang_Color fog;
    float      fog_distance;
} Fang_Map;

enum {
    temp_map_size = 8,
};

Fang_TileType temp_map_map[temp_map_size][temp_map_size] = {
    {1, 1, 1, 0, 0, 0, 1, 1},
    {1, 0, 0, 0, 0, 0, 2, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 2, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 1, 0, 1},
    {1, 1, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
};

Fang_Map temp_map = {
    .tiles = &temp_map_map[0][0],
    .size = temp_map_size,
    .fog = FANG_BLACK,
    .fog_distance = 16,
};

static inline int
Fang_LoadMap(void)
{
    assert(!temp_map.skybox.pixels);
    assert(!temp_map.floor.pixels);

    for (Fang_Texture i = 0; i < FANG_NUM_TEXTURES; ++i)
        Fang_AtlasLoad(&temp_map.textures, i);

    temp_map.skybox = *Fang_AtlasQuery(
        &temp_map.textures, FANG_TEXTURE_SKYBOX
    );

    temp_map.floor = *Fang_AtlasQuery(
        &temp_map.textures, FANG_TEXTURE_FLOOR
    );

    return 0;
}

static inline void
Fang_DestroyMap(void)
{
    Fang_AtlasFree(&temp_map.textures);
}

static inline const Fang_Tile *
Fang_MapQuery(
    const Fang_Map * const map,
    const int              x,
    const int              y)
{
    assert(map);

    static const Fang_Tile solid_tile = {
        .texture = FANG_TEXTURE_TILE,
        .y = 0.0f,
        .h = 1.0f,
    };

    static const Fang_Tile floating_tile = {
        .texture = FANG_TEXTURE_TILE,
        .y = 1.0f,
        .h = 1.0f,
    };

    if (x < 0 || x >= map->size)
        return NULL;

    if (y < 0 || y >= map->size)
        return NULL;

    const Fang_TileType type = map->tiles[y * map->size + x];

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
