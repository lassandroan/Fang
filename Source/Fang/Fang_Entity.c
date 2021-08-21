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
typedef struct Fang_EntityCollision {
    Fang_EntityId first;
    Fang_EntityId second;
} Fang_EntityCollision;

/**
 * A set used to hold collisions between entities for a given frame.
**/
typedef struct Fang_EntityCollisionSet {
    Fang_EntityCollision collisions[FANG_MAX_COLLISIONS];
    size_t          count;
} Fang_EntityCollisionSet;

/**
 * A set used to hold entities and information about them.
 *
 * This structure holds the entity array used to manage and query entities. It
 * also maintains the collision tables for both the current and previous frame.
**/
typedef struct Fang_EntitySet {
    Fang_Entity              entities[FANG_MAX_ENTITIES];
    Fang_EntityId            last_index;
    Fang_EntityCollisionSet  collisions;
    Fang_EntityCollisionSet  last_collisions;
} Fang_EntitySet;

/**
 * Returns the relevant texture for the entity's entity type.
**/
static inline Fang_TextureId
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
    const Fang_Entity    * const initial)
{
    assert(entities);
    assert(entities->last_index < FANG_MAX_ENTITIES);

    const Fang_EntityId result = entities->last_index;

    {
        Fang_Entity * const entity = &entities->entities[result];

        memcpy(entity, initial, sizeof(Fang_Entity));

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
Fang_EntityCollisionSetAdd(
          Fang_EntityCollisionSet * const collisions,
    const Fang_EntityCollision                 pair)
{
    assert(collisions);
    assert(pair.first != pair.second);
    assert(collisions->count <= FANG_MAX_COLLISIONS - 1);

    for (size_t i = 0; i < collisions->count; ++i)
    {
        const Fang_EntityCollision collision = collisions->collisions[i];

        if (pair.first == collision.first && pair.second == collision.second)
            return;

        if (pair.first == collision.second && pair.second == collision.first)
            return;
    }

    collisions->collisions[collisions->count++] = pair;
}

