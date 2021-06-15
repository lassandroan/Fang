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

#include <assert.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef min
  #define min(x, y) (x < y ? x : y)
#endif

#ifndef max
  #define max(x, y) (x > y ? x : y)
#endif

#ifndef clamp
  #define clamp(x, low, high) (max(min(x, high), low))
#endif

#ifndef breakpoint
  #define breakpoint() raise(SIGTRAP)
#endif

#include "Fang_Memory.c"
#include "Fang_Color.c"
#include "Fang_Rect.c"
#include "Fang_Vector.c"
#include "Fang_Matrix.c"
#include "Fang_Input.c"
#include "Fang_Image.c"
#include "Fang_TGA.c"
#include "Fang_Font.c"
#include "Fang_Framebuffer.c"
#include "Fang_Camera.c"
#include "Fang_Tile.c"
#include "Fang_Map.c"
#include "Fang_Ray.c"
#include "Fang_Render.c"
#include "Fang_Interface.c"

Fang_Interface interface = (Fang_Interface){
    .theme = (Fang_InterfaceTheme){
        .font = FANG_FONT_FORMULA,
        .colors = (Fang_InterfaceColors){
            .background = FANG_TRANSPARENT,
            .foreground = FANG_RED,
            .highlight  = FANG_WHITE,
            .disabled   = FANG_GREY,
        },
    },
};

Fang_Ray raycast[FANG_WINDOW_SIZE];

Fang_Camera camera = (Fang_Camera){
    .pos = {
        .x = 32 * 2,
        .y = 32 * 2,
        .z = 32 / 2,
    },
    .dir = {.x = -1.0f},
    .cam = {.y =  0.5f},
};

static inline void
Fang_UpdateAndRender(
    const Fang_Input       * const input,
          Fang_Framebuffer * const framebuf)
{
    assert(input);
    assert(framebuf);

    interface.input    = input;
    interface.framebuf = framebuf;
    Fang_InterfaceUpdate(&interface);

    Fang_FramebufferClear(framebuf);

    Fang_CameraRotate(
        &camera,
        0.015f,
        0
    );

    Fang_RayCast(
        &temp_map,
        &camera,
        raycast,
        (size_t)FANG_WINDOW_SIZE
    );

    Fang_DrawMap(
        &temp_map,
        framebuf,
        &camera,
        raycast,
        (size_t)FANG_WINDOW_SIZE
    );

    Fang_DrawMinimap(
        &temp_map,
        framebuf,
        &camera,
        raycast,
        (size_t)FANG_WINDOW_SIZE,
        &(Fang_Rect){
            .x = FANG_WINDOW_SIZE - 32,
            .y = FANG_WINDOW_SIZE - 32,
            .w = 32,
            .h = 32,
        }
    );

    Fang_DrawText(
        framebuf,
        "FANG",
        FANG_FONT_FORMULA,
        FANG_FONTAREA_HEIGHT,
        &(Fang_Point){.x = 5, .y = 3}
    );
}
