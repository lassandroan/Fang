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
#include "Fang_Constants.c"
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
#include "Fang_Textures.c"
#include "Fang_Body.c"
#include "Fang_Tile.c"
#include "Fang_Camera.c"
#include "Fang_Map.c"
#include "Fang_Ray.c"
#include "Fang_Entity.c"
#include "Fang_Render.c"
#include "Fang_Interface.c"
#include "Fang_State.c"

const uint32_t update_dt = 10;

Fang_State gamestate;

static inline void
Fang_Init(void)
{
    for (Fang_Texture i = 0; i < FANG_NUM_TEXTURES; ++i)
        Fang_AtlasLoad(&gamestate.textures, i);

    gamestate.interface = (Fang_Interface){
        .textures = &gamestate.textures,
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

    gamestate.camera = (Fang_Camera){
        .pos = {
            .x = 2,
            .y = 2,
            .z = 0,
        },
        .dir = {.x = -1.0f},
        .cam = {.y =  0.5f},
    };

    {
        Fang_Entity entities [FANG_MAX_ENTITIES] = {
            [0] = (Fang_Entity){
                (Fang_Body){
                    .pos  = (Fang_Vec2){.x = 2.0f, .y = 2.0f},
                    .size = 1,
                },
            },
            [1] = (Fang_Entity){
                (Fang_Body){
                    .pos  = (Fang_Vec2){.x = 6.0f, .y = 5.5f},
                    .size = 1,
                },
            },
        };

        memcpy(gamestate.entities, entities, sizeof(entities));
    }

    gamestate.map = (Fang_Map){
        .size         = 8,
        .skybox       = FANG_TEXTURE_SKYBOX,
        .floor        = FANG_TEXTURE_FLOOR,
        .fog          = FANG_BLACK,
        .fog_distance = 16.0f,
    };
}

static inline void
Fang_Update(
    const Fang_Input       * const input,
          Fang_Framebuffer * const framebuf,
          uint32_t                 time)
{
    assert(input);
    assert(framebuf);

    {
        if (!gamestate.clock.time)
            gamestate.clock.time = time;

        const uint32_t frame_time = time - gamestate.clock.time;
        gamestate.clock.time = time;
        gamestate.clock.accumulator += frame_time;

        while (gamestate.clock.accumulator >= update_dt)
            gamestate.clock.accumulator -= update_dt;
    }

    {
        gamestate.interface.input    = input;
        gamestate.interface.framebuf = framebuf;
        Fang_InterfaceUpdate(&gamestate.interface);
    }

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

    Fang_RayCast(
        &gamestate.camera,
        &gamestate.map,
        gamestate.raycast,
        (size_t)FANG_WINDOW_SIZE
    );

    Fang_DrawMap(
        framebuf,
        &gamestate.camera,
        &gamestate.textures,
        &gamestate.map,
        gamestate.raycast,
        (size_t)FANG_WINDOW_SIZE
    );

    Fang_DrawEntities(
        framebuf,
        &gamestate.camera,
        gamestate.entities,
        FANG_MAX_ENTITIES
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
            &gamestate.camera,
            &gamestate.map,
            gamestate.raycast,
            (size_t)FANG_WINDOW_SIZE
        );

        framebuf->state = state;
    }

    framebuf->state.current_depth = 0.0f;

    Fang_DrawText(
        framebuf,
        "FANG",
        Fang_AtlasQuery(&gamestate.textures, FANG_TEXTURE_FORMULA),
        FANG_FONT_HEIGHT,
        &(Fang_Point){.x = 5, .y = 3}
    );

    static float height = 0.0f;
    static float pitch  = 0.0f;
    static float rotate = 0.0f;
    static float move   = 0.0f;

    if (Fang_InterfaceSlider(
        &gamestate.interface, &height, "height",
        &(Fang_Rect){.x = 256 - 100, .w = 100, .h = 15}))
    {
        gamestate.camera.pos.z = (height * 2.0f - 1.0f) * (1.0f);
    }

    if (Fang_InterfaceSlider(
        &gamestate.interface, &pitch, "pitch",
        &(Fang_Rect){.x = 256 - 100, .y = 20, .w = 100, .h = 15}))
    {
        gamestate.camera.cam.z = pitch * 2.0f - 1.0f;
    }

    if (Fang_InterfaceSlider(
        &gamestate.interface, &rotate, "rotate",
        &(Fang_Rect){.x = 256 - 100, .y = 40, .w = 100, .h = 15}))
    {
        Fang_CameraRotate(
            &gamestate.camera,
            (rotate * 2.0f - 1.0f) / 100.0f,
            0
        );
    }

    if (Fang_InterfaceSlider(
        &gamestate.interface, &move, "move",
        &(Fang_Rect){.x = 256 - 100, .y = 60, .w = 100, .h = 15}))
    {
        const float vel = (move * 2.0f - 1.0f) * 0.05f;

        Fang_Vec3 * const pos = &gamestate.camera.pos;
        Fang_Vec3 * const dir = &gamestate.camera.dir;

        *pos = (Fang_Vec3){
            .x = pos->x + (dir->x * vel),
            .y = pos->y + (dir->y * vel),
            .z = pos->z,
        };
    }
}

static inline void
Fang_Quit(void)
{
    Fang_AtlasFree(&gamestate.textures);
}
