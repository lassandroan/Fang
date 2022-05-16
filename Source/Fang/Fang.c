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
#include "Fang_Chunk.c"
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
    Fang_AllocImage(
        &gamestate.framebuffer.color,
        FANG_WINDOW_SIZE,
        FANG_WINDOW_SIZE,
        32
    );

    Fang_AllocImage(
        &gamestate.framebuffer.depth,
        FANG_WINDOW_SIZE,
        FANG_WINDOW_SIZE,
        32
    );

    assert(Fang_ImageValid(&gamestate.framebuffer.color));
    assert(Fang_ImageValid(&gamestate.framebuffer.depth));

    gamestate.framebuffer.state.current_depth = 0.0f;
    gamestate.framebuffer.state.enable_depth  = true;
    gamestate.framebuffer.state.transform     = Fang_IdentityMatrix();

    Fang_LoadTextures(&gamestate.textures);

    {
        Fang_Chunk * const chunk = (Fang_Chunk*)Fang_GetIndexedChunk(
            &gamestate.map.chunks, 0, 0
        );

        chunk->floor = FANG_TEXTURE_FLOOR;

        Fang_Tile * tile = &chunk->tiles[0][0];
        tile->type    = FANG_TILETYPE_SOLID;
        tile->height  = 1.0f;
        tile->texture = FANG_TEXTURE_TILE;

        tile = &chunk->tiles[7][3];
        tile->type    = FANG_TILETYPE_SOLID;
        tile->height  = 0.5f;
        tile->texture = FANG_TEXTURE_TILE;

        for (int x = 0; x < FANG_CHUNK_SIZE; ++x)
        {
            tile = &chunk->tiles[x][0];
            tile->type    = FANG_TILETYPE_SOLID;
            tile->height  = 0.5f;
            tile->texture = FANG_TEXTURE_TILE;

            tile = &chunk->tiles[x][FANG_CHUNK_SIZE - 1];
            tile->type    = FANG_TILETYPE_SOLID;
            tile->height  = 0.5f;
            tile->texture = FANG_TEXTURE_TILE;

            tile = &chunk->tiles[0][x];
            tile->type    = FANG_TILETYPE_SOLID;
            tile->height  = 0.5f;
            tile->texture = FANG_TEXTURE_TILE;

            tile = &chunk->tiles[FANG_CHUNK_SIZE - 1][x];
            tile->type    = FANG_TILETYPE_SOLID;
            tile->height  = 0.5f;
            tile->texture = FANG_TEXTURE_TILE;
        }
    }

    {
        Fang_Chunk * const chunk = (Fang_Chunk*)Fang_GetIndexedChunk(
            &gamestate.map.chunks, -2, 0
        );

        chunk->floor = FANG_TEXTURE_SKYBOX;

        Fang_Tile * tile = &chunk->tiles[3][3];
        tile->type    = FANG_TILETYPE_SOLID;
        tile->height  = 0.25f;
        tile->texture = FANG_TEXTURE_TILE;

        tile = &chunk->tiles[4][4];
        tile->type    = FANG_TILETYPE_SOLID;
        tile->height  = 2.0f;
        tile->texture = FANG_TEXTURE_TILE;
    }

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

    gamestate.player = Fang_CreatePlayer(
        &gamestate.entities,
        FANG_INPUT_ONE,
        (Fang_Vec3){.x =  2.0f, .y = 2.0f},
        (Fang_Vec3){.x = -1.0f}
    );

    gamestate.sway.delta = (Fang_Vec2){.x = 0.1f, .y = 0.1f};

    Fang_AddEntity(
        &gamestate.entities,
        &(Fang_Entity){
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

    Fang_CreateAmmo(
        &gamestate.entities,
        FANG_WEAPONTYPE_PISTOL,
        10,
        (Fang_Vec3){.x = 6.0f, .y = 4.0f}
    );

    Fang_CreateHealth(
        &gamestate.entities,
        10,
        (Fang_Vec3){.x = 6.0f, .y = 5.5f}
    );

    gamestate.map.skybox       = FANG_TEXTURE_SKYBOX;
    gamestate.map.floor        = FANG_TEXTURE_FLOOR;
    gamestate.map.fog          = FANG_BLACK;
    gamestate.map.fog_distance = FANG_CHUNK_SIZE * 2.0f;
}

static inline const Fang_Image *
Fang_Update(
    const Fang_Input * const input,
          uint32_t           time)
{
    assert(input);

    const Fang_Rect viewport = Fang_GetViewport(&gamestate.framebuffer);

    Fang_Entity * const player = Fang_GetEntity(
        &gamestate.entities, gamestate.player
    );

    gamestate.sway.target = (Fang_Vec2){.x = 0.0f, .y = 0.0f};

    if (player && player->state == FANG_ENTITYSTATE_ACTIVE)
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

        const float prev_pitch = gamestate.camera.dir.z;

        {
            const Fang_Vec2 mouse_rotate = {
                .x = input->mouse.relative.x /  (FANG_WINDOW_SIZE / 2.0f),
                .y = input->mouse.relative.y / -(FANG_WINDOW_SIZE / 2.0f),
            };

            const Fang_Vec2 joystick_rotate = {
                .x = input->controller.joystick_right.x /  10.0f,
                .y = input->controller.joystick_right.y / -10.0f,
            };

            Fang_RotateCamera(
                &gamestate.camera,
                mouse_rotate.x + joystick_rotate.x,
                mouse_rotate.y + joystick_rotate.y
            );

            player->body.dir = (Fang_Vec3){
                .x = gamestate.camera.dir.x,
                .y = gamestate.camera.dir.y,
                .z = gamestate.camera.dir.z / FANG_PROJECTION_RATIO,
            };
        }

        /* Sway based on player velocity */
        gamestate.sway.target.x += player->body.vel.value.y / 8.0f;
        gamestate.sway.target.y += player->body.vel.value.x / 16.0f;
        gamestate.sway.target.y += player->body.vel.value.z / 2.0f;

        /* Sway based on camera movement */
        gamestate.sway.target.x -= (input->mouse.relative.x / 8);
        gamestate.sway.target.x -= input->controller.joystick_right.x;

        if (fabsf(prev_pitch - gamestate.camera.dir.z) > FLT_EPSILON)
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

        Fang_SetTargetVelocity(&player->body, forward, left);
        player->body.vel.target.z = up;
    }

    {
        if (!gamestate.clock.time)
            gamestate.clock.time = time;

        const uint32_t frame_time = time - gamestate.clock.time;

        gamestate.clock.time = time;
        gamestate.clock.accumulator += frame_time;

        while (gamestate.clock.accumulator >= FANG_DELTA_TIME_MS)
        {
            // Update entity body positions
            for (Fang_EntityId i = 0; i < FANG_MAX_ENTITIES; ++i)
            {
                Fang_Entity * const entity = Fang_GetEntity(
                    &gamestate.entities, i
                );

                if (!entity)
                    continue;

                Fang_UpdateBody(
                    &entity->body,
                    &gamestate.map.chunks,
                    FANG_DELTA_TIME_S
                );
            }

            // Soft-reset location tables
            for (size_t i = 0; i < FANG_CHUNK_COUNT; ++i)
                gamestate.map.chunks.chunks[i].entities.count = 0;

            // Update location table entries
            for (Fang_EntityId i = 0; i < FANG_MAX_ENTITIES; ++i)
            {
                Fang_Entity * const entity = Fang_GetEntity(
                    &gamestate.entities, i
                );

                if (!entity)
                    continue;

                Fang_Chunk * const chunk = (Fang_Chunk*)Fang_GetChunk(
                    &gamestate.map.chunks, &entity->body.pos
                );

                assert(chunk->entities.count <= FANG_CHUNK_ENTITY_CAPACITY - 1);
                chunk->entities.entities[chunk->entities.count++] = entity->id;
            }

            // Check entity-tile collisions
            for (Fang_EntityId i = 0; i < FANG_MAX_ENTITIES; ++i)
            {
                Fang_Entity * const entity = Fang_GetEntity(
                    &gamestate.entities, i
                );

                if (!entity)
                    continue;

                const bool collides_tile = Fang_ResolveTileCollision(
                    &entity->body, &gamestate.map.chunks
                );

                if (collides_tile)
                {
                    switch (entity->type)
                    {
                        case FANG_ENTITYTYPE_PLAYER:
                            Fang_PlayerCollideMap(entity);
                            break;

                        case FANG_ENTITYTYPE_AMMO:
                            Fang_AmmoCollideMap(entity);
                            break;

                        case FANG_ENTITYTYPE_HEALTH:
                            Fang_HealthCollideMap(entity);
                            break;

                        case FANG_ENTITYTYPE_PROJECTILE:
                            Fang_ProjectileCollideMap(entity);
                            break;

                        default:
                            break;
                    };
                }
            }

            // Check entity-entity collisions
            for (Fang_EntityId i = 0; i < FANG_MAX_ENTITIES; ++i)
            {
                const Fang_Entity * const entity = Fang_GetEntity(
                    &gamestate.entities, i
                );

                if (!entity)
                    continue;

                Fang_Chunk * const chunk = (Fang_Chunk*)Fang_GetChunk(
                    &gamestate.map.chunks, &entity->body.pos
                );

                if (i == 0)
                // {
                //     assert(entity->type == FANG_ENTITYTYPE_PLAYER);

                //     printf("Other entities in your chunk:\n");
                // }

                for (size_t j = 0; j < chunk->entities.count; ++j)
                {
                    if (chunk->entities.entities[j] == entity->id)
                        continue;

                    const Fang_Entity * const other = Fang_GetEntity(
                        &gamestate.entities, chunk->entities.entities[j]
                    );

                    if (!other)
                        continue;

                    // if (i == 0)
                    // {
                    //     const char * name;
                    //     switch (other->type)
                    //     {
                    //         case FANG_ENTITYTYPE_PLAYER:     name = "Player"; break;
                    //         case FANG_ENTITYTYPE_PROJECTILE: name = "Projectile"; break;
                    //         case FANG_ENTITYTYPE_AMMO:       name = "Ammo"; break;
                    //         case FANG_ENTITYTYPE_HEALTH:     name = "Health"; break;
                    //     }

                    //     printf("\t%zu - %s\n", other->id, name);
                    // }

                    if (Fang_BodiesIntersect(&entity->body, &other->body))
                    {
                        Fang_AddEntityCollision(
                            &gamestate.entities.collisions,
                            (Fang_EntityCollision){entity->id, other->id}
                        );
                    }
                }
            }

            // Resolve collisions
            {
                Fang_EntityCollisions * const collisions = (
                    &gamestate.entities.collisions
                );

                Fang_EntityCollisions * const last_collisions = (
                    &gamestate.entities.last_collisions
                );

                for (size_t i = 0; i < collisions->count; ++i)
                {
                    const Fang_EntityCollision collision = (
                        collisions->collisions[i]
                    );

                    Fang_Entity * const first = Fang_GetEntity(
                        &gamestate.entities, collision.first
                    );

                    if (!first || first->state != FANG_ENTITYSTATE_ACTIVE)
                        continue;

                    Fang_Entity * const second = Fang_GetEntity(
                        &gamestate.entities, collision.second
                    );

                    if (!second || second->state != FANG_ENTITYSTATE_ACTIVE)
                        continue;

                    bool initial_collision = true;
                    for (size_t j = 0; j < last_collisions->count; ++j)
                    {
                        const Fang_EntityCollision last_collision = (
                            last_collisions->collisions[j]
                        );

                        if (last_collision.first  == collision.first
                        &&  last_collision.second == collision.second)
                        {
                            initial_collision = false;
                            break;
                        }

                        if (last_collision.first  == collision.second
                        &&  last_collision.second == collision.first)
                        {
                            initial_collision = false;
                            break;
                        }
                    }

                    if (initial_collision)
                    {
                        assert(last_collisions->count <= FANG_MAX_COLLISIONS - 1);
                        last_collisions->collisions[last_collisions->count++] = collision;
                    }

                    Fang_ResolveBodyCollision(&first->body, &second->body);

                    for (size_t i = 0; i < 2; ++i)
                    {
                        Fang_Entity * const entity = (i == 0) ?  first : second;
                        Fang_Entity * const  other = (i == 0) ? second :  first;

                        switch (entity->type)
                        {
                            case FANG_ENTITYTYPE_PLAYER:
                                Fang_PlayerCollideEntity(
                                    entity, other, initial_collision
                                );
                                break;

                            case FANG_ENTITYTYPE_AMMO:
                                Fang_AmmoCollideEntity(
                                    entity, other, initial_collision
                                );
                                break;

                            case FANG_ENTITYTYPE_HEALTH:
                                Fang_HealthCollideEntity(
                                    entity, other, initial_collision
                                );
                                break;

                            default:
                                break;
                        };
                    }
                }

                memcpy(last_collisions, collisions, sizeof(*last_collisions));
                memset(collisions, 0, sizeof(*collisions));
            }

            // Run entity update functions
            for (Fang_EntityId i = 0; i < FANG_MAX_ENTITIES; ++i)
            {
                Fang_Entity * const entity = Fang_GetEntity(
                    &gamestate.entities, i
                );

                if (!entity)
                    continue;

                switch (entity->type)
                {
                    case FANG_ENTITYTYPE_PLAYER:
                        Fang_UpdatePlayer(
                            &gamestate, entity, FANG_DELTA_TIME_MS
                        );
                        break;

                    case FANG_ENTITYTYPE_AMMO:
                        Fang_UpdateAmmo(
                            &gamestate, entity, FANG_DELTA_TIME_MS
                        );
                        break;

                    case FANG_ENTITYTYPE_HEALTH:
                        Fang_UpdateHealth(
                            &gamestate, entity, FANG_DELTA_TIME_MS
                        );
                        break;

                    case FANG_ENTITYTYPE_PROJECTILE:
                        Fang_UpdateProjectile(
                            &gamestate, entity, FANG_DELTA_TIME_MS
                        );
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

                Fang_Lerp(&gamestate.sway, FANG_DELTA_TIME_S);
            }

            gamestate.clock.accumulator -= FANG_DELTA_TIME_MS;
        }
    }

    {
        gamestate.interface.input = input;
        Fang_UpdateInterface(&gamestate.interface);
    }

    Fang_ClearImage(&gamestate.framebuffer.color);

    for (int x = 0; x < viewport.w; ++x)
    {
        for (int y = 0; y < viewport.h; ++y)
        {
            float * const depth = (float*)(
                gamestate.framebuffer.depth.pixels
              + (x * gamestate.framebuffer.depth.stride)
              + (y * gamestate.framebuffer.depth.pitch)
            );

            *depth = FLT_MAX;
        }
    }

    Fang_CastRays(
        &gamestate.camera,
        &gamestate.map.chunks,
        gamestate.raycast,
        (size_t)FANG_WINDOW_SIZE
    );

    gamestate.framebuffer.state.current_depth = FLT_MAX;
    gamestate.framebuffer.state.enable_depth  = true;

    Fang_DrawMapSkybox(
        &gamestate.framebuffer,
        &gamestate.camera,
        &gamestate.map,
        Fang_GetTexture(&gamestate.textures, gamestate.map.skybox)
    );

    Fang_DrawMapFloor(
        &gamestate.framebuffer,
        &gamestate.camera,
        &gamestate.map,
        &gamestate.textures
    );

    Fang_DrawMapTiles(
        &gamestate.framebuffer,
        &gamestate.camera,
        &gamestate.textures,
        &gamestate.map,
        gamestate.raycast,
        (size_t)FANG_WINDOW_SIZE
    );

    // Fang_DrawEntitiesUnculled(
    //     &gamestate.framebuffer,
    //     &gamestate.camera,
    //     &gamestate.textures,
    //     &gamestate.map,
    //     &gamestate.entities
    // );

    Fang_DrawEntitiesCulled(
        &gamestate.framebuffer,
        &gamestate.camera,
        &gamestate.textures,
        gamestate.raycast,
        &gamestate.map,
        &gamestate.entities,
        (size_t)FANG_WINDOW_SIZE
    );

    Fang_ShadeFramebuffer(
        &gamestate.framebuffer,
        &gamestate.map.fog,
        gamestate.map.fog_distance
    );

    gamestate.framebuffer.state.enable_depth = false;

    if (player)
    {
        const Fang_Weapon * const weapon = Fang_GetWeapon(
            player->props.player.weapon
        );

        if (weapon)
        {
            const Fang_Image * const weapon_texture = Fang_GetTexture(
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
                    &gamestate.framebuffer,
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
                &gamestate.framebuffer,
                weapon->name,
                Fang_GetTexture(&gamestate.textures, FANG_TEXTURE_FORMULA),
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
                &gamestate.framebuffer,
                ammo_count,
                Fang_GetTexture(&gamestate.textures, FANG_TEXTURE_FORMULA),
                FANG_FONT_HEIGHT,
                &(Fang_Point){.x = 5, .y = 3 + FANG_FONT_HEIGHT}
            );
        }

        {
            char health[4] = "000";
            snprintf(
                health,
                sizeof(health),
                "%3d",
                player->props.player.health
            );

            const int text_offset = 3 - (int)strlen(health);

            Fang_DrawText(
                &gamestate.framebuffer,
                health,
                Fang_GetTexture(&gamestate.textures, FANG_TEXTURE_FORMULA),
                FANG_FONT_HEIGHT,
                &(Fang_Point){
                    .x = viewport.w - 5 - (FANG_FONT_WIDTH * (3 - text_offset)),
                    .y = 3
                }
            );
        }

        {
            char position[15];

            snprintf(
                position,
                sizeof(position),
                "%3.2f, %3.2f",
                fmodf(player->body.pos.x, FANG_CHUNK_SIZE),
                fmodf(player->body.pos.y, FANG_CHUNK_SIZE)
            );

            Fang_DrawText(
                &gamestate.framebuffer,
                position,
                Fang_GetTexture(&gamestate.textures, FANG_TEXTURE_FORMULA),
                FANG_FONT_HEIGHT,
                &(Fang_Point){
                    .x = 3,
                    .y = viewport.h - FANG_FONT_HEIGHT - 3
                }
            );
        }
    }

    {
        const Fang_FrameState state = Fang_SetViewport(
            &gamestate.framebuffer,
            &(Fang_Rect){
                .x = FANG_WINDOW_SIZE - 32,
                .y = FANG_WINDOW_SIZE - 32,
                .w = 32,
                .h = 32,
            }
        );

        Fang_DrawMinimap(
            &gamestate.framebuffer,
            &gamestate.camera,
            &gamestate.map,
            gamestate.raycast,
            (size_t)FANG_WINDOW_SIZE
        );

        gamestate.framebuffer.state = state;
    }

    gamestate.framebuffer.state.current_depth = 0.0f;

    Fang_SetFragment(
        &gamestate.framebuffer,
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

    return &gamestate.framebuffer.color;
}

static inline void
Fang_Quit(void)
{
    Fang_FreeTextures(&gamestate.textures);
    Fang_FreeImage(&gamestate.framebuffer.color);
    Fang_FreeImage(&gamestate.framebuffer.depth);
}
