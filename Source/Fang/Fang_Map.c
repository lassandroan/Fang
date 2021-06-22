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
    int width;
    int height;

    int tile_size;

    Fang_TileType * tiles;
    Fang_TileSize * sizes;
    Fang_Color    * colors;
    Fang_Image    * textures;

    Fang_Image skybox;
    Fang_Image floor;
} Fang_Map;

enum {
    temp_map_width  = 8,
    temp_map_height = 8,
};

Fang_TileType temp_map_map[temp_map_width][temp_map_height] = {
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 2, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 2, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 2, 0, 1},
    {1, 1, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
};

Fang_Map temp_map = {
    .tiles = &temp_map_map[0][0],
    .width = temp_map_width,
    .height = temp_map_height,
    .tile_size = 16,
};

static inline Fang_TileType
Fang_MapQueryType(
    const Fang_Map * const map,
    int x,
    int y)
{
    assert(map);

    x /= map->tile_size;
    y /= map->tile_size;

    if (x < 0 || x >= map->width)
        return FANG_TILETYPE_NONE;

    if (y < 0 || y >= map->height)
        return FANG_TILETYPE_NONE;

    return map->tiles[y * map->width + x];
}

static inline Fang_TileSize
Fang_MapQuerySize(
    const Fang_Map * const map,
    const int x,
    const int y)
{
    assert(map);

    const Fang_TileType type = Fang_MapQueryType(map, x, y);

    switch (type)
    {
        case FANG_TILETYPE_SOLID:
            return (Fang_TileSize){0, map->tile_size};

        case FANG_TILETYPE_FLOATING:
            return (Fang_TileSize){map->tile_size + 3, map->tile_size};

        default:
            return (Fang_TileSize){0, 0};
    }
}

static inline Fang_Color
Fang_MapQueryColor(
    const Fang_Map * const map,
    const int x,
    const int y)
{
    assert(map);

    const Fang_TileType type = Fang_MapQueryType(map, x, y);

    switch (type)
    {
        case FANG_TILETYPE_SOLID:
            return (Fang_Color){
                .r = 255,
                .g = 255,
                .b = 255,
                .a = 128,
            };

        case FANG_TILETYPE_FLOATING:
            return FANG_GREY;

        default:
            return FANG_TRANSPARENT;
    }
}

static inline Fang_Image
Fang_MapQueryTexture(
    const Fang_Map * const map,
    const int x,
    const int y)
{
    assert(map);
    (void)x;
    (void)y;

    return Fang_TileTextures[0];
}
