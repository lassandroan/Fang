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

static const float FANG_PROJECTION_RATIO = 1.0f / (1.0f / 2.0f);

enum {
    FANG_CHUNK_SIZE  = 16,
    FANG_CHUNK_COUNT = 1 << 12,
    FANG_CHUNK_ENTITY_CAPACITY = 16,
    FANG_CHUNK_MAX =  (1 << 6) - 1,
    FANG_CHUNK_MIN = -(1 << 6),
};

static const float FANG_GRAVITY        = 9.834f;
static const float FANG_RUN_SPEED      = 8.33f; // 30 km/h ~= 8.33 m/s
static const float FANG_JUMP_SPEED     = 3.0f;
static const float FANG_PLAYER_WIDTH   = 0.35f;
static const float FANG_PLAYER_HEIGHT  = 0.35f;
static const float FANG_PICKUP_WIDTH   = FANG_PLAYER_WIDTH  / 2.0f;
static const float FANG_PICKUP_HEIGHT  = FANG_PLAYER_HEIGHT / 2.0f;
static const float FANG_JUMP_TOLERANCE = FANG_GRAVITY / 6.0f;

static const uint32_t FANG_DELTA_TIME_MS = 10;
static const float    FANG_DELTA_TIME_S = (float)FANG_DELTA_TIME_MS / 1000.0f;

/**
 * A constant representing the width|height of a tile texture.
**/
enum {
    FANG_TEXTURE_SIZE = 128,
};

/**
 * All fonts should be 8x9 in size, with a 1px barrier in between each
 * character.
**/
enum {
    FANG_FONT_HEIGHT = 9,
    FANG_FONT_WIDTH  = 8,
};

enum {
    FANG_RAY_MAX_STEPS = 64,
};

enum {
    FANG_MAX_ENTITIES   = 256,
    FANG_MAX_COLLISIONS = FANG_MAX_ENTITIES * 64,
};
