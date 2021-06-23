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

/**
 * The texture types available to the game.
 *
 * Each one of these corresponds to a texture file in the resource folder.
**/
typedef enum Fang_TextureType {
    FANG_TEXTURETYPE_TILE,

    FANG_NUM_TEXTURETYPE,
} Fang_TextureType;

enum {
    FANG_TEXTURE_HEIGHT = 128,
    FANG_TEXTURE_WIDTH  = 128,
};

static const char * const Fang_TexturePaths[FANG_NUM_TEXTURETYPE] = {
    [FANG_TEXTURETYPE_TILE] = "Maps/test/tile.tga"
};

/**
 * This structure is used for managing loaded textures in-game.
**/
typedef struct Fang_TextureAtlas {
    Fang_Image textures[FANG_NUM_TEXTURETYPE];
} Fang_TextureAtlas;

/**
 * Frees a previously loaded texture.
**/
static inline void
Fang_TextureAtlasUnload(
          Fang_TextureAtlas * const atlas,
    const size_t                    id)
{
    assert(atlas);
    assert(id < FANG_NUM_TEXTURETYPE);
    assert(atlas->textures[id].pixels);

    free(atlas->textures[id].pixels);
    memset(&atlas->textures[id], 0, sizeof(Fang_Image));
}

/**
 * Loads a texture from the game's resource directory.
 *
 * If the texture has already been loaded, it is unloaded and then loaded again.
 * This can be used for refreshing textures that may have changed on disk.
**/
static inline int
Fang_TextureAtlasLoad(
          Fang_TextureAtlas * const atlas,
    const size_t                    id)
{
    assert(atlas);
    assert(id < FANG_NUM_TEXTURETYPE);

    if (atlas->textures[id].pixels)
        Fang_TextureAtlasUnload(atlas, id);

    atlas->textures[id] = Fang_TGALoad(Fang_TexturePaths[id]);
    return atlas->textures[id].pixels != NULL;
}

/**
 * Unloads all textures currently loaded in the atlas.
**/
static inline void
Fang_TextureAtlasFree(
    Fang_TextureAtlas * const atlas)
{
    assert(atlas);

    for (size_t i = 0; i < FANG_NUM_TEXTURETYPE; ++i)
        if (atlas->textures[i].pixels)
            Fang_TextureAtlasUnload(atlas, i);
}

static inline Fang_Image
Fang_TextureAtlasQuery(
    const Fang_TextureAtlas * const atlas,
    const size_t                    id)
{
    assert(atlas);
    assert(id < FANG_NUM_TEXTURETYPE);

    return atlas->textures[id];
}
