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
 * The tile types that can exist in the game world.
 *
 * If a tile has a type of 'none' it effectively does not exist in the world and
 * cannot be collided with or interacted with.
**/
typedef enum Fang_TileType {
    FANG_TILETYPE_NONE,
    FANG_TILETYPE_SOLID,
    FANG_TILETYPE_FLOATING,

    FANG_NUM_TILETYPE,
} Fang_TileType;

/**
 * A structure representing the core geometry of the game world, measuring 1.0^2
 * unit in size (ignoring Z).
 *
 * Tiles may have an associated 'type' which dictates how the game engine should
 * handle it. If the tile-type is 'none' then effectively the tile does not
 * exist and cannot be collided with or interacted with.
 *
 * Each tile may have a texture associated with it that is used to render the
 * proper image for each given side of the tile. If the tile does not have a
 * texture set then it is not rendered, but may still be collided with or
 * interacted with.
 *
 * Though tiles have a fixed size along the X/Y axes, they have 'offset' and
 * 'height' properties which dictate their effective Z axes attributes. Offset
 * determines how high off the ground the tile (from its bottom face) is, while
 * height determines the size of the tile between its bottom and top faces.
**/
typedef struct Fang_Tile {
    Fang_TileType  type;
    Fang_TextureId texture;
    float          offset;
    float          height;
} Fang_Tile;
