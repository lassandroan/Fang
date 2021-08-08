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

typedef struct Fang_LerpVec2 {
    Fang_Vec2 target;
    Fang_Vec2 value;
    Fang_Vec2 delta;
} Fang_LerpVec2;

typedef struct Fang_LerpVec3 {
    Fang_Vec3 target;
    Fang_Vec3 value;
    Fang_Vec3 delta;
} Fang_LerpVec3;

#define Fang_Lerp(value, delta) _Generic(value,  \
    Fang_LerpVec2*: Fang_LerpStepVec2, \
    Fang_LerpVec3*: Fang_LerpStepVec3  \
    )(value, delta)

static inline void
Fang_LerpStepVec2(
          Fang_LerpVec2 * const vec,
    const float                 delta)
{
    assert(vec);

    const Fang_Vec2 dt = (Fang_Vec2){
        .x = vec->delta.x * delta,
        .y = vec->delta.y * delta,
    };

    vec->value.x = (fabsf(vec->value.x - vec->target.x) > vec->target.x * dt.x)
        ? vec->value.x * (1.0f - dt.x) + vec->target.x * dt.x
        : vec->target.x;

    vec->value.y = (fabsf(vec->value.y - vec->target.y) > vec->target.y * dt.y)
        ? vec->value.y * (1.0f - dt.y) + vec->target.y * dt.y
        : vec->target.y;
}

static inline void
Fang_LerpStepVec3(
          Fang_LerpVec3 * const vec,
    const float                 delta)
{
    assert(vec);

    const Fang_Vec3 dt = (Fang_Vec3){
        .x = vec->delta.x * delta,
        .y = vec->delta.y * delta,
        .z = vec->delta.z * delta,
    };

    vec->value.x = (fabsf(vec->value.x - vec->target.x) > vec->target.x * dt.x)
        ? vec->value.x * (1.0f - dt.x) + vec->target.x * dt.x
        : vec->target.x;

    vec->value.y = (fabsf(vec->value.y - vec->target.y) > vec->target.y * dt.y)
        ? vec->value.y * (1.0f - dt.y) + vec->target.y * dt.y
        : vec->target.y;

    vec->value.z = (fabsf(vec->value.z - vec->target.z) > vec->target.z * dt.z)
        ? vec->value.z * (1.0f - dt.z) + vec->target.z * dt.z
        : vec->target.z;
}
