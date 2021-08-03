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
    FANG_MAX_ENTITIES   = 4,
    FANG_MAX_COLLISIONS = FANG_MAX_ENTITIES * 64,
};

/**
 * Flags that designate how an entity can interact with the world.
 *
 * These flags (which may be combined) inform the entity system what
 * interactions are allowed to happen on an entity, such as colliding with other
 * entities, dealing damage to other entities, being picked up by other
 * entities, etc.
**/
typedef enum Fang_EntityFlags {
    FANG_ENTITYFLAG_NONE    = 0,
    FANG_ENTITYFLAG_CONTROL = 1 << 0,
    FANG_ENTITYFLAG_COLLIDE = 1 << 1,
    FANG_ENTITYFLAG_PICKUP  = 1 << 2,
    FANG_ENTITYFLAG_DAMAGE  = 1 << 3,
} Fang_EntityFlags;

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
    FANG_ENTITYTYPE_PLAYER = 0,
} Fang_EntityType;

typedef size_t Fang_EntityId;

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
    Fang_InputId     input;
    Fang_EntityState state;
    int              flags;
    Fang_Body        body;
    Fang_Texture     texture;
    int              health;
    int              damage;
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

    // Attempt to grab the next array index, otherwise find the first available
    if (result < FANG_MAX_ENTITIES - 1 && !entities->entities[result + 1].flags)
    {
        entities->last_index++;
    }
    else
    {
        for (Fang_EntityId i = 0; i < FANG_MAX_ENTITIES; ++i)
        {
            if (i == result)
                continue;

            if (!entities->entities[i].flags)
            {
                entities->last_index = i;
                break;
            }
        }
    }

    assert(entities->last_index != result);

    if (entities->last_index == result)
        entities->last_index = FANG_MAX_ENTITIES;

    return result;
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
 * Executes a "pick up" of one entity from another.
 *
 * When one entity picks up another, the entity that was picked up transitions
 * to a "removing" state. This allows entities to display animations or play
 * sounds when picked up before being removed from the world (if they are to be
 * removed).
 *
 * Only initial collisions are considered in this subroutine, as items should
 * not be considered to be "picked up" per each frame that they are colliding.
 *
 * If both entities are enabled for pick-up, this function does nothing.
**/
static inline void
Fang_EntityPickup(
          Fang_Entity * const first,
          Fang_Entity * const second,
    const bool                initial_collision)
{
    assert(first);
    assert(second);
    assert(first->id != second->id);

    if (!initial_collision)
        return;

    const bool first_pickup  = first->flags  & FANG_ENTITYFLAG_PICKUP;
    const bool second_pickup = second->flags & FANG_ENTITYFLAG_PICKUP;

    assert(first_pickup || second_pickup);

    // Don't allow pickups to pick up each other
    if (first_pickup && second_pickup)
        return;

    if (first_pickup)
        first->state = FANG_ENTITYSTATE_REMOVING;

    if (second_pickup)
        second->flags = FANG_ENTITYSTATE_REMOVING;
}

/**
 * Calculates damage when one entity collides with another.
 *
 * This is used for things such as projectiles which should lower the health of
 * entities they come into contact with.
**/
static inline void
Fang_EntityDamage(
          Fang_Entity * const first,
          Fang_Entity * const second,
    const bool                initial_collision)
{
    assert(first);
    assert(second);
    assert(first->id != second->id);

    (void)initial_collision;

    if (first->flags & FANG_ENTITYFLAG_DAMAGE)
        second->health -= first->damage;

    if (second->flags & FANG_ENTITYFLAG_DAMAGE)
        first->health -= second->damage;
}

/**
 * Calculates the new position for entities that have collided.
 *
 * When entities have the collideable flag enabled, they should not be able to
 * pass through one another. Collisions of this type stop both entities from
 * moving, and cancel the jump state of whichever entities may have been
 * jumping.
 *
 * This function also resolves discrepancies in Z positions, allowing falling
 * entities to land on top of other collideable entities.
**/
static inline void
Fang_EntityCollide(
          Fang_Entity * const first,
          Fang_Entity * const second,
    const bool                initial_collision)
{
    assert(first);
    assert(second);
    assert(first->id != second->id);

    (void)initial_collision;

    const bool first_collide  = first->flags  & FANG_ENTITYFLAG_COLLIDE;
    const bool second_collide = second->flags & FANG_ENTITYFLAG_COLLIDE;

    if (!(first_collide && second_collide))
        return;

    first->body.jump  = false;
    second->body.jump = false;

    if (first->body.pos.z > second->body.pos.z && first->body.vel.z < 0.0f)
    {
        first->body.vel.z = 0.0f;
        first->body.pos.z = second->body.pos.z + second->body.size;

        return;
    }

    if (second->body.pos.z > first->body.pos.z && second->body.vel.z < 0.0f)
    {
        second->body.vel.z = 0.0f;
        second->body.pos.z = first->body.pos.z + first->body.size;

        return;
    }

    first->body.pos.x   = first->body.last.x;
    first->body.pos.y   = first->body.last.y;
    second->body.pos.x  = second->body.last.x;
    second->body.pos.y  = second->body.last.y;
    first->body.vel.x   = 0.0f;
    first->body.vel.y   = 0.0f;
    second->body.vel.x  = 0.0f;
    second->body.vel.y  = 0.0f;
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

    const int flags = first->flags | second->flags;

    if (flags & FANG_ENTITYFLAG_COLLIDE)
        Fang_EntityCollide(first, second, initial_collision);

    if (flags & FANG_ENTITYFLAG_PICKUP)
        Fang_EntityPickup(first, second, initial_collision);

    if (flags & FANG_ENTITYFLAG_DAMAGE)
        Fang_EntityDamage(first, second, initial_collision);
}

/**
 * Resolves current-frame collisions for entities in the set.
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
    Fang_EntitySet * const entities)
{
    assert(entities);
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
