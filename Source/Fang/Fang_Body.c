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
} Fang_Body;

/**
 * Moves a body using a given 'move' vector.
 *
 * The X and Y values of the move vector will be multiplied with the body's
 * acceleration values to determine the increase/decrease in velocity.
 *
 * The Z value of the move vector will be directly applied as the body's new
 * Z velocity.
 *
 * If the movement vector is 0.0f for X/Y, the current velocities for those axes
 * will be decreased until they reach 0.0f. The same applies to the Z axis, but
 * instead it is gravity acting on the body if its position is above the floor.
 *
 * If the body is in the air, deceleration in the X/Y axes will not occur.
**/
static void
Fang_BodyMove(
          Fang_Body * const body,
    const Fang_Vec3 * const move,
    const float             delta)
{
    assert(body);
    assert(move);

    if (body->pos.z <= 0.0f)
    {
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
    }

    if (fabsf(move->z) > FLT_EPSILON)
        body->vel.z = move->z;
    else
        body->vel.z -= FANG_GRAVITY * delta;

    if (body->pos.z + (body->vel.z * delta) <= 0.0f)
    {
        body->vel.z = 0.0f;
        body->pos.z = 0.0f;
    }

    body->pos.x += (body->dir.x * body->vel.x + body->dir.y * body->vel.y)
                 * delta;

    body->pos.y += (body->dir.y * body->vel.x - body->dir.x * body->vel.y)
                 * delta;

    body->pos.z += body->vel.z * delta;
}
