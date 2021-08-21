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

/**
 * Flags that control the body behavior in the physics system.
**/
typedef enum Fang_BodyFlags {
    FANG_BODYFLAG_NONE             = 0,
    FANG_BODYFLAG_JUMP             = 1 << 1,
    FANG_BODYFLAG_STEP             = 1 << 2,
    FANG_BODYFLAG_FALL             = 1 << 3,
    FANG_BODYFLAG_COLLIDE_WALLS    = 1 << 4,
    FANG_BODYFLAG_COLLIDE_BODIES   = 1 << 5,
} Fang_BodyFlags;

/**
 * A structure representing a physical body in the game world.
 *
 * Body positions represent the bottom of the body, so effectively the "head"
 * would be at Fang_Body.pos.z + Fang_Body.size.
**/
typedef struct Fang_Body {
    int           flags;
    Fang_Vec3     pos;
    Fang_Vec3     last;
    Fang_Vec3     dir;
    Fang_LerpVec3 vel;
    float         height;
    float         width;
    bool          jump;
} Fang_Body;

static inline void
Fang_BodySetTargetVelocity(
          Fang_Body * const body,
    const float             forward,
    const float             left)
{
    assert(body);

    const Fang_Vec3 dir_cross = {
        .x =  body->dir.y,
        .y = -body->dir.x,
        .z =  body->dir.z,
    };

    body->vel.target = Fang_Vec3Add(
        Fang_Vec3Multf(Fang_Vec3Normalize(body->dir), forward),
        Fang_Vec3Multf(Fang_Vec3Normalize(dir_cross),    left)
    );
}

static inline bool
Fang_BodyCanStep(
    const Fang_Body * const body,
    const float             surface_top)
{
    assert(body);

    if (!(body->flags & FANG_BODYFLAG_STEP))
        return false;

    return (surface_top <= body->pos.z + (body->height / 3.0f));
}

static inline bool
Fang_BodyCollidesTile(
    const Fang_Body     * const body,
    const Fang_ChunkSet * const chunks)
{
    assert(body);
    assert(chunks);

    if (!(body->flags & FANG_BODYFLAG_COLLIDE_WALLS))
        return false;

    const Fang_Tile * const tile = Fang_GetChunkTile(chunks, &body->pos);

    if (!tile)
        return false;

    const float body_top = body->pos.z + body->height;
    const float tile_top = tile->offset + tile->height;

    if (tile->offset >= body_top)
        return false;

    if (tile_top <= body->pos.z)
        return false;

    if (Fang_BodyCanStep(body, tile_top))
        return false;

    return true;
}

static inline float
Fang_BodyFindFloor(
    const Fang_Body     * const body,
    const Fang_ChunkSet * const chunks)
{
    assert(body);
    assert(chunks);

    if (!(body->flags & FANG_BODYFLAG_COLLIDE_WALLS))
        return 0.0f;

    const Fang_Tile * const tile = Fang_GetChunkTile(chunks, &body->pos);

    if (!tile)
        return 0.0f;

    const float tile_top = tile->offset + tile->height;

    if (tile_top > body->pos.z)
        return 0.0f;

    return tile_top;
}

static inline float
Fang_BodyFindStep(
    const Fang_Body     * const body,
    const Fang_ChunkSet * const chunks)
{
    assert(body);
    assert(chunks);

    if (!(body->flags & FANG_BODYFLAG_COLLIDE_WALLS))
        return 0.0f;

    const Fang_Tile * const tile = Fang_GetChunkTile(chunks, &body->pos);

    if (!tile)
        return 0.0f;

    const float tile_top = tile->offset + tile->height;

    if (Fang_BodyCanStep(body, tile_top))
        return tile_top;

    return 0.0f;
}

/**
 * Moves a body using its given 'move' vector.
 *
 * The X and Y values of the move vector will be multiplied with the body's
 * acceleration values to determine the increase/decrease in velocity.
 *
 * The Z value of the move vector will be directly applied as the body's new
 * Z velocity, but only if the body is standing on a surface.
 *
 * If the movement vector is 0.0f for X/Y, the current velocities for those axes
 * will be decreased until they reach 0.0f. The same applies to the Z axis, but
 * instead it is gravity acting on the body if its position is above a surface.
 *
 * If the body is in the air, deceleration in the X/Y axes will not occur.
**/
static void
Fang_BodyMove(
          Fang_Body     * const body,
    const Fang_ChunkSet * const chunks,
    const float                 delta)
{
    assert(body);
    assert(chunks);

    body->last = body->pos;

    Fang_LerpVec3 * const vel = &body->vel;

    if (!body->jump && vel->target.z > 0.0f)
    {
        if (vel->value.z >= -FANG_JUMP_TOLERANCE && vel->value.z <= 0.0f)
        {
            body->jump = true;
            body->vel.value.z = body->vel.target.z;
        }
    }

    if (body->flags & FANG_BODYFLAG_FALL)
    {
        const float standing_surface = Fang_BodyFindFloor(body, chunks);

        if (body->pos.z > standing_surface)
            vel->value.z -= FANG_GRAVITY * delta;
    }

    Fang_Lerp(vel, delta);

    body->pos = Fang_Vec3Add(body->pos, Fang_Vec3Multf(vel->value, delta));
}

/**
 * Returns whether two bodies currently intersect, accounting for both height
 * and size.
**/
static inline bool
Fang_BodiesIntersect(
    const Fang_Body * const a,
    const Fang_Body * const b)
{
    assert(a);
    assert(b);

    const bool a_above_b = a->pos.z > b->pos.z + b->height;
    const bool b_above_a = b->pos.z > a->pos.z + a->height;

    if (a_above_b || b_above_a)
        return false;

    const float dx = a->pos.x - b->pos.x;
    const float dy = a->pos.y - b->pos.y;

    return sqrtf(dx * dx + dy * dy) <= a->width + b->width;
}

/**
 * Resolves collisions between a body and the map tiles.
 *
 * This accounts for wall collisions as well as keeping the body standing on
 * the surface below it (if it is below said surface).
 *
 * If the body can step onto short tiles, this finds potential steps and applies
 * them to the body.
 *
 * If the body cannot collide with walls, this function does nothing.
**/
static inline bool
Fang_BodyResolveMapCollision(
          Fang_Body     * const body,
    const Fang_ChunkSet * const chunks)
{
    assert(body);
    assert(chunks);

    if (!(body->flags & FANG_BODYFLAG_COLLIDE_WALLS))
        return false;

    Fang_Body test_body = *body;
    test_body.pos = test_body.last;

    bool result = false;

    /* X collision */
    test_body.pos.x = body->pos.x;

    if (Fang_BodyCollidesTile(&test_body, chunks))
    {
        result = true;
        test_body.pos.x = test_body.last.x;
    }

    /* Y collision */
    test_body.pos.y = body->pos.y;

    if (Fang_BodyCollidesTile(&test_body, chunks))
    {
        result = true;
        test_body.pos.y = test_body.last.y;
    }

    /* Z collision */
    test_body.pos.z = body->pos.z;

    if (Fang_BodyCollidesTile(&test_body, chunks))
    {
        result = true;
        body->jump        = false;
        body->vel.value.z = 0.0f;
        test_body.pos.z   = test_body.last.z;
    }

    body->pos = test_body.pos;

    /* If we moved onto a short tile, step up onto it */
    const float standing_surface = Fang_BodyFindStep(body, chunks);

    if (body->pos.z <= standing_surface)
    {
        body->jump        = false;
        body->pos.z       = standing_surface;
        body->vel.value.z = 0.0f;
    }

    return result;
}

/**
 * Calculates the new position for bodies that have collided.
 *
 * When bodies have the collide-body flag enabled, they should not be able to
 * pass through one another. Collisions of this type stop both bodies from
 * moving, and cancel the jump state of whichever bodies may have been jumping.
 *
 * This function also resolves discrepancies in Z positions, allowing falling
 * bodies to land on top of other collideable bodies.
**/
static inline void
Fang_BodyResolveBodyCollision(
    Fang_Body * const a,
    Fang_Body * const b)
{
    assert(a);
    assert(b);

    if (!(a->flags & FANG_BODYFLAG_COLLIDE_BODIES))
        return;

    if (!(b->flags & FANG_BODYFLAG_COLLIDE_BODIES))
        return;

    a->jump = false;
    b->jump = false;

    if (a->pos.z > b->pos.z && a->vel.value.z < 0.0f)
    {
        a->vel.value.z = 0.0f;
        a->pos.z = b->pos.z + b->height;

        return;
    }

    if (b->pos.z > a->pos.z && b->vel.value.z < 0.0f)
    {
        b->vel.value.z = 0.0f;
        b->pos.z = a->pos.z + a->height;

        return;
    }

    a->pos.x = a->last.x;
    a->pos.y = a->last.y;
    b->pos.x = b->last.x;
    b->pos.y = b->last.y;
    a->vel.value.x = 0.0f;
    a->vel.value.y = 0.0f;
    b->vel.value.x = 0.0f;
    b->vel.value.y = 0.0f;
}
