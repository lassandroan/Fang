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

typedef enum Fang_EntityFlags {
    FANG_ENTITYFLAG_NONE     = 0,
    FANG_ENTITYFLAG_ACTIVE   = 1 << 0,
    FANG_ENTITYFLAG_COLLIDES = 1 << 1,
} Fang_EntityFlags;

typedef size_t Fang_EntityId;

typedef struct Fang_Entity {
    Fang_EntityId id;
    int           flags;
    Fang_Body     body;
    Fang_Texture  texture;
} Fang_Entity;

typedef union Fang_EntityPair {
    struct {
        Fang_EntityId first;
        Fang_EntityId second;
    };
    Fang_EntityId entities[2];
} Fang_EntityPair;

typedef struct Fang_CollisionSet {
    Fang_EntityPair collisions[FANG_MAX_COLLISIONS];
    size_t          count;
} Fang_CollisionSet;

typedef struct Fang_EntitySet {
    Fang_Entity       entities[FANG_MAX_ENTITIES];
    Fang_EntityId     last_index;
    Fang_CollisionSet collisions;
    Fang_CollisionSet last_collisions;
} Fang_EntitySet;

static inline Fang_EntityId
Fang_EntitySetAdd(
          Fang_EntitySet * const entities,
    const Fang_Entity            entity)
{
    assert(entities);
    assert(entities->last_index < FANG_MAX_ENTITIES);

    const Fang_EntityId result = entities->last_index;

    entities->entities[result] = entity;
    entities->entities[result].id = result;
    entities->entities[result].flags |= FANG_ENTITYFLAG_ACTIVE;

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

    if (entities->last_index == result)
    {
        assert(0);
        entities->last_index = FANG_MAX_ENTITIES;
    }

    return result;
}

static inline Fang_Entity *
Fang_EntitySetQuery(
          Fang_EntitySet * const entities,
    const Fang_EntityId          entity_id)
{
    assert(entities);
    assert(entity_id < FANG_MAX_ENTITIES);

    Fang_Entity * result = &entities->entities[entity_id];

    if (!result->flags)
        return NULL;

    return result;
}

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

static inline void
Fang_EntityResolveCollision(
          Fang_Entity * const first,
          Fang_Entity * const second,
    const bool                initial_collision)
{
    assert(first);
    assert(second);
    assert(first->id != second->id);

    (void)initial_collision;

    if (!(first->flags & FANG_ENTITYFLAG_COLLIDES))
        return;

    if (!(second->flags & FANG_ENTITYFLAG_COLLIDES))
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

        if (!first)
            continue;

        Fang_Entity * const second = Fang_EntitySetQuery(
            entities, collision.second
        );

        if (!second)
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
