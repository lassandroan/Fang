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

const float FANG_PROJECTION_RATIO = 1.0f / (1.0f / 2.0f);

const size_t FANG_CHUNK_TILES = 8;
const size_t FANG_CHUNK_COUNT = 256;

const float FANG_GRAVITY        = 9.834f;
const float FANG_RUN_SPEED      = 8.33f; // 30 km/h ~= 8.33 m/s
const float FANG_JUMP_SPEED     = 3.0f;
const float FANG_PLAYER_WIDTH   = 0.35f;
const float FANG_PLAYER_HEIGHT  = 0.35f;
const float FANG_PICKUP_WIDTH   = FANG_PLAYER_WIDTH  / 2.0f;
const float FANG_PICKUP_HEIGHT  = FANG_PLAYER_HEIGHT / 2.0f;
const float FANG_JUMP_TOLERANCE = FANG_GRAVITY / 6.0f;

const uint32_t FANG_DELTA_TIME_MS = 10;
const float    FANG_DELTA_TIME_S = (float)FANG_DELTA_TIME_MS / 1000.0f;
