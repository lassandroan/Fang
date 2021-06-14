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

typedef struct Fang_Mat3x3 {
    float m00, m01, m02,
          m10, m11, m12,
          m20, m21, m22;
} Fang_Mat3x3;

#define Fang_MatMult(a, b) _Generic((b), \
        Fang_Vec3:   Fang_Mat3x3Multv, \
        Fang_Point:  Fang_Mat3x3Multp, \
        Fang_Mat3x3: Fang_Mat3x3Mult   \
    )(a, b)

static inline Fang_Mat3x3
Fang_Mat3x3Identity(void)
{
    return (Fang_Mat3x3){
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
    };
}

static inline Fang_Mat3x3
Fang_Mat3x3Mult(
    const Fang_Mat3x3 a,
    const Fang_Mat3x3 b)
{
    return (Fang_Mat3x3){
        a.m00 * b.m00 + a.m01 * b.m10 + a.m02 * b.m20,
        a.m00 * b.m01 + a.m01 * b.m11 + a.m02 * b.m21,
        a.m00 * b.m02 + a.m01 * b.m12 + a.m02 * b.m22,

        a.m10 * b.m00 + a.m11 * b.m10 + a.m12 * b.m20,
        a.m10 * b.m01 + a.m11 * b.m11 + a.m12 * b.m21,
        a.m10 * b.m02 + a.m11 * b.m12 + a.m12 * b.m22,

        a.m20 * b.m00 + a.m21 * b.m10 + a.m22 * b.m20,
        a.m20 * b.m01 + a.m21 * b.m11 + a.m22 * b.m21,
        a.m20 * b.m02 + a.m21 * b.m12 + a.m22 * b.m22,
    };
}

static inline Fang_Vec3
Fang_Mat3x3Multv(
    const Fang_Mat3x3 a,
    const Fang_Vec3   b)
{
    return (Fang_Vec3){
        a.m00 * b.x + a.m01 * b.y + a.m02 * b.z,
        a.m10 * b.x + a.m11 * b.y + a.m12 * b.z,
        a.m20 * b.x + a.m21 * b.y + a.m22 * b.z,
    };
}

static inline Fang_Point
Fang_Mat3x3Multp(
    const Fang_Mat3x3 a,
    const Fang_Point  b)
{
    const Fang_Vec3 result = Fang_Mat3x3Multv(
        a, (Fang_Vec3){.x = b.x, .y = b.y, .z = 1.0f}
    );

    return (Fang_Point){.x = (int)result.x, .y = (int)result.y};
}

static inline Fang_Mat3x3
Fang_Mat3x3Translate(
    const float x,
    const float y)
{
    return (Fang_Mat3x3){
        1.0f, 0.0f,    x,
        0.0f, 1.0f,    y,
        0.0f, 0.0f, 1.0f,
    };
}

static inline Fang_Mat3x3
Fang_Mat3x3Scale(
    const float x,
    const float y)
{
    return (Fang_Mat3x3){
           x, 0.0f, 0.0f,
        0.0f,    y, 0.0f,
        0.0f, 0.0f, 1.0f,
    };
}
