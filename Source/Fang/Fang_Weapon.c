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

typedef struct Fang_Weapon {
    const char * const name;
    Fang_Texture texture;
} Fang_Weapon;

static inline const Fang_Weapon *
Fang_WeaponQuery(
    const Fang_WeaponType type)
{
    if (type == FANG_WEAPONTYPE_NONE)
        return NULL;

    assert(type < FANG_NUM_WEAPONTYPE);

    static const Fang_Weapon weapons[FANG_NUM_WEAPONTYPE] = {
        [FANG_WEAPONTYPE_PISTOL] = (Fang_Weapon){
            .name = "Pistol",
            .texture = FANG_TEXTURE_PISTOL_HUD,
        },
        [FANG_WEAPONTYPE_CARBINE] = (Fang_Weapon){
            .name = "Cast-Iron Carbine",
            .texture = FANG_TEXTURE_CARBINE_HUD,
        },
        [FANG_WEAPONTYPE_FLAKGUN] = (Fang_Weapon){
            .name = "Flakgun",
            .texture = FANG_TEXTURE_FLAKGUN_HUD,
        },
        [FANG_WEAPONTYPE_CHAINGUN] = (Fang_Weapon){
            .name = "Chaingun",
            .texture = FANG_TEXTURE_CHAINGUN_HUD,
        },
        [FANG_WEAPONTYPE_LRAD] = (Fang_Weapon){
            .name = "LRAD",
            .texture = FANG_TEXTURE_LRAD_HUD,
        },
        [FANG_WEAPONTYPE_PLASTICANNON] = (Fang_Weapon){
            .name = "Plasti-cannon",
            .texture = FANG_TEXTURE_PLASTICANNON_HUD,
        },
        [FANG_WEAPONTYPE_FAZER] = (Fang_Weapon){
            .name = "Fazer",
            .texture = FANG_TEXTURE_FAZER_HUD,
        },
    };

    return &weapons[type];
}