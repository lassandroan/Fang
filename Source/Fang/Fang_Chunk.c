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
    assert(index->x <= FANG_CHUNK_MAX);
    assert(index->y >= FANG_CHUNK_MIN);
    assert(index->y <= FANG_CHUNK_MAX);

    static uint64_t lookup[1 << 4] = {
        [ 0] = 0b00000000,
        [ 1] = 0b00000001,
        [ 2] = 0b00000100,
        [ 3] = 0b00000101,
        [ 4] = 0b00010000,
        [ 5] = 0b00010001,
        [ 6] = 0b00010100,
        [ 7] = 0b00010101,
        [ 8] = 0b01000000,
        [ 9] = 0b01000001,
        [10] = 0b01000100,
        [11] = 0b01000101,
        [12] = 0b01010000,
        [13] = 0b01010001,
        [14] = 0b01010100,
        [15] = 0b01010101,
    };

    // const uint8_t x = (uint8_t)index->x;
    // const uint8_t y = (uint8_t)index->y;

    const uint8_t x = (index->x < 0)
        ? (uint8_t)(index->x & ~(1 << 7))
        : (uint8_t)index->x;

    const uint8_t y = (index->y < 0)
        ? (uint8_t)(index->y & ~(1 << 7))
        : (uint8_t)index->y;

    uint16_t result = 0;
    for (uint8_t i = 0; i < 16; i += 4)
    {
        const uint8_t xbits = (x >> i) & 0b1111;
        const uint8_t ybits = (y >> i) & 0b1111;
        result |= (lookup[xbits] | (lookup[ybits] << 1)) << (i * 2);
    }

    // assert(result < FANG_CHUNK_COUNT);
    result = clamp(result, 0, FANG_CHUNK_COUNT);

    return &chunks->chunks[result];
}

#define Fang_GetChunk(chunks, position) _Generic(position,  \
    const Fang_Vec2*:  Fang_GetChunkVec2,                   \
          Fang_Vec2*:  Fang_GetChunkVec2,                   \
    const Fang_Vec3*:  Fang_GetChunkVec3,                   \
          Fang_Vec3*:  Fang_GetChunkVec3,                   \
    const Fang_Point*: Fang_GetChunkPoint,                  \
          Fang_Point*: Fang_GetChunkPoint                   \
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
    assert(position->x >= INT8_MIN);
    assert(position->x <= INT8_MAX);
    assert(position->y >= INT8_MIN);
    assert(position->y <= INT8_MAX);

    return Fang_GetIndexedChunk(
        chunks,
        &(Fang_ChunkIndex){
            .x = (int8_t)(position->x / (float)FANG_CHUNK_SIZE),
            .y = (int8_t)(position->y / (float)FANG_CHUNK_SIZE),
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
    assert(position->x >= INT8_MIN);
    assert(position->x <= INT8_MAX);
    assert(position->y >= INT8_MIN);
    assert(position->y <= INT8_MAX);

    return Fang_GetIndexedChunk(
        chunks,
        &(Fang_ChunkIndex){
            .x = (int8_t)(position->x / (float)FANG_CHUNK_SIZE),
            .y = (int8_t)(position->y / (float)FANG_CHUNK_SIZE),
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
    assert(position->x >= INT8_MIN);
    assert(position->x <= INT8_MAX);
    assert(position->y >= INT8_MIN);
    assert(position->y <= INT8_MAX);

    return Fang_GetIndexedChunk(
        chunks,
        &(Fang_ChunkIndex){
            .x = (int8_t)(position->x / FANG_CHUNK_SIZE),
            .y = (int8_t)(position->y / FANG_CHUNK_SIZE),
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

    Fang_Vec2 fractional = {
        .x = fmodf(position->x, (float)FANG_CHUNK_SIZE),
        .y = fmodf(position->y, (float)FANG_CHUNK_SIZE),
    };

    fractional.x = (fractional.x < 0.0f) ? 1.0f - fractional.x : fractional.x;
    fractional.y = (fractional.y < 0.0f) ? 1.0f - fractional.y : fractional.y;

    const Fang_Point tile_index = {
        .x = (int)(fractional.x * FANG_CHUNK_SIZE),
        .y = (int)(fractional.y * FANG_CHUNK_SIZE),
    };

    if (chunk->tiles[tile_index.x][tile_index.y].type)
        return &chunk->tiles[tile_index.x][tile_index.y];

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
