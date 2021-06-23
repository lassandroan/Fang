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
#include <float.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Fang_Defines.c"
#include "Fang_Macros.c"
#include "Fang_File.c"
#include "Fang_Color.c"
#include "Fang_Rect.c"
#include "Fang_Vector.c"
#include "Fang_Matrix.c"
#include "Fang_Input.c"
#include "Fang_Image.c"
#include "Fang_TGA.c"
#include "Fang_Framebuffer.c"
#include "Fang_Body.c"
#include "Fang_Camera.c"
#include "Fang_Tile.c"
#include "Fang_Texture.c"
#include "Fang_Map.c"
#include "Fang_Ray.c"
#include "Fang_Entity.c"
#include "Fang_Render.c"
#include "Fang_Interface.c"

Fang_Interface interface = (Fang_Interface){
    .theme = (Fang_InterfaceTheme){
        .font = FANG_TEXTURE_FORMULA,
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

enum {
    FANG_NUM_ENTITIES = 2,
};

Fang_Entity entities[FANG_NUM_ENTITIES] = {
    [0] = (Fang_Entity){
        (Fang_Body){
            .pos  = (Fang_Vec2){.x = 32, .y = 32},
            .size = 16,
        },
    },
    [1] = (Fang_Entity){
        (Fang_Body){
            .pos  = (Fang_Vec2){.x = 96, .y = 84},
            .size = 16,
        },
    },
};

static inline int
Fang_Initialize(void)
{
    if (Fang_LoadMap())
        return 1;

    return 0;
}

static inline void
Fang_Quit(void)
{
    Fang_DestroyMap();
}

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

    Fang_ImageClear(&framebuf->color);

    for (int x = 0; x < framebuf->depth.width; ++x)
    {
        for (int y = 0; y < framebuf->depth.height; ++y)
        {
            float * const depth = (float*)(
                framebuf->depth.pixels
              + (x * framebuf->depth.stride)
              + (y * framebuf->depth.pitch)
            );

            *depth = FLT_MAX;
        }
    }

    framebuf->state.current_depth = 0.0f;
    framebuf->state.enable_depth  = true;

    Fang_CameraRotate(
        &camera,
        0.0075f,
        0
    );

    Fang_RayCast(
        &temp_map,
        &camera,
        raycast,
        (size_t)FANG_WINDOW_SIZE
    );

    Fang_DrawMap(
        framebuf,
        &temp_map,
        &camera,
        raycast,
        (size_t)FANG_WINDOW_SIZE
    );

    Fang_DrawEntities(
        framebuf,
        &temp_map,
        &camera,
        entities,
        FANG_NUM_ENTITIES
    );

    {
        const Fang_FrameState state = Fang_FramebufferSetViewport(
            framebuf,
            &(Fang_Rect){
                .x = FANG_WINDOW_SIZE - 32,
                .y = FANG_WINDOW_SIZE - 32,
                .w = 32,
                .h = 32,
            }
        );

        framebuf->state.enable_depth = false;

        Fang_DrawMinimap(
            framebuf,
            &temp_map,
            &camera,
            raycast,
            (size_t)FANG_WINDOW_SIZE
        );

        framebuf->state = state;
    }

    framebuf->state.current_depth = 0.0f;

    Fang_DrawText(
        framebuf,
        "FANG",
        Fang_AtlasQuery(&temp_map.textures, FANG_TEXTURE_FORMULA),
        FANG_FONT_HEIGHT,
        &(Fang_Point){.x = 5, .y = 3}
    );
}
