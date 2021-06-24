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

typedef struct Fang_Vec2 {
    float x, y;
} Fang_Vec2;

typedef struct Fang_Vec3 {
    float x, y, z;
} Fang_Vec3;

static inline Fang_Vec2
Fang_Vec2Divf(
    const Fang_Vec2 a,
    const float     b)
{
    return (Fang_Vec2){a.x / b, a.y / b};
}

static inline float
Fang_Vec2Dot(
    const Fang_Vec2 a,
    const Fang_Vec2 b)
{
    return (a.x * b.x) + (a.y * b.y);
}

static inline float
Fang_Vec2Determ(
    const Fang_Vec2 a,
    const Fang_Vec2 b)
{
    return (a.x * b.y) - (a.y * b.x);
}

static inline float
Fang_Vec2Angle(
    const Fang_Vec2 a,
    const Fang_Vec2 b)
{
    return atan2f(Fang_Vec2Dot(a, b), Fang_Vec2Determ(a, b));
}

static inline float
Fang_Vec2Norm(
    const Fang_Vec2 a)
{
    return sqrtf(a.x * a.x + a.y * a.y);
}

static inline Fang_Vec2
Fang_Vec2Normalize(
    const Fang_Vec2 a)
{
    return Fang_Vec2Divf(a, Fang_Vec2Norm(a));
}
