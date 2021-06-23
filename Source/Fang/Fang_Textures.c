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
 * A constant representing the width|height of a face in a tile-type texture.
**/
enum {
    FANG_FACE_SIZE = 128,
};

/**
 * All fonts should be 8x9 in size, with a 1px barrier in between each
 * character.
**/
enum {
    FANG_FONT_HEIGHT = 9,
    FANG_FONT_WIDTH  = 8,
};

/**
 * The textures available to the game.
 *
 * Each one of these corresponds to a texture file in the resource folder.
**/
typedef enum Fang_Texture {
    /* Map Textures */
    FANG_TEXTURE_SKYBOX,
    FANG_TEXTURE_FLOOR,
    FANG_TEXTURE_TILE,

    /* Fonts */
    FANG_TEXTURE_FORMULA,

    FANG_NUM_TEXTURES,
} Fang_Texture;

/**
 * This structure is used for managing textures and fonts.
**/
typedef struct Fang_Atlas {
    Fang_Image textures[FANG_NUM_TEXTURES];
} Fang_Atlas;

/**
 * Frees a previously loaded texture.
**/
static inline void
Fang_AtlasUnload(
          Fang_Atlas   * const atlas,
    const Fang_Texture         id)
{
    assert(atlas);
    assert(id < FANG_NUM_TEXTURES);
    assert(atlas->textures[id].pixels);

    free(atlas->textures[id].pixels);
    memset(&atlas->textures[id], 0, sizeof(Fang_Image));
}

/**
 * Loads a texture from the game's resource directory.
 *
 * If the texture has already been loaded, it is unloaded and then loaded again.
 * This can be used for refreshing textures that may have changed on disk.
 *
 * When a texture is loaded, its attributes such as width, height, stride, etc.
 * may be checked for validation.
**/
static inline int
Fang_AtlasLoad(
          Fang_Atlas   * const atlas,
    const Fang_Texture         id)
{
    assert(atlas);
    assert(id < FANG_NUM_TEXTURES);

    Fang_Image * const result = &atlas->textures[id];

    if (result->pixels)
        Fang_AtlasUnload(atlas, id);

    typedef enum {
        FONT_TEXTURE,
        TILE_TEXTURE,
        OTHER_TEXTURE,
    } Type;

    typedef struct {
        const char * const path;
        const Type         type;
    } Info;

    static const Info texture_info[FANG_NUM_TEXTURES] = {
        /* Map Textures */
        [FANG_TEXTURE_SKYBOX] = (Info){
            .path = "Textures/skybox.tga",
            .type = OTHER_TEXTURE,
        },
        [FANG_TEXTURE_FLOOR] = (Info){
            .path = "Textures/floor.tga",
            .type = OTHER_TEXTURE,
        },
        [FANG_TEXTURE_TILE] = (Info){
            .path = "Textures/tile.tga",
            .type = TILE_TEXTURE,
        },

        /* Fonts */
        [FANG_TEXTURE_FORMULA] = (Info){
            .path = "Fonts/Formula.tga",
            .type = FONT_TEXTURE,
        },
    };

    *result = Fang_TGALoad(texture_info[id].path);
    if (!result->pixels)
        return 1;

    switch (texture_info[id].type)
    {
        case TILE_TEXTURE:
            assert(result->width  == FANG_FACE_SIZE * 6);
            assert(result->height == FANG_FACE_SIZE);
            break;

        case FONT_TEXTURE:
            break;

        default:
            break;
    }

    return 0;
}

/**
 * Unloads all textures currently loaded in the atlas.
**/
static inline void
Fang_AtlasFree(
    Fang_Atlas * const atlas)
{
    assert(atlas);

    for (Fang_Texture i = 0; i < FANG_NUM_TEXTURES; ++i)
        if (atlas->textures[i].pixels)
            Fang_AtlasUnload(atlas, i);
}

static inline const Fang_Image*
Fang_AtlasQuery(
    const Fang_Atlas   * const atlas,
    const Fang_Texture         id)
{
    assert(atlas);
    assert(id < FANG_NUM_TEXTURES);

    return &atlas->textures[id];
}
