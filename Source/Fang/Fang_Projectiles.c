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
Fang_ProjectileCreate(
          Fang_EntitySet       * const entities,
    const Fang_ProjectileProps         props)
{
    assert(entities);

    Fang_Entity * const owner = Fang_EntitySetQuery(entities, props.owner);

    assert(owner);

    return Fang_EntitySetAdd(
        entities,
        (Fang_Entity){
            .type = FANG_ENTITYTYPE_PROJECTILE,
            .props.projectile = props,
            .body = {
                .pos = {
                    .x = owner->body.pos.x,
                    .y = owner->body.pos.y,
                    .z = owner->body.pos.z + owner->body.size / 2,
                },
                .acc = {
                    .x = FANG_RUN_SPEED * 100,
                    .y = FANG_RUN_SPEED * 100,
                },
                .max = {
                    .x = FANG_RUN_SPEED * 4,
                    .y = FANG_RUN_SPEED * 4,
                },
                .dir = owner->body.dir,
                .mov = owner->body.dir,
                .size = owner->body.size / 4,
                .flags = FANG_BODYFLAG_COLLIDE_WALLS,
            },
        }
    );
}

static inline void
Fang_ProjectileUpdate(
    Fang_State  * const state,
    Fang_Entity * const projectile)
{
    assert(state);
    assert(projectile);
    assert(projectile->state);
    assert(projectile->type == FANG_ENTITYTYPE_PROJECTILE);

    Fang_ProjectileProps * const projectile_props = (
        &projectile->props.projectile
    );

    if (projectile_props->health <= 0)
        projectile->state = FANG_ENTITYSTATE_REMOVING;

    if (projectile->state == FANG_ENTITYSTATE_CREATING)
    {
        projectile->state = FANG_ENTITYSTATE_ACTIVE;
    }
    else if (projectile->state == FANG_ENTITYSTATE_REMOVING)
    {
        Fang_EntitySetRemove(&state->entities, projectile->id);
        return;
    }
}

void
Fang_ProjectileCollide(
          Fang_Entity * const projectile,
          Fang_Entity * const entity,
    const bool                initial_collision)
{
    assert(projectile);
    assert(entity);
    assert(projectile->type == FANG_ENTITYTYPE_PROJECTILE);

    if (!initial_collision)
        return;

    Fang_ProjectileProps * const projectile_props = (
        &projectile->props.projectile
    );

    if (!projectile_props->richochets && entity->id == projectile_props->owner)
        return;

    switch (entity->type)
    {
        case FANG_ENTITYTYPE_PLAYER:
        {
            Fang_PlayerProps * const player_props = &entity->props.player;
            player_props->health -= projectile_props->damage;
            return;
        }

        case FANG_ENTITYTYPE_PROJECTILE:
        {
            Fang_ProjectileProps * other_props = &entity->props.projectile;
            other_props->health -= projectile_props->damage;
            return;
        }

        default:
            return;
    }
}