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
Fang_CreateAmmo(
          Fang_Entities   * const entities,
    const Fang_WeaponType         type,
    const int                     count,
    const Fang_Vec3               pos)
{
    assert(entities);

    return Fang_AddEntity(
        entities,
        &(Fang_Entity){
            .type = FANG_ENTITYTYPE_AMMO,
            .props.ammo.type  = type,
            .props.ammo.count = count,
            .body = (Fang_Body){
                .pos    = pos,
                .width  = FANG_PICKUP_WIDTH,
                .height = FANG_PICKUP_HEIGHT,
                .flags  = (
                    FANG_BODYFLAG_FALL
                  | FANG_BODYFLAG_COLLIDE_WALLS
                ),
            },
        }
    );
}

static inline void
Fang_UpdateAmmo(
          Fang_State  * const state,
          Fang_Entity * const ammo,
    const uint32_t            delta)
{
    assert(state);
    assert(ammo);
    assert(ammo->state);
    assert(ammo->type == FANG_ENTITYTYPE_AMMO);

    (void)delta;

    if (ammo->state == FANG_ENTITYSTATE_REMOVING)
    {
        Fang_RemoveEntity(&state->entities, ammo->id);
        return;
    }

    if (ammo->state == FANG_ENTITYSTATE_CREATING)
        ammo->state = FANG_ENTITYSTATE_ACTIVE;
}

static inline void
Fang_AmmoCollideMap(
    Fang_Entity * const ammo)
{
    assert(ammo);
}

static inline void
Fang_AmmoCollideEntity(
          Fang_Entity * const ammo,
          Fang_Entity * const entity,
    const bool                initial_collision)
{
    assert(ammo);
    assert(entity);
    assert(ammo->type == FANG_ENTITYTYPE_AMMO);

    if (!initial_collision)
        return;

    Fang_AmmoProps * const ammo_props = &ammo->props.ammo;

    switch (entity->type)
    {
        case FANG_ENTITYTYPE_PLAYER:
        {
            Fang_PlayerProps * const player_props = &entity->props.player;

            int * const inventory = &player_props->ammo[ammo_props->type];

            if (*inventory >= 100)
                break;

            *inventory = min(100, (*inventory) + ammo_props->count);

            ammo->state = FANG_ENTITYSTATE_REMOVING;

            return;
        }

        default:
            return;
    };
}

static inline Fang_EntityId
Fang_CreateHealth(
          Fang_Entities * const entities,
    const int                   count,
    const Fang_Vec3             pos)
{
    assert(entities);

    return Fang_AddEntity(
        entities,
        &(Fang_Entity){
            .type = FANG_ENTITYTYPE_HEALTH,
            .props.health.count = count,
            .body = {
                .pos    = pos,
                .width  = FANG_PICKUP_WIDTH,
                .height = FANG_PICKUP_HEIGHT,
                .flags  = (
                    FANG_BODYFLAG_FALL
                  | FANG_BODYFLAG_COLLIDE_WALLS
                ),
            }
        }
    );
}

static inline void
Fang_UpdateHealth(
          Fang_State  * const state,
          Fang_Entity * const health,
    const uint32_t            delta)
{
    assert(state);
    assert(health);
    assert(health->state);
    assert(health->type == FANG_ENTITYTYPE_HEALTH);

    (void)delta;

    if (health->state == FANG_ENTITYSTATE_REMOVING)
    {
        Fang_RemoveEntity(&state->entities, health->id);
        return;
    }

    if (health->state == FANG_ENTITYSTATE_CREATING)
        health->state = FANG_ENTITYSTATE_ACTIVE;
}

static inline void
Fang_HealthCollideMap(
    Fang_Entity * const health)
{
    assert(health);
}

static inline void
Fang_HealthCollideEntity(
          Fang_Entity * const health,
          Fang_Entity * const entity,
    const bool                initial_collision)
{
    assert(health);
    assert(entity);
    assert(health->type == FANG_ENTITYTYPE_HEALTH);

    if (!initial_collision)
        return;

    Fang_HealthProps * const health_props = &health->props.health;

    switch (entity->type)
    {
        case FANG_ENTITYTYPE_PLAYER:
        {
            Fang_PlayerProps * const player_props = &entity->props.player;

            player_props->health += health_props->count;

            health->state = FANG_ENTITYSTATE_REMOVING;
            return;
        }

        default:
            return;
    }
}
