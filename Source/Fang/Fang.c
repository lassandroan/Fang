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
#include "Fang_Lerp.c"
#include "Fang_Matrix.c"
#include "Fang_Input.c"
#include "Fang_Image.c"
#include "Fang_TGA.c"
#include "Fang_Framebuffer.c"
#include "Fang_Texture.c"
#include "Fang_Tile.c"
#include "Fang_Map.c"
#include "Fang_Body.c"
#include "Fang_Camera.c"
#include "Fang_DDA.c"
#include "Fang_Ray.c"
#include "Fang_Weapon.c"
#include "Fang_Entity.c"
#include "Fang_Render.c"
#include "Fang_Interface.c"
#include "Fang_State.c"
#include "Fang_Pickups.c"
#include "Fang_Projectiles.c"
#include "Fang_Player.c"

Fang_State gamestate;

static inline void
Fang_Init(void)
{
    for (Fang_Texture i = 0; i < FANG_NUM_TEXTURES; ++i)
        Fang_TextureSetLoad(&gamestate.textures, i);

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
        .pos = {.x = 2, .y = 2},
        .dir = {.x = -1.0f},
        .cam = {.y =  0.5f},
    };

    gamestate.player = Fang_PlayerCreate(
        &gamestate.entities,
        FANG_INPUT_ONE,
        (Fang_Vec3){.x =  2.0f, .y = 2.0f},
        (Fang_Vec3){.x = -1.0f}
    );

    gamestate.sway.delta = (Fang_Vec2){.x = 0.1f, .y = 0.1f};

    Fang_EntitySetAdd(
        &gamestate.entities,
        (Fang_Entity){
            .type = FANG_ENTITYTYPE_PLAYER,
            .body = (Fang_Body){
                .pos = (Fang_Vec3){.x = 4.0f, .y = 4.0f},
                .dir = {.x = -1.0f},
                .width  = FANG_PLAYER_WIDTH,
                .height = FANG_PLAYER_HEIGHT,
                .flags  = FANG_BODYFLAG_COLLIDE_BODIES,
            }
        }
    );

    Fang_AmmoCreate(
        &gamestate.entities,
        FANG_WEAPONTYPE_PISTOL,
        10,
        (Fang_Vec3){.x = 6.0f, .y = 4.0f}
    );

    Fang_HealthCreate(
        &gamestate.entities,
        10,
        (Fang_Vec3){.x = 6.0f, .y = 5.5f}
    );

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

    const Fang_Rect viewport = Fang_FramebufferGetViewport(framebuf);

    Fang_Entity * const player = Fang_EntitySetQuery(
        &gamestate.entities, gamestate.player
    );

    gamestate.sway.target = (Fang_Vec2){.x = 0.0f, .y = 0.0f};

    if (player)
    {
        float forward = 0.0f;
        float left    = 0.0f;
        float up      = 0.0f;

        if (input->controller.direction_up.pressed)
            forward += FANG_RUN_SPEED;

        if (input->controller.direction_down.pressed)
            forward -= FANG_RUN_SPEED;

        if (input->controller.direction_left.pressed)
            left += FANG_RUN_SPEED;

        if (input->controller.direction_right.pressed)
            left -= FANG_RUN_SPEED;

        if (Fang_InputPressed(&input->controller.action_down))
            up = FANG_JUMP_SPEED;

        if (input->mouse.left.pressed)
        {
            Fang_PlayerFireWeapon(
                player,
                &gamestate.entities,
                Fang_InputPressed(&input->mouse.left)
            );
        }

        Fang_WeaponType * const weapon_type = &player->props.player.weapon;

        if (Fang_InputPressed(&input->controller.shoulder_left))
        {
            if (*weapon_type == FANG_WEAPONTYPE_NONE)
                *weapon_type = FANG_WEAPONTYPE_FAZER;
            else if (*weapon_type == FANG_WEAPONTYPE_PISTOL)
                *weapon_type = FANG_WEAPONTYPE_NONE;
            else
                (*weapon_type)--;
        }

        if (Fang_InputPressed(&input->controller.shoulder_right))
        {
            if (*weapon_type == FANG_WEAPONTYPE_FAZER)
                *weapon_type = FANG_WEAPONTYPE_NONE;
            else if (*weapon_type == FANG_WEAPONTYPE_NONE)
                *weapon_type = FANG_WEAPONTYPE_PISTOL;
            else
                (*weapon_type)++;
        }

        left    -= input->controller.joystick_left.x * FANG_RUN_SPEED;
        forward -= input->controller.joystick_left.y * FANG_RUN_SPEED;

        const float prev_pitch = gamestate.camera.cam.z;

        Fang_CameraRotate(
            &gamestate.camera,
            ((float)input->mouse.relative.x / (FANG_WINDOW_SIZE / 2.0f))
            + (input->controller.joystick_right.x /  10.0f),
            ((float)input->mouse.relative.y / -(FANG_WINDOW_SIZE / 2.0f))
            + (input->controller.joystick_right.y / -10.0f)
        );

        player->body.dir = gamestate.camera.dir;

        /* Sway based on player velocity */
        gamestate.sway.target.x += player->body.vel.value.y / 8.0f;
        gamestate.sway.target.y += player->body.vel.value.x / 16.0f;
        gamestate.sway.target.y += player->body.vel.value.z / 2.0f;

        /* Sway based on camera movement */
        gamestate.sway.target.x -= (input->mouse.relative.x / 8);
        gamestate.sway.target.x -= input->controller.joystick_right.x;

        if (fabsf(prev_pitch - gamestate.camera.cam.z) > FLT_EPSILON)
        {
            gamestate.sway.target.y -= (input->mouse.relative.y / 8);
            gamestate.sway.target.y -= input->controller.joystick_right.y;
        }

        /* Bob if player is moving on a surface */
        if (player->body.vel.value.z == 0.0f
        && (forward != 0.0f || left != 0.0f))
        {
            gamestate.bob += ((float)M_PI / 20.0f);
            gamestate.sway.target.x += cosf(gamestate.bob) * 0.5f;
            gamestate.sway.target.y += fabsf(sinf(gamestate.bob)) * 0.5f;
        }

        player->body.vel.target = Fang_Vec3Translate(
            player->body.dir, forward, left, up
        );
    }

    {
        if (!gamestate.clock.time)
            gamestate.clock.time = time;

        const uint32_t frame_time = time - gamestate.clock.time;

        gamestate.clock.time = time;
        gamestate.clock.accumulator += frame_time;

        while (gamestate.clock.accumulator >= FANG_DELTA_TIME_MS)
        {
            for (Fang_EntityId i = 0; i < FANG_MAX_ENTITIES; ++i)
            {
                Fang_Entity * const entity = Fang_EntitySetQuery(
                    &gamestate.entities, i
                );

                if (!entity)
                    continue;

                Fang_BodyMove(&entity->body, &gamestate.map, FANG_DELTA_TIME_S);
            }

            Fang_EntitySetResolveCollisions(
                &gamestate.entities,
                &gamestate.map
            );

            // Placeholder for entity update loop
            for (Fang_EntityId i = 0; i < FANG_MAX_ENTITIES; ++i)
            {
                Fang_Entity * const entity = Fang_EntitySetQuery(
                    &gamestate.entities, i
                );

                if (!entity)
                    continue;

                switch (entity->type)
                {
                    case FANG_ENTITYTYPE_PLAYER:
                        Fang_PlayerUpdate(&gamestate, entity, FANG_DELTA_TIME_MS);
                        break;

                    case FANG_ENTITYTYPE_AMMO:
                        Fang_AmmoUpdate(&gamestate, entity);
                        break;

                    case FANG_ENTITYTYPE_HEALTH:
                        Fang_HealthUpdate(&gamestate, entity);
                        break;

                    case FANG_ENTITYTYPE_PROJECTILE:
                        Fang_ProjectileUpdate(&gamestate, entity);
                        break;
                }
            }

            if (player)
            {
                gamestate.camera.pos = (Fang_Vec3){
                    .x = player->body.pos.x,
                    .y = player->body.pos.y,
                    .z = player->body.pos.z + player->body.height,
                };

                Fang_Lerp(&gamestate.sway);
            }

            gamestate.clock.accumulator -= FANG_DELTA_TIME_MS;
        }
    }

    {
        gamestate.interface.input    = input;
        gamestate.interface.framebuf = framebuf;
        Fang_InterfaceUpdate(&gamestate.interface);
    }

    Fang_ImageClear(&framebuf->color);

    for (int x = 0; x < viewport.w; ++x)
    {
        for (int y = 0; y < viewport.h; ++y)
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
        &gamestate.textures,
        &gamestate.map,
        &gamestate.entities
    );

    Fang_FramebufferShadeDepth(
        framebuf, &gamestate.map.fog, gamestate.map.fog_distance
    );

    framebuf->state.enable_depth = false;

    if (player)
    {
        const Fang_Weapon * const weapon = Fang_WeaponQuery(
            player->props.player.weapon
        );

        if (weapon)
        {
            const Fang_Image * const weapon_texture = Fang_TextureSetQuery(
                &gamestate.textures, weapon->texture
            );

            if (weapon_texture)
            {
                const Fang_Point offset = {
                    .x = (int)roundf(
                        clamp(gamestate.sway.value.x, -1.0f, 1.0f) * 20
                    ),
                    .y = (int)roundf(
                        clamp(gamestate.sway.value.y, -1.0f, 1.0f) * 20
                    ) + 20,
                };

                Fang_DrawImage(
                    framebuf,
                    weapon_texture,
                    NULL,
                    &(Fang_Rect){
                        .x = offset.x,
                        .y = offset.y,
                        .w = viewport.w,
                        .h = viewport.h
                    }
                );
            }

            Fang_DrawText(
                framebuf,
                weapon->name,
                Fang_TextureSetQuery(&gamestate.textures, FANG_TEXTURE_FORMULA),
                FANG_FONT_HEIGHT,
                &(Fang_Point){.x = 5, .y = 3}
            );

            char ammo_count[4];
            snprintf(
                ammo_count,
                sizeof(ammo_count),
                "%03d",
                player->props.player.ammo[player->props.player.weapon]
            );

            Fang_DrawText(
                framebuf,
                ammo_count,
                Fang_TextureSetQuery(&gamestate.textures, FANG_TEXTURE_FORMULA),
                FANG_FONT_HEIGHT,
                &(Fang_Point){.x = 5, .y = 3 + FANG_FONT_HEIGHT}
            );
        }

        char health[4] = "000";

        if (player)
        {
            snprintf(
                health,
                sizeof(health),
                "%3d",
                player->props.player.health
            );
        }

        const int text_offset = 3 - (int)strlen(health);

        Fang_DrawText(
            framebuf,
            health,
            Fang_TextureSetQuery(&gamestate.textures, FANG_TEXTURE_FORMULA),
            FANG_FONT_HEIGHT,
            &(Fang_Point){
                .x = viewport.w - 5 - (FANG_FONT_WIDTH * (3 - text_offset)),
                .y = 3
            }
        );
    }

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

    Fang_FramebufferPutPixel(
        framebuf,
        &(Fang_Point){
            .x = viewport.w / 2,
            .y = viewport.h / 2,
        },
        &(Fang_Color){
            .r = 255,
            .g = 255,
            .b = 255,
            .a = 128,
        }
    );
}

static inline void
Fang_Quit(void)
{
    Fang_TextureSetFree(&gamestate.textures);
}
