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

void
Fang_AmmoUpdate(
    Fang_State  * const state,
    Fang_Entity * const ammo)
{
    assert(state);
    assert(ammo);
    assert(ammo->state);
    assert(ammo->type == FANG_ENTITYTYPE_AMMO_PICKUP);

    if (ammo->state == FANG_ENTITYSTATE_CREATING)
        ammo->state = FANG_ENTITYSTATE_ACTIVE;
    else if (ammo->state == FANG_ENTITYSTATE_REMOVING)
        Fang_EntitySetRemove(&state->entities, ammo->id);
}

void
Fang_AmmoCollide(
          Fang_Entity * const ammo,
          Fang_Entity * const entity,
    const bool                initial_collision)
{
    assert(ammo);
    assert(entity);
    assert(ammo->type == FANG_ENTITYTYPE_AMMO_PICKUP);

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

            *inventory = max(100, (*inventory) + ammo_props->count);

            ammo->state = FANG_ENTITYSTATE_REMOVING;

            return;
        }

        default:
            return;
    };
}

void
Fang_HealthUpdate(
    Fang_State  * const state,
    Fang_Entity * const health)
{
    assert(state);
    assert(health);
    assert(health->state);
    assert(health->type == FANG_ENTITYTYPE_HEALTH_PICKUP);

    if (health->state == FANG_ENTITYSTATE_CREATING)
        health->state = FANG_ENTITYSTATE_ACTIVE;
    else if (health->state == FANG_ENTITYSTATE_REMOVING)
        Fang_EntitySetRemove(&state->entities, health->id);
}

void
Fang_HealthCollide(
          Fang_Entity * const health,
          Fang_Entity * const entity,
    const bool                initial_collision)
{
    assert(health);
    assert(entity);
    assert(health->type == FANG_ENTITYTYPE_HEALTH_PICKUP);

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
