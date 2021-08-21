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

#define Fang_ChunkSetQuery(set, value) _Generic(value,         \
        const Fang_ChunkLocation*: Fang_ChunkSetQueryLocation, \
                       Fang_Vec3*: Fang_ChunkSetQueryv3,       \
        const          Fang_Vec3*: Fang_ChunkSetQueryv3        \
    )(set, value)

typedef size_t Fang_EntityId;

/**
 *
**/
typedef struct Fang_ChunkEntitySet Fang_ChunkEntitySet;
typedef struct Fang_ChunkEntitySet
{
    Fang_EntityId entities[FANG_CHUNK_ENTITY_CAPACITY];
    size_t count;
} Fang_ChunkEntitySet;

/**
 *
**/
typedef struct Fang_Chunk {
    Fang_Tile           tiles[FANG_CHUNK_SIZE][FANG_CHUNK_SIZE];
    Fang_ChunkEntitySet entities;
    Fang_TextureId      floor;
} Fang_Chunk;

/**
 *
**/
typedef struct Fang_ChunkSet {
    Fang_Chunk chunks[FANG_CHUNK_COUNT];
} Fang_ChunkSet;

/**
 *
**/
typedef struct Fang_ChunkIndex {
    int8_t x, y;
} Fang_ChunkIndex;


static const Fang_Chunk *
Fang_GetIndexedChunk(
    const Fang_ChunkSet   * const chunks,
    const Fang_ChunkIndex * const index)
{
    assert(chunks);
    assert(index);

    assert(index->x >= FANG_CHUNK_MIN);
    assert(index->x <= FANG_CHUNK_MAX - 1);
    assert(index->y >= FANG_CHUNK_MIN);
    assert(index->y <= FANG_CHUNK_MAX - 1);

    static uint64_t lookup[1 << 3] = {
        [ 0] = 0b000000,
        [ 1] = 0b000001,
        [ 2] = 0b000100,
        [ 3] = 0b000101,
        [ 4] = 0b010000,
        [ 5] = 0b010001,
        [ 6] = 0b010100,
        [ 7] = 0b010101,
    };


    struct {
        unsigned x:      6;
        unsigned y:      6;
        unsigned result: 12;
    } values;

    values.x = (uint8_t)index->x;
    values.y = (uint8_t)index->y;
    values.result = 0;

    {
        const uint8_t xbits = values.x & 0b111;
        const uint8_t ybits = values.y & 0b111;
        values.result |= (lookup[xbits] | (lookup[ybits] << 1));
    }

    {
        const uint8_t xbits = (values.x >> 3) & 0b111;
        const uint8_t ybits = (values.y >> 3) & 0b111;
        values.result |= (lookup[xbits] | (lookup[ybits] << 1)) << 3;
    }

    assert(values.result < FANG_CHUNK_COUNT);

    return &chunks->chunks[values.result];
}

#define Fang_GetChunk(chunks, position) _Generic(position,       \
    const Fang_Vec2*:  Fang_GetChunkVec2,                        \
          Fang_Vec2*:  Fang_GetChunkVec2,                        \
    const Fang_Vec3*:  Fang_GetChunkVec3,                        \
          Fang_Vec3*:  Fang_GetChunkVec3,                        \
    const Fang_Point*: Fang_GetChunkPoint,                       \
          Fang_Point*: Fang_GetChunkPoint                        \
    )(chunks, position)

#define Fang_GetChunkTile(chunks, position) _Generic(position,  \
    const Fang_Vec2*:  Fang_GetChunkTileVec2,                   \
          Fang_Vec2*:  Fang_GetChunkTileVec2,                   \
    const Fang_Vec3*:  Fang_GetChunkTileVec3,                   \
          Fang_Vec3*:  Fang_GetChunkTileVec3,                   \
    const Fang_Point*: Fang_GetChunkTilePoint,                  \
          Fang_Point*: Fang_GetChunkTilePoint                   \
    )(chunks, position)

static inline const Fang_Chunk *
Fang_GetChunkVec2(
    const Fang_ChunkSet * const chunks,
    const Fang_Vec2     * const position)
{
    assert(chunks);
    assert(position);
    assert(position->x >= FANG_CHUNK_SIZE * FANG_CHUNK_MIN);
    assert(position->x <= FANG_CHUNK_SIZE * FANG_CHUNK_MAX);
    assert(position->y >= FANG_CHUNK_SIZE * FANG_CHUNK_MIN);
    assert(position->y <= FANG_CHUNK_SIZE * FANG_CHUNK_MAX);

    const int8_t x_offset = (position->x < 0.0f) ? -1 : 0;
    const int8_t y_offset = (position->y < 0.0f) ? -1 : 0;

    return Fang_GetIndexedChunk(
        chunks,
        &(Fang_ChunkIndex){
            .x = (int8_t)(position->x / (float)FANG_CHUNK_SIZE) + x_offset,
            .y = (int8_t)(position->y / (float)FANG_CHUNK_SIZE) + y_offset,
        }
    );
}

static inline const Fang_Chunk *
Fang_GetChunkVec3(
    const Fang_ChunkSet * const chunks,
    const Fang_Vec3     * const position)
{
    assert(chunks);
    assert(position);
    assert(position->x >= FANG_CHUNK_SIZE * FANG_CHUNK_MIN);
    assert(position->x <= FANG_CHUNK_SIZE * FANG_CHUNK_MAX);
    assert(position->y >= FANG_CHUNK_SIZE * FANG_CHUNK_MIN);
    assert(position->y <= FANG_CHUNK_SIZE * FANG_CHUNK_MAX);

    const int8_t x_offset = (position->x < 0.0f) ? -1 : 0;
    const int8_t y_offset = (position->y < 0.0f) ? -1 : 0;

    return Fang_GetIndexedChunk(
        chunks,
        &(Fang_ChunkIndex){
            .x = (int8_t)(position->x / (float)FANG_CHUNK_SIZE) + x_offset,
            .y = (int8_t)(position->y / (float)FANG_CHUNK_SIZE) + y_offset,
        }
    );
}

static inline const Fang_Chunk *
Fang_GetChunkPoint(
    const Fang_ChunkSet * const chunks,
    const Fang_Point    * const position)
{
    assert(chunks);
    assert(position);
    assert(position->x >= FANG_CHUNK_SIZE * FANG_CHUNK_MIN);
    assert(position->x <= FANG_CHUNK_SIZE * FANG_CHUNK_MAX);
    assert(position->y >= FANG_CHUNK_SIZE * FANG_CHUNK_MIN);
    assert(position->y <= FANG_CHUNK_SIZE * FANG_CHUNK_MAX);

    const int8_t x_offset = (position->x < 0) ? -1 : 0;
    const int8_t y_offset = (position->y < 0) ? -1 : 0;

    return Fang_GetIndexedChunk(
        chunks,
        &(Fang_ChunkIndex){
            .x = (int8_t)(position->x / FANG_CHUNK_SIZE) + x_offset,
            .y = (int8_t)(position->y / FANG_CHUNK_SIZE) + y_offset,
        }
    );
}

static inline const Fang_Tile *
Fang_GetChunkTileVec2(
    const Fang_ChunkSet * const chunks,
    const Fang_Vec2     * const position)
{
    assert(chunks);
    assert(position);

    const Fang_Chunk * const chunk = Fang_GetChunk(chunks, position);

    assert(chunk);

    Fang_Point tile_index = {
        .x = (int)fmodf(position->x, FANG_CHUNK_SIZE),
        .y = (int)fmodf(position->y, FANG_CHUNK_SIZE),
    };

    if (tile_index.x < 0)
        tile_index.x += FANG_CHUNK_SIZE - 1;

    if (tile_index.y < 0)
        tile_index.y += FANG_CHUNK_SIZE - 1;

    const Fang_Tile * const result = &chunk->tiles[tile_index.x][tile_index.y];

    if (result->type)
        return result;

    return NULL;
}

static inline const Fang_Tile *
Fang_GetChunkTileVec3(
    const Fang_ChunkSet * const chunks,
    const Fang_Vec3     * const position)
{
    assert(chunks);
    assert(position);

    return Fang_GetChunkTileVec2(
        chunks, &(Fang_Vec2){.x = position->x, .y = position->y}
    );
}

static inline const Fang_Tile *
Fang_GetChunkTilePoint(
    const Fang_ChunkSet * const chunks,
    const Fang_Point    * const position)
{
    assert(chunks);
    assert(position);

    return Fang_GetChunkTileVec2(
        chunks, &(Fang_Vec2){.x = (float)position->x, .y = (float)position->y}
    );
}
