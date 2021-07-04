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
    float     delta;
} Fang_LerpVec2;

#define Fang_Lerp(x) _Generic(x,  \
    Fang_LerpVec2*: Fang_LerpStepVec2 \
    )(x)

static inline void
Fang_LerpStepVec2(
    Fang_LerpVec2 * const vec)
{
    assert(vec);

    const float dt = FANG_DELTA_TIME_S / vec->delta;

    vec->value.x = (fabsf(vec->value.x - vec->target.x) > vec->target.x * dt)
        ? vec->value.x * (1.0f - dt) + vec->target.x * dt
        : vec->target.x;

    vec->value.y = (fabsf(vec->value.y - vec->target.y) > vec->target.y * dt)
        ? vec->value.y * (1.0f - dt) + vec->target.y * dt
        : vec->target.y;
}
