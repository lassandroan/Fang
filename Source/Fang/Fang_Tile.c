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

typedef enum Fang_TileType {
    FANG_TILETYPE_NONE,
    FANG_TILETYPE_SOLID,
    FANG_TILETYPE_FLOATING,

    FANG_NUM_TILETYPE,
} Fang_TileType;

typedef struct Fang_TileSize {
    int height;
    int size;
} Fang_TileSize;

enum {
    FANG_TILEAREA_HEIGHT = 128,
    FANG_TILEAREA_WIDTH  = 128,
};

typedef enum Fang_TileTextureType {
    FANG_TILETEXTURE_TILE,

    FANG_NUM_TILETEXTURE,
} Fang_TileTextureType;

const char * const Fang_TileTexturePaths[FANG_NUM_TILETEXTURE] = {
    [FANG_TILETEXTURE_TILE] = "Maps/test/tile.tga"
};

Fang_Image Fang_TileTextures[FANG_NUM_TILETEXTURE];

static inline int
Fang_CreateTextures(void)
{
    for (int i = 0; i < FANG_NUM_TILETEXTURE; ++i)
    {
        const char * const path = Fang_TileTexturePaths[i];

        Fang_File file = {.data = NULL};
        if (Fang_LoadFile(path, &file) != 0)
            return 1;

        Fang_Image * const texture = &Fang_TileTextures[i];

        *texture = Fang_TGALoad(&file);

        Fang_FreeFile(&file);

        if (!texture->pixels)
            return 1;
    }

    return 0;
}

static inline void
Fang_DestroyTextures(void)
{
    for (int i = 0; i < FANG_NUM_TILETEXTURE; ++i)
    {
        assert(Fang_TileTextures[i].pixels);

        free(Fang_TileTextures[i].pixels);
        memset(&Fang_TileTextures[i], 0, sizeof(Fang_Image));
    }
}

static inline Fang_Image*
Fang_TileTextureGet(
    const Fang_TileTextureType type)
{
    assert(type >= 0);
    assert(type <= FANG_NUM_TILETEXTURE);

    if (type == FANG_NUM_TILETEXTURE)
        return &Fang_TileTextures[type - 1];

    return &Fang_TileTextures[type];
}
