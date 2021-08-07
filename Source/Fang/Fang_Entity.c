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

enum {
    FANG_MAX_ENTITIES   = 256,
    FANG_MAX_COLLISIONS = FANG_MAX_ENTITIES * 64,
};

/**
 * The life-cycle state that the entity is in.
 *
 * When entities are inactive, they are considered to not exist in the game
 * world (and subsequently, their spots in the entity table aren't reserved).
 * Entities that are in the process of being created or removed are still
 * available for different systems - such as the rendering system - but they may
 * be excluded from other systems - such as the collision system.
**/
typedef enum Fang_EntityState {
    FANG_ENTITYSTATE_INACTIVE = 0,
    FANG_ENTITYSTATE_ACTIVE   = 1 << 0,
    FANG_ENTITYSTATE_CREATING = 1 << 1,
    FANG_ENTITYSTATE_REMOVING = 1 << 2,
} Fang_EntityState;

/**
 * The type of entity.
 *
 * This represents what type of object or character an entity is. The type
 * influences various behavioral aspects of the entity as well as appearance.
 * Textures, animations, sounds, and AI, are determined by the entity type.
**/
typedef enum Fang_EntityType {
    FANG_ENTITYTYPE_PLAYER,
    FANG_ENTITYTYPE_HEALTH,
    FANG_ENTITYTYPE_AMMO,
    FANG_ENTITYTYPE_PROJECTILE,
} Fang_EntityType;

typedef size_t Fang_EntityId;

/**
 * Properties specific to health pickup entity types.
**/
typedef struct Fang_HealthProps {
    int count;
} Fang_HealthProps;

/**
 * Properties specific to ammo pickup entity types.
**/
typedef struct Fang_AmmoProps {
    Fang_WeaponType type;
    int             count;
} Fang_AmmoProps;

/**
 * Properties specific to player entity types.
**/
typedef struct Fang_PlayerProps {
    Fang_InputId    input;
    Fang_WeaponType weapon;
    int             ammo[FANG_NUM_WEAPONTYPE];
    uint32_t        cooldown;
    int             health;
    Fang_LerpVec2   sway;
    float           bob;
} Fang_PlayerProps;

/**
 * Properties specific to projectile entity types.
**/
typedef struct Fang_ProjectileProps {
    Fang_WeaponType type;
    Fang_EntityId   owner;
    int             damage;
    int             health;
    uint32_t        lifespan;
    int             ricochets;
    float           speed;
} Fang_ProjectileProps;

/**
 * The core "thing" to the game engine.
 *
 * Entities represent physical things in that world that can interact with each
 * other and the environment. This includes both pickups, characters, players,
 * projectiles, etc.
 *
 * Entities use a conjunction of type, state, and flags to determine what
 * appearance and behavior they should have during gameplay.
**/
typedef struct Fang_Entity {
    Fang_EntityId    id;
    Fang_EntityType  type;
    Fang_EntityState state;
    Fang_Body        body;
    union {
        Fang_AmmoProps       ammo;
        Fang_HealthProps     health;
        Fang_PlayerProps     player;
        Fang_ProjectileProps projectile;
    } props;
} Fang_Entity;

/**
 * A structure used for identifying an interaction between two entities.
**/
typedef union Fang_EntityPair {
    struct {
        Fang_EntityId first;
        Fang_EntityId second;
    };
    Fang_EntityId entities[2];
} Fang_EntityPair;

/**
 * A set used to hold collisions between entities for a given frame.
**/
typedef struct Fang_CollisionSet {
    Fang_EntityPair collisions[FANG_MAX_COLLISIONS];
    size_t          count;
} Fang_CollisionSet;

/**
 * A set used to hold entities and information about them.
 *
 * This structure holds the entity array used to manage and query entities. It
 * also maintains the collision tables for both the current and previous frame.
**/
typedef struct Fang_EntitySet {
    Fang_Entity       entities[FANG_MAX_ENTITIES];
    Fang_EntityId     last_index;
    Fang_CollisionSet collisions;
    Fang_CollisionSet last_collisions;
} Fang_EntitySet;

/**
 * Returns the relevant texture for the entity's entity type.
**/
static inline Fang_Texture
Fang_EntityQueryTexture(
    const Fang_Entity * const entity)
{
    assert(entity);

    switch (entity->type)
    {
        case FANG_ENTITYTYPE_PLAYER:     return FANG_TEXTURE_NONE;
        case FANG_ENTITYTYPE_AMMO:       return FANG_TEXTURE_AMMO;
        case FANG_ENTITYTYPE_HEALTH:     return FANG_TEXTURE_HEALTH;
        case FANG_ENTITYTYPE_PROJECTILE: return FANG_TEXTURE_PROJECTILE;

        default:
            return FANG_TEXTURE_NONE;
    }
}

/**
 * Searches the entity array for a given entity.
 *
 * If the entity in the designated index is marked inactive, this will return
 * a null pointer.
**/
static inline Fang_Entity *
Fang_EntitySetQuery(
          Fang_EntitySet * const entities,
    const Fang_EntityId          entity_id)
{
    assert(entities);
    assert(entity_id < FANG_MAX_ENTITIES);

    Fang_Entity * result = &entities->entities[entity_id];

    if (!result->state)
        return NULL;

    return result;
}

/**
 * Adds an entity to the world.
 *
 * This will overwrite the initial `id` and `state` values with the new ID and
 * FANG_ENTITYSTATE_CREATING. It is up to the entity logic to transition the
 * entity from creating to active.
 *
 * This will place the new entity in either the spot of the last removed entity,
 * or the next available index into the entity array. If the number of entities
 * exceeds the maximum this function will do nothing.
**/
static inline Fang_EntityId
Fang_EntitySetAdd(
          Fang_EntitySet * const entities,
    const Fang_Entity            initial)
{
    assert(entities);
    assert(entities->last_index < FANG_MAX_ENTITIES);

    const Fang_EntityId result = entities->last_index;

    {
        Fang_Entity * const entity = &entities->entities[result];

        memcpy(entity, &initial, sizeof(Fang_Entity));

        entity->id    = result;
        entity->state = FANG_ENTITYSTATE_CREATING;
    }

    const bool last_index_valid = result < FANG_MAX_ENTITIES - 1;
    const bool last_index_open  = !Fang_EntitySetQuery(
        entities, entities->last_index + 1
    );

    // Attempt to grab the next array index, otherwise find the first available
    if (last_index_valid && last_index_open)
    {
        entities->last_index++;
    }
    else
    {
        for (Fang_EntityId i = 0; i < FANG_MAX_ENTITIES; ++i)
        {
            if (i == result)
                continue;

            if (!Fang_EntitySetQuery(entities, i))
            {
                entities->last_index = i;
                break;
            }
        }
    }

    // The entity limit has been reached!
    assert(entities->last_index != result);

    return result;
}

/**
 * Removes an entity from the world.
 *
 * This will simply zero-out the entity properties and update the last-index in
 * the entity set. The entity array is not dynamically resized but rather holds
 * a set maximum number of entities.
**/
static inline void
Fang_EntitySetRemove(
          Fang_EntitySet * const entities,
    const Fang_EntityId          entity_id)
{
    assert(entities);
    assert(entity_id < FANG_MAX_ENTITIES);

    memset(&entities->entities[entity_id], 0, sizeof(Fang_Entity));
    entities->last_index = entity_id;
}

/**
 * Adds a pair of entities to the current frame's collision table, if the entity
 * collision has not already been recorded.
**/
static inline void
Fang_CollisionSetAdd(
          Fang_CollisionSet * const collisions,
    const Fang_EntityPair           pair)
{
    assert(collisions);
    assert(pair.first != pair.second);
    assert(collisions->count <= FANG_MAX_COLLISIONS - 1);

    for (size_t i = 0; i < collisions->count; ++i)
    {
        const Fang_EntityPair collision = collisions->collisions[i];

        if (pair.first == collision.first && pair.second == collision.second)
            return;

        if (pair.first == collision.second && pair.second == collision.first)
            return;
    }

    collisions->collisions[collisions->count++] = pair;
}

/**
 * Executes the relevant entity behavior subroutines based on the flags from
 * both entities.
 *
 * If either of the two entities has a flag enabled, the relevant subroutine is
 * run even if the subroutine may require that both entities have the flag
 * enabled.
**/
static inline void
Fang_EntityResolveCollision(
          Fang_Entity * const first,
          Fang_Entity * const second,
    const bool                initial_collision)
{
    assert(first);
    assert(second);
    assert(first->id != second->id);

    typedef void (CollisionFunc)(
        Fang_Entity * const,
        Fang_Entity * const,
        const bool
    );

    CollisionFunc Fang_AmmoCollideEntity;
    CollisionFunc Fang_HealthCollideEntity;
    CollisionFunc Fang_PlayerCollideEntity;

    Fang_BodyResolveBodyCollision(&first->body, &second->body);

    for (size_t i = 0; i < 2; ++i)
    {
        Fang_Entity * const entity = (i == 0) ?  first : second;
        Fang_Entity * const  other = (i == 0) ? second :  first;

        switch (entity->type)
        {
            case FANG_ENTITYTYPE_PLAYER:
                Fang_PlayerCollideEntity(entity, other, initial_collision);
                break;

            case FANG_ENTITYTYPE_AMMO:
                Fang_AmmoCollideEntity(entity, other, initial_collision);
                break;

            case FANG_ENTITYTYPE_HEALTH:
                Fang_HealthCollideEntity(entity, other, initial_collision);
                break;

            default:
                break;
        };
    }
}

/**
 * Calculates and resolves current-frame collisions for entities in the set.
 *
 * The results of the current-frame collision table are compared with those of
 * the previous-frame collision table to determine if the current collision is
 * a "sustained collision" or not. Sustained collisions may occur over multiple
 * frames, but their side effects (such as damage) typically may happen only on
 * the initial impact (such as a projectile hitting an entity and passing
 * through it).
 *
 * If an entity is removed during collision resolution (i.e. their state is set
 * to inactive), further collisions that the entity is involved in will be
 * disregarded.
 *
 * Collisions that are not disregarded for this frame are saved into the
 * previous-frame collision table at the end of this subroutine.
**/
static inline void
Fang_EntitySetResolveCollisions(
          Fang_EntitySet * const entities,
    const Fang_Map       * const map)
{
    assert(entities);
    assert(!entities->collisions.count);
    assert(map);

    typedef void (CollisionFunc)(
        Fang_Entity * const
    );

    CollisionFunc Fang_AmmoCollideMap;
    CollisionFunc Fang_HealthCollideMap;
    CollisionFunc Fang_PlayerCollideMap;
    CollisionFunc Fang_ProjectileCollideMap;

    for (Fang_EntityId i = 0; i < FANG_MAX_ENTITIES; ++i)
    {
        Fang_Entity * const entity = Fang_EntitySetQuery(entities, i);

        if (!entity)
            continue;

        if (Fang_BodyResolveMapCollision(&entity->body, map))
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

    for (Fang_EntityId i = 0; i < FANG_MAX_ENTITIES; ++i)
    {
        for (Fang_EntityId j = i + 1; j < FANG_MAX_ENTITIES; ++j)
        {
            Fang_Entity * const pair[2] = {
                Fang_EntitySetQuery(entities, i),
                Fang_EntitySetQuery(entities, j),
            };

            if (!pair[0] || !pair[1])
                continue;

            if (Fang_BodiesIntersect(&pair[0]->body, &pair[1]->body))
            {
                Fang_CollisionSetAdd(
                    &entities->collisions,
                    (Fang_EntityPair){{i, j}}
                );
            }
        }
    }

    assert(entities->collisions.count < FANG_MAX_COLLISIONS);

    Fang_CollisionSet * const collisions      = &entities->collisions;
    Fang_CollisionSet * const last_collisions = &entities->last_collisions;

    for (size_t i = 0; i < entities->collisions.count; ++i)
    {
        const Fang_EntityPair collision = entities->collisions.collisions[i];

        Fang_Entity * const first = Fang_EntitySetQuery(
            entities, collision.first
        );

        if (!first || first->state != FANG_ENTITYSTATE_ACTIVE)
            continue;

        Fang_Entity * const second = Fang_EntitySetQuery(
            entities, collision.second
        );

        if (!second || second->state != FANG_ENTITYSTATE_ACTIVE)
            continue;

        bool initial_collision = true;
        for (size_t j = 0; j < last_collisions->count; ++j)
        {
            const Fang_EntityPair last_collision = (
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

        Fang_EntityResolveCollision(first, second, initial_collision);
    }

    memset(last_collisions, 0, sizeof(Fang_CollisionSet));
    memcpy(last_collisions, collisions, sizeof(Fang_CollisionSet));
    memset(collisions, 0, sizeof(Fang_CollisionSet));
}
