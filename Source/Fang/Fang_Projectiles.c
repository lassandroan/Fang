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
          Fang_EntitySet  * const entities,
    const Fang_WeaponType         type,
    const Fang_EntityId           owner_id)
{
    assert(entities);

    const Fang_Entity * const owner  = Fang_EntitySetQuery(entities, owner_id);
    const Fang_Weapon * const weapon = Fang_WeaponQuery(type);

    assert(weapon);
    assert(owner);
    assert(owner->state);

    return Fang_EntitySetAdd(
        entities,
        (Fang_Entity){
            .type = FANG_ENTITYTYPE_PROJECTILE,
            .props.projectile = (Fang_ProjectileProps){
                .type     = type,
                .owner    = owner->id,
                .health   = weapon->damage,
                .damage   = weapon->damage,
                .speed    = weapon->speed,
                .lifespan = weapon->lifespan,
            },
            .body = {
                .pos = {
                    .x = owner->body.pos.x,
                    .y = owner->body.pos.y,
                    .z = owner->body.pos.z + owner->body.height / 2,
                },
                .vel.delta = {
                    .x = 1.0f,
                    .y = 1.0f,
                },
                .dir    = owner->body.dir,
                .width  = owner->body.width / 4,
                .height = owner->body.width / 4,
                .flags  = FANG_BODYFLAG_COLLIDE_WALLS,
            },
        }
    );
}

static inline void
Fang_ProjectileUpdate(
          Fang_State  * const state,
          Fang_Entity * const projectile,
    const uint32_t            delta)
{
    assert(state);
    assert(projectile);
    assert(projectile->state);
    assert(projectile->type == FANG_ENTITYTYPE_PROJECTILE);

    Fang_ProjectileProps * const props = &projectile->props.projectile;

    if (props->health <= 0 || props->lifespan == 0)
        projectile->state = FANG_ENTITYSTATE_REMOVING;

    if (projectile->state == FANG_ENTITYSTATE_REMOVING)
    {
        Fang_EntitySetRemove(&state->entities, projectile->id);
        return;
    }

    if (projectile->state == FANG_ENTITYSTATE_CREATING)
        projectile->state = FANG_ENTITYSTATE_ACTIVE;

    if (delta >= props->lifespan)
        props->lifespan = 0;
    else
        props->lifespan -= delta;

    projectile->body.vel.target = Fang_Vec3Translate(
        projectile->body.dir, props->speed, 0.0f, 0.0f
    );
}

void
Fang_ProjectileCollideMap(
    Fang_Entity * const projectile)
{
    assert(projectile);

    projectile->state = FANG_ENTITYSTATE_REMOVING;
}

void
Fang_ProjectileCollideEntity(
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

    if (entity->id == projectile_props->owner)
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
