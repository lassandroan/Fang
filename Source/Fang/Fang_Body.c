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
 * A structure representing a physical body in the game world.
 *
 * Body positions represent the bottom of the body, so effectively the "head"
 * would be at Fang_Body.pos.z + Fang_Body.size.
**/
typedef struct Fang_Body {
    Fang_Vec3 pos;  /* Current Position     */
    Fang_Vec3 dir;  /* Current Direction    */
    Fang_Vec3 vel;  /* Current Velocity     */
    Fang_Vec2 acc;  /* Acceleration Speeds  */
    Fang_Vec3 max;  /* Max Velocity Values  */
    float     size; /* Body Size (All Axes) */
    Fang_Vec3 last; /* Previous Position    */
    bool      jump; /* Jump State           */
} Fang_Body;

static inline bool
Fang_BodyCanStep(
    const Fang_Body * const body,
    const float             surface_top)
{
    assert(body);

    return (surface_top <= body->pos.z + (body->size / 3.0f));
}

static inline bool
Fang_BodyCollideMap(
    const Fang_Body * const body,
    const Fang_Map  * const map)
{
    assert(body);
    assert(map);

    const Fang_Tile * const tile = Fang_MapQuery(
        map, (int)floorf(body->pos.x), (int)floorf(body->pos.y)
    );

    if (!tile)
        return false;

    const float body_top = body->pos.z + body->size;
    const float tile_top = tile->y + tile->h;

    if (tile->y >= body_top)
        return false;

    if (tile_top <= body->pos.z)
        return false;

    if (Fang_BodyCanStep(body, tile_top))
        return false;

    return true;
}

static inline float
Fang_BodyFindFloor(
    const Fang_Body * const body,
    const Fang_Map  * const map)
{
    assert(body);
    assert(map);

    const Fang_Tile * const tile = Fang_MapQuery(
        map, (int)floorf(body->pos.x), (int)floorf(body->pos.y)
    );

    if (!tile)
        return 0.0f;

    const float tile_top = tile->y + tile->h;

    if (tile_top > body->pos.z)
        return 0.0f;

    return tile_top;
}

static inline float
Fang_BodyFindStep(
    const Fang_Body * const body,
    const Fang_Map  * const map)
{
    assert(body);
    assert(map);

    const Fang_Tile * const tile = Fang_MapQuery(
        map, (int)floorf(body->pos.x), (int)floorf(body->pos.y)
    );

    if (!tile)
        return 0.0f;

    const float tile_top = tile->y + tile->h;

    if (Fang_BodyCanStep(body, tile_top))
        return tile_top;

    return 0.0f;
}

/**
 * Moves a body using a given 'move' vector.
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
          Fang_Body * const body,
          Fang_Map  * const map,
    const Fang_Vec3 * const move,
    const float             delta)
{
    assert(body);
    assert(map);
    assert(move);

    body->last = body->pos;

    for (size_t i = 0; i < 2; ++i)
    {
        float * const target = &body->vel.xyz[i];

        if (fabsf(move->xyz[i]) < FLT_EPSILON)
        {
            const float delta_acc = body->acc.xy[i] * delta;

            if (*target < 0.0f)
            {
                if (*target + delta_acc < 0.0f)
                    *target += delta_acc;
                else
                    *target = 0.0f;
            }
            else if (*target > 0.0f)
            {
                if (*target - delta_acc > 0.0f)
                    *target -= delta_acc;
                else
                    *target = 0.0f;
            }
        }

        *target = clamp(
            *target + ((body->acc.xy[i] * delta) * move->xyz[i]),
            -body->max.xyz[i],
            body->max.xyz[i]
        );
    }

    {
        if (!body->jump && move->z > 0.0f)
        {
            if (body->vel.z >= -FANG_JUMP_TOLERANCE && body->vel.z <= 0.0f)
            {
                body->jump  = true;
                body->vel.z = move->z;
            }
        }

        const float standing_surface = Fang_BodyFindFloor(body, map);

        if (body->pos.z > standing_surface)
            body->vel.z -= FANG_GRAVITY * delta;
    }

    Fang_Vec3 new = body->pos;
    new.x += (body->dir.x * body->vel.x + body->dir.y * body->vel.y) * delta;
    new.y += (body->dir.y * body->vel.x - body->dir.x * body->vel.y) * delta;
    new.z += body->vel.z * delta;

    /* X velocity collision */
    {
        Fang_Body test_body = *body;
        test_body.pos.x     = new.x;

        if (!Fang_BodyCollideMap(&test_body, map))
            body->pos.x = new.x;
    }

    /* Y velocity collision */
    {
        Fang_Body test_body = *body;
        test_body.pos.y     = new.y;

        if (!Fang_BodyCollideMap(&test_body, map))
            body->pos.y = new.y;
    }

    /* Z velocity collision */
    {
        Fang_Body test_body = *body;
        test_body.pos.z     = new.z;

        if (!Fang_BodyCollideMap(&test_body, map))
        {
            body->pos.z = new.z;
        }
        else
        {
            body->vel.z = 0.0f;
            body->jump = false;
        }
    }

    /* If we moved onto a short tile, step up onto it */
    {
        const float standing_surface = Fang_BodyFindStep(body, map);

        if (body->pos.z <= standing_surface)
        {
            body->vel.z = 0.0f;
            body->pos.z = standing_surface;
            body->jump  = false;
        }
    }
}

static inline bool
Fang_BodyCollideBody(
    Fang_Body * const a,
    Fang_Body * const b)
{
    assert(a);
    assert(b);

    const bool a_above_b = a->pos.z >= b->pos.z + b->size;
    const bool b_above_a = b->pos.z >= a->pos.z + a->size;

    if (a_above_b || b_above_a)
        return false;

    const float dx   = a->pos.x - b->pos.x;
    const float dy   = a->pos.y - b->pos.y;
    const float dist = sqrtf(dx * dx + dy * dy);

    if (dist <= a->size + b->size)
    {
        a->pos = a->last;

        {
            const Fang_Vec3 temp = a->dir;
            a->dir = b->dir;
            b->dir = temp;
        }

        const Fang_Vec3 temp = Fang_Vec3Multf(a->vel, 10.0f);

        a->vel = Fang_Vec3Multf(b->vel, 10.0f);
        b->vel = temp;

        a->vel.z = 0.0f;
        b->vel.z = 0.0f;

        a->jump = false;
        b->jump = false;

        return true;
    }

    return false;
}
