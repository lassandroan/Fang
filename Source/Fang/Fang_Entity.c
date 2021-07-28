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
    FANG_MAX_ENTITIES = 4,
};

typedef enum Fang_EntityFlags {
    FANG_ENTITYFLAG_NONE   = 0,
    FANG_ENTITYFLAG_ACTIVE = 1 << 0,
} Fang_EntityFlags;

typedef size_t Fang_EntityId;

typedef struct Fang_Entity {
    Fang_EntityId id;
    int           flags;
    Fang_Body     body;
    Fang_Texture  texture;
} Fang_Entity;

typedef struct Fang_EntitySet {
    Fang_Entity   entities[FANG_MAX_ENTITIES];
    Fang_EntityId last;
} Fang_EntitySet;

static inline Fang_EntityId
Fang_EntitySetAdd(
    Fang_EntitySet * const entities,
    Fang_Entity            entity)
{
    assert(entities);
    assert(entities->last < FANG_MAX_ENTITIES);

    const Fang_EntityId result = entities->last;

    entity.id     = result;
    entity.flags |= FANG_ENTITYFLAG_ACTIVE;
    entities->entities[result] = entity;

    // Attempt to grab the next array index, otherwise find the first available
    if (result < FANG_MAX_ENTITIES - 1 && !entities->entities[result + 1].flags)
    {
        entities->last++;
    }
    else
    {
        for (Fang_EntityId i = 0; i < FANG_MAX_ENTITIES; ++i)
        {
            if (i == result)
                continue;

            if (!entities->entities[i].flags)
            {
                entities->last = i;
                break;
            }
        }
    }

    if (entities->last == result)
        entities->last = FANG_MAX_ENTITIES;

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
    entities->last = entity_id;
}
