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
 * The available weapon types in the game.
**/
typedef enum Fang_WeaponType {
    FANG_WEAPONTYPE_PISTOL,
    FANG_WEAPONTYPE_CARBINE,
    FANG_WEAPONTYPE_FLAKGUN,
    FANG_WEAPONTYPE_CHAINGUN,
    FANG_WEAPONTYPE_LRAD,
    FANG_WEAPONTYPE_PLASTICANNON,
    FANG_WEAPONTYPE_FAZER,

    FANG_NUM_WEAPONTYPE,
    FANG_WEAPONTYPE_NONE,
} Fang_WeaponType;

/**
 * A structure detailing the properties of an in-game weapon.
 *
 * Weapons have various properties that determine their behavior when firing,
 * such as how quickly they can fire, whether they fire automatically, how much
 * damage or speed the projectile should have, and how long the projectile
 * should stay active before de-spawning.
 *
 * The texture of a weapon controls the image displayed in the first person
 * view. If no texture has been assigned then the weapon will not render in the
 * HUD.
**/
typedef struct Fang_Weapon {
    char           * name;
    Fang_TextureId   texture;
    int              damage;
    bool             automatic;
    uint32_t         cooldown;
    float            speed;
    uint32_t         lifespan;
} Fang_Weapon;

/**
 * Retrieves weapon details given a weapon-id.
 *
 * If the type is 'none' this will return NULL.
**/
static inline const Fang_Weapon *
Fang_GetWeapon(
    const Fang_WeaponType type)
{
    if (type == FANG_WEAPONTYPE_NONE)
        return NULL;

    assert(type < FANG_NUM_WEAPONTYPE);

    static const Fang_Weapon weapons[FANG_NUM_WEAPONTYPE] = {
        [FANG_WEAPONTYPE_PISTOL] = (Fang_Weapon){
            .name      = "Pistol",
            .damage    = 10,
            .texture   = FANG_TEXTURE_PISTOL_HUD,
            .automatic = false,
            .cooldown  = 250,
            .speed     = 10.0f,
            .lifespan  = 1000,
        },
        [FANG_WEAPONTYPE_CARBINE] = (Fang_Weapon){
            .name      = "Cast-Iron Carbine",
            .damage    = 10,
            .texture   = FANG_TEXTURE_CARBINE_HUD,
            .automatic = false,
            .cooldown  = 1000,
            .speed     = 20.0f,
            .lifespan  = 1000,
        },
        [FANG_WEAPONTYPE_FLAKGUN] = (Fang_Weapon){
            .name      = "Flakgun",
            .damage    = 10,
            .texture   = FANG_TEXTURE_FLAKGUN_HUD,
            .automatic = false,
            .cooldown  = 500,
            .speed     = 5.0f,
            .lifespan  = 1000,
        },
        [FANG_WEAPONTYPE_CHAINGUN] = (Fang_Weapon){
            .name      = "Chaingun",
            .damage    = 10,
            .texture   = FANG_TEXTURE_CHAINGUN_HUD,
            .automatic = true,
            .cooldown  = 50,
            .speed     = 15.0f,
            .lifespan  = 1000,
        },
        [FANG_WEAPONTYPE_LRAD] = (Fang_Weapon){
            .name      = "LRAD",
            .damage    = 10,
            .texture   = FANG_TEXTURE_LRAD_HUD,
            .automatic = false,
            .cooldown  = 3000,
            .speed     = 10.0f,
            .lifespan  = 1000,
        },
        [FANG_WEAPONTYPE_PLASTICANNON] = (Fang_Weapon){
            .name      = "Plasti-cannon",
            .damage    = 10,
            .texture   = FANG_TEXTURE_PLASTICANNON_HUD,
            .automatic = false,
            .cooldown  = 1000,
            .speed     = 3.5f,
            .lifespan  = 10000,
        },
        [FANG_WEAPONTYPE_FAZER] = (Fang_Weapon){
            .name      = "Fazer",
            .damage    = 10,
            .texture   = FANG_TEXTURE_FAZER_HUD,
            .automatic = true,
            .cooldown  = 100,
            .speed     = 17.5f,
            .lifespan  = 1000,
        },
    };

    return &weapons[type];
}
