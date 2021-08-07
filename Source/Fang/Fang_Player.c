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


static inline Fang_EntityId
Fang_PlayerCreate(
          Fang_EntitySet  * const entities,
    const Fang_InputId            input,
    const Fang_Vec3               pos,
    const Fang_Vec3               dir)
{
    assert(entities);

    return Fang_EntitySetAdd(
        entities,
        (Fang_Entity){
            .type = FANG_ENTITYTYPE_PLAYER,
            .props.player.input  = input,
            .props.player.health = 100,
            .props.player.weapon = FANG_WEAPONTYPE_NONE,
            .body = (Fang_Body){
                .pos       = pos,
                .dir       = dir,
                .vel.delta = {
                    .x = FANG_RUN_SPEED / 100.0f,
                    .y = FANG_RUN_SPEED / 100.0f,
                    .z = FANG_GRAVITY,
                },
                .width     = FANG_PLAYER_WIDTH,
                .height    = FANG_PLAYER_HEIGHT,
                .flags     = (
                    FANG_BODYFLAG_JUMP
                  | FANG_BODYFLAG_FALL
                  | FANG_BODYFLAG_STEP
                  | FANG_BODYFLAG_COLLIDE_WALLS
                  | FANG_BODYFLAG_COLLIDE_BODIES
                ),
            }
        }
    );
}

void
Fang_PlayerUpdate(
    Fang_State  * const state,
    Fang_Entity * const player)
{
    assert(state);
    assert(player);

    if (player->state == FANG_ENTITYSTATE_CREATING)
    {
        player->state = FANG_ENTITYSTATE_ACTIVE;
    }
    else if (player->state == FANG_ENTITYSTATE_REMOVING)
    {
        Fang_EntitySetRemove(&state->entities, player->id);
        return;
    }
}

void
Fang_PlayerCollideMap(
    Fang_Entity * const player)
{
    assert(player);
}

void
Fang_PlayerCollideEntity(
          Fang_Entity * const player,
          Fang_Entity * const entity,
    const bool                initial_collision)
{
    assert(player);
    assert(entity);
    assert(player->type == FANG_ENTITYTYPE_PLAYER);

    (void)initial_collision;
}

void
Fang_PlayerFireWeapon(
    Fang_Entity    * const player,
    Fang_EntitySet * const entities)
{
    assert(player);
    assert(player->state == FANG_ENTITYSTATE_ACTIVE);
    assert(entities);

    Fang_WeaponType * const weapon_type = &player->props.player.weapon;

    if (*weapon_type == FANG_WEAPONTYPE_NONE)
        return;

    int * const inventory = &player->props.player.ammo[*weapon_type];

    if (!*inventory)
        return;

    (*inventory)--;

    Fang_ProjectileCreate(entities, *weapon_type, player->id);
}
