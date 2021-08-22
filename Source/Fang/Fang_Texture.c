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
 * The textures available to the game.
 *
 * Each one of these corresponds to a texture file in the resource folder.
**/
typedef enum Fang_TextureId {
    /* Map Textures */
    FANG_TEXTURE_SKYBOX,
    FANG_TEXTURE_FLOOR,
    FANG_TEXTURE_TILE,

    /* Fonts */
    FANG_TEXTURE_FORMULA,

    /* HUD */
    FANG_TEXTURE_PISTOL_HUD,
    FANG_TEXTURE_CARBINE_HUD,
    FANG_TEXTURE_FLAKGUN_HUD,
    FANG_TEXTURE_CHAINGUN_HUD,
    FANG_TEXTURE_LRAD_HUD,
    FANG_TEXTURE_PLASTICANNON_HUD,
    FANG_TEXTURE_FAZER_HUD,

    /* Sprites */
    FANG_TEXTURE_AMMO,
    FANG_TEXTURE_HEALTH,
    FANG_TEXTURE_PROJECTILE,

    FANG_NUM_TEXTURES,
    FANG_TEXTURE_NONE,
} Fang_TextureId;

/**
 * This structure is used for managing textures and fonts.
**/
typedef struct Fang_Textures {
    Fang_Image textures[FANG_NUM_TEXTURES];
} Fang_Textures;

/**
 * Frees a previously loaded texture.
**/
static inline void
Fang_FreeTexture(
          Fang_Textures  * const textures,
    const Fang_TextureId         id)
{
    assert(textures);
    assert(id < FANG_NUM_TEXTURES);
    assert(Fang_ImageValid(&textures->textures[id]));

    Fang_FreeImage(&textures->textures[id]);
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
Fang_LoadTexture(
          Fang_Textures  * const textures,
    const Fang_TextureId         id)
{
    assert(textures);
    assert(id < FANG_NUM_TEXTURES);

    Fang_Image * const result = &textures->textures[id];

    if (Fang_ImageValid(result))
        Fang_FreeTexture(textures, id);

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
            .path = "Textures/Skybox.tga",
            .type = OTHER_TEXTURE,
        },

        [FANG_TEXTURE_FLOOR] = (Info){
            .path = "Textures/Floor.tga",
            .type = OTHER_TEXTURE,
        },

        [FANG_TEXTURE_TILE] = (Info){
            .path = "Textures/Tile.tga",
            .type = TILE_TEXTURE,
        },

        /* Fonts */
        [FANG_TEXTURE_FORMULA] = (Info){
            .path = "Fonts/Formula.tga",
            .type = FONT_TEXTURE,
        },

        /* HUD */
        [FANG_TEXTURE_PISTOL_HUD] = {
            .path = NULL,
            .type = OTHER_TEXTURE,
        },

        [FANG_TEXTURE_CARBINE_HUD] = {
            .path = NULL,
            .type = OTHER_TEXTURE,
        },

        [FANG_TEXTURE_FLAKGUN_HUD] = {
            .path = NULL,
            .type = OTHER_TEXTURE,
        },

        [FANG_TEXTURE_CHAINGUN_HUD] = {
            .path = NULL,
            .type = OTHER_TEXTURE,
        },

        [FANG_TEXTURE_LRAD_HUD] = {
            .path = NULL,
            .type = OTHER_TEXTURE,
        },

        [FANG_TEXTURE_PLASTICANNON_HUD] = {
            .path = NULL,
            .type = OTHER_TEXTURE,
        },

        [FANG_TEXTURE_FAZER_HUD] = {
            .path = NULL,
            .type = OTHER_TEXTURE,
        },

        /* Sprites */
        [FANG_TEXTURE_AMMO] = {
            .path = "Sprites/Ammo.tga",
            .type = OTHER_TEXTURE,
        },

        [FANG_TEXTURE_HEALTH] = {
            .path = "Sprites/Health.tga",
            .type = OTHER_TEXTURE,
        },

        [FANG_TEXTURE_PROJECTILE] = {
            .path = "Sprites/Projectile.tga",
            .type = OTHER_TEXTURE,
        },
    };

    {
        const Info info = texture_info[id];

        if (info.path)
        {
            *result = Fang_LoadTGA(texture_info[id].path);

            if (!Fang_ImageValid(result))
                return 1;
        }
    }

    switch (texture_info[id].type)
    {
        case TILE_TEXTURE:
            assert(result->width  == FANG_TEXTURE_SIZE * 6);
            assert(result->height == FANG_TEXTURE_SIZE);
            break;

        case FONT_TEXTURE:
            /* Fonts have one pixel barriers between each character */
            assert(result->width  == (FANG_FONT_WIDTH + 1) * (127 - '!'));
            assert(result->height == FANG_FONT_HEIGHT);
            break;

        default:
            break;
    }

    return 0;
}

/**
 * Loads all texture types into the textures structure.
 *
 * Returns non-zero if any textures failed to load.
**/
static inline int
Fang_LoadTextures(
    Fang_Textures * const textures)
{
    assert(textures);

    int error = 0;

    for (Fang_TextureId i = 0; i < FANG_NUM_TEXTURES; ++i)
        error |= Fang_LoadTexture(textures, i);

    return error;
}

/**
 * Unloads all textures currently loaded in the texture set.
**/
static inline void
Fang_FreeTextures(
    Fang_Textures * const textures)
{
    assert(textures);

    for (Fang_TextureId i = 0; i < FANG_NUM_TEXTURES; ++i)
        if (textures->textures[i].pixels)
            Fang_FreeTexture(textures, i);
}

/**
 * Retrieves a texture from the loaded textures.
 *
 * If the id is FANG_TEXTURE_NONE or the target image is invalid this will
 * return NULL.
**/
static inline const Fang_Image *
Fang_GetTexture(
    const Fang_Textures  * const textures,
    const Fang_TextureId         id)
{
    assert(textures);

    if (id == FANG_TEXTURE_NONE)
        return NULL;

    assert(id < FANG_NUM_TEXTURES);

    const Fang_Image * const result = &textures->textures[id];

    if (!Fang_ImageValid(result))
        return NULL;

    return result;
}
