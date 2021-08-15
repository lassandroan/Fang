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
typedef struct Fang_ChunkLocation {
    int8_t x, y;
} Fang_ChunkLocation;

/**
 *
**/
static Fang_Chunk *
Fang_GetChunk(
          Fang_ChunkSet      * const chunks,
    const Fang_ChunkLocation * const location)
{
    assert(chunks);
    assert(location);

    static int8_t FANG_CHUNK_MAX =  (1 << 6) - 1;
    static int8_t FANG_CHUNK_MIN = -(1 << 6);

    assert(location->x >= FANG_CHUNK_MIN);
    assert(location->x <= FANG_CHUNK_MAX);
    assert(location->y >= FANG_CHUNK_MIN);
    assert(location->y <= FANG_CHUNK_MAX);

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

    uint16_t index = 0;
    for (uint8_t i = 0; i < 32; i += 4)
    {
        const uint8_t xbits = (location->x >> i) & 0b1111;
        const uint8_t ybits = (location->y >> i) & 0b1111;
        index |= (lookup[xbits] | (lookup[ybits] << 1)) << (i * 2);
    }

    assert(index < FANG_CHUNK_COUNT);

    return &chunks->chunks[index];
}

/**
 *
**/
static Fang_Tile *
Fang_GetChunkTile(
          Fang_ChunkSet * const chunks,
    const Fang_Vec3     * const vec)
{
    assert(chunks);
    assert(vec);

    Fang_Vec2 integral;
    Fang_Vec2 fractional;

    fractional.x = modff(vec->x / (float)FANG_CHUNK_SIZE, &integral.x);
    fractional.y = modff(vec->y / (float)FANG_CHUNK_SIZE, &integral.y);

    fractional.x = (fractional.x < 0.0f) ? 1.0f - fractional.x : fractional.x;
    fractional.y = (fractional.y < 0.0f) ? 1.0f - fractional.y : fractional.y;

    assert(integral.x >= (float)INT8_MIN);
    assert(integral.x <= (float)INT8_MAX);
    assert(integral.y >= (float)INT8_MIN);
    assert(integral.y <= (float)INT8_MAX);

    Fang_Chunk * const chunk = Fang_GetChunk(
        chunks,
        &(Fang_ChunkLocation){
            .x = (int8_t)integral.x,
            .y = (int8_t)integral.y,
        }
    );

    assert(chunk);

    const Fang_Point tile_index = {
        .x = (int)(fractional.x * FANG_CHUNK_SIZE),
        .y = (int)(fractional.y * FANG_CHUNK_SIZE),
    };

    return &chunk->tiles[tile_index.x][tile_index.y];
}
