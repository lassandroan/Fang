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

typedef size_t Fang_EntityId;

/**
 * A structure representing which entities are in a given chunk during a frame.
**/
typedef struct Fang_ChunkEntities
{
    Fang_EntityId entities[FANG_CHUNK_ENTITY_CAPACITY];
    size_t        count;
} Fang_ChunkEntities;

/**
 * A structure representing a logical section of the world.
 *
 * Each chunk contains 16^2 tiles, the core geometry of the game world.
 *
 * Chunks also contain storage for entity-ids to indicate which entities exist
 * in a particular chunk for a given frame.
 *
 * Each chunk can also include which texture should be used for the floor at
 * that position, allowing for varying floor textures across the game map.
**/
typedef struct Fang_Chunk {
    Fang_Tile          tiles[FANG_CHUNK_SIZE][FANG_CHUNK_SIZE];
    Fang_ChunkEntities entities;
    Fang_TextureId     floor;
} Fang_Chunk;

/**
 * A structure that holds all the available chunks of the game world.
**/
typedef struct Fang_Chunks {
    Fang_Chunk chunks[FANG_CHUNK_COUNT];
} Fang_Chunks;

/**
 * Returns the given chunk based on X and Y index values.
 *
 * The amount of chunks per world is dictated by FANG_CHUNK_COUNT, with
 * FANG_CHUNK_MIN and FANG_CHUNK_MAX values representing the allowed index
 * values for finding the chunk.
 *
 * This function calculates the array index using a Z-order curve, interleaving
 * the 'n/2' bits from each number to construct an 'n' bit number capable of
 * addressing the array of size 'FANG_CHUNK_COUNT'.
 *
 * Indices should not exceed FANG_CHUNK_MIN or FANG_CHUNK_MAX - 1.
 *
 * The function never returns NULL.
**/
static const Fang_Chunk *
Fang_GetIndexedChunk(
    const Fang_Chunks * const chunks,
    const int8_t              x_index,
    const int8_t              y_index)
{
    assert(chunks);

    assert(x_index >= FANG_CHUNK_MIN);
    assert(x_index <= FANG_CHUNK_MAX);
    assert(y_index >= FANG_CHUNK_MIN);
    assert(y_index <= FANG_CHUNK_MAX);

    static uint64_t lookup[1 << 3] = {
        [0] = 0b000000,
        [1] = 0b000001,
        [2] = 0b000100,
        [3] = 0b000101,
        [4] = 0b010000,
        [5] = 0b010001,
        [6] = 0b010100,
        [7] = 0b010101,
    };

    struct {
        unsigned x:      6;
        unsigned y:      6;
        unsigned result: 12;
    } values;

    values.x = (uint8_t)x_index;
    values.y = (uint8_t)y_index;
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

/**
 * Returns a chunk based on a given position value.
 *
 * The position is divided by the chunk size (i.e. number of tiles per chunk) to
 * acquire chunk index values. These values are then used to call
 * Fang_GetIndexecChunk().
 *
 * Positions should not exceed FANG_CHUNK_MIN * FANG_CHUNK_SIZE or
 * FANG_CHUNK_MAX * FANG_CHUNK_SIZE - 1.
**/
#define Fang_GetChunk(chunks, position) _Generic(position, \
    const Fang_Vec2*:  Fang_GetChunkVec2,                  \
          Fang_Vec2*:  Fang_GetChunkVec2,                  \
    const Fang_Vec3*:  Fang_GetChunkVec3,                  \
          Fang_Vec3*:  Fang_GetChunkVec3,                  \
    const Fang_Point*: Fang_GetChunkPoint,                 \
          Fang_Point*: Fang_GetChunkPoint                  \
    )(chunks, position)

/**
 * Returns a chunk based on a Fang_Vec2 position.
**/
static inline const Fang_Chunk *
Fang_GetChunkVec2(
    const Fang_Chunks * const chunks,
    const Fang_Vec2   * const position)
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
        (int8_t)(position->x / (float)FANG_CHUNK_SIZE) + x_offset,
        (int8_t)(position->y / (float)FANG_CHUNK_SIZE) + y_offset
    );
}

/**
 * Returns a chunk based on a Fang_Vec3 position.
 *
 * Because chunk division only happens in two dimensions, the Z coordinate of
 * the position is unused here.
**/
static inline const Fang_Chunk *
Fang_GetChunkVec3(
    const Fang_Chunks * const chunks,
    const Fang_Vec3   * const position)
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
        (int8_t)(position->x / (float)FANG_CHUNK_SIZE) + x_offset,
        (int8_t)(position->y / (float)FANG_CHUNK_SIZE) + y_offset
    );
}

/**
 * Returns a chunk based on a Fang_Point position.
**/
static inline const Fang_Chunk *
Fang_GetChunkPoint(
    const Fang_Chunks * const chunks,
    const Fang_Point  * const position)
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
        (int8_t)(position->x / FANG_CHUNK_SIZE) + x_offset,
        (int8_t)(position->y / FANG_CHUNK_SIZE) + y_offset
    );
}

/**
 * Returns a chunk-tile based on a given position.
 *
 * The position is divided by the chunk size (i.e. number of tiles per chunk) to
 * acquire chunk index values. These values are then used to call
 * Fang_GetIndexecChunk() to receive a chunk.
 *
 * The given position is reduced to its modulo value with FANG_CHUNK_SIZE to
 * acquire the tile index within the chunk.
 *
 * Positions should not exceed FANG_CHUNK_MIN * FANG_CHUNK_SIZE or
 * FANG_CHUNK_MAX * FANG_CHUNK_SIZE - 1.
 *
 * If the chunk's tile at that given index does not have a type associated with
 * it (i.e. is FANG_TILETYPE_NONE), this function returns NULL.
**/
#define Fang_GetChunkTile(chunks, position) _Generic(position,  \
    const Fang_Vec2*:  Fang_GetChunkTileVec2,                   \
          Fang_Vec2*:  Fang_GetChunkTileVec2,                   \
    const Fang_Vec3*:  Fang_GetChunkTileVec3,                   \
          Fang_Vec3*:  Fang_GetChunkTileVec3,                   \
    const Fang_Point*: Fang_GetChunkTilePoint,                  \
          Fang_Point*: Fang_GetChunkTilePoint                   \
    )(chunks, position)

/**
 * Returns a chunk-tile based on a Fang_Vec2 position.
**/
static inline const Fang_Tile *
Fang_GetChunkTileVec2(
    const Fang_Chunks * const chunks,
    const Fang_Vec2   * const position)
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

/**
 * Returns a chunk-tile based on a Fang_Vec3 position.
 *
 * Because chunk division only happens in two dimensions, the Z coordinate of
 * the position is unused here.
**/
static inline const Fang_Tile *
Fang_GetChunkTileVec3(
    const Fang_Chunks * const chunks,
    const Fang_Vec3   * const position)
{
    assert(chunks);
    assert(position);

    return Fang_GetChunkTileVec2(
        chunks, &(Fang_Vec2){.x = position->x, .y = position->y}
    );
}

/**
 * Returns a chunk-tile based on a Fang_Point position.
**/
static inline const Fang_Tile *
Fang_GetChunkTilePoint(
    const Fang_Chunks * const chunks,
    const Fang_Point  * const position)
{
    assert(chunks);
    assert(position);

    return Fang_GetChunkTileVec2(
        chunks, &(Fang_Vec2){.x = (float)position->x, .y = (float)position->y}
    );
}
