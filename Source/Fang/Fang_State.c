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
 * A structure representing the in-game Unix time.
 *
 * An accumulator is included here for use in the fixed-step update loop.
**/
typedef struct Fang_Clock {
    uint32_t time;
    uint32_t accumulator;
} Fang_Clock;

/**
 * The current state of the game.
 *
 * While this structure holds all the necessary data for the game to run, it is
 * not indicative of the game's "save state".
**/
typedef struct Fang_State {
    Fang_Framebuffer framebuffer;
    Fang_Map         map;
    Fang_Textures    textures;
    Fang_Ray         raycast[FANG_WINDOW_SIZE];
    Fang_Clock       clock;
    Fang_Camera      camera;
    Fang_EntityId    player;
    Fang_Interface   interface;
    Fang_Entities    entities;
    Fang_LerpVec2    sway;
    float            bob;
} Fang_State;
