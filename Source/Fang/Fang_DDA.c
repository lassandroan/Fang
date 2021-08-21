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

typedef enum Fang_Face {
    FANG_FACE_NORTH  = 0,
    FANG_FACE_SOUTH  = 1,
    FANG_FACE_EAST   = 2,
    FANG_FACE_WEST   = 3,
    FANG_FACE_TOP    = 4,
    FANG_FACE_BOTTOM = 5,
} Fang_Face;

/**
 * A structure containing the necessary data for the Digital Differential
 * Analyzer.
**/
typedef struct Fang_DDAState {
    Fang_Vec2 start; /* Starting position of the DDA                      */
    Fang_Vec2 dir;   /* Starting direction vector of the DDA              */
    Fang_Vec2 pos;   /* Current integer position (tile) of the DDA        */
    Fang_Vec2 delta; /* Distance it takes (X/Y) to step over a tile       */
    Fang_Vec2 step;  /* Direction (X/Y) to step when moving the position  */
    Fang_Vec2 side;  /* Distance from the start position to the next tile */
    Fang_Face face;  /* Face direction at the current position            */
} Fang_DDAState;

/**
 * Initializes the DDA based on a given starting position and direction vector.
**/
static inline void
Fang_InitDDA(
          Fang_DDAState * const dda,
    const Fang_Vec2     * const start,
    const Fang_Vec2     * const dir)
{
    assert(dda);
    assert(start);
    assert(dir);

    dda->start = *start;
    dda->dir   = *dir;

    dda->pos = (Fang_Vec2){
        .x = floorf(start->x),
        .y = floorf(start->y),
    };

    dda->delta = (Fang_Vec2){
        .x = (dir->y == 0.0f)
            ? 0.0f
            : (dir->x == 0.0f) ? 0.0f : fabsf(1.0f / dir->x),
        .y = (dir->x == 0.0f)
            ? 0.0f
            : (dir->y == 0.0f) ? 0.0f : fabsf(1.0f / dir->y),
    };

    if (dir->x < 0.0f)
    {
        dda->step.x = -1.0f;
        dda->side.x = (start->x - dda->pos.x) * dda->delta.x;
    }
    else
    {
        dda->step.x = 1.0f;
        dda->side.x = (dda->pos.x + 1.0f - start->x) * dda->delta.x;
    }

    if (dir->y < 0.0f)
    {
        dda->step.y = -1.0f;
        dda->side.y = (start->y - dda->pos.y) * dda->delta.y;
    }
    else
    {
        dda->step.y = 1.0f;
        dda->side.y = (dda->pos.y + 1.0f - start->y) * dda->delta.y;
    }
}

/**
 * Increments the DDA by one step, returning the distance from the starting
 * position to the current point.
**/
static float
Fang_StepDDA(
    Fang_DDAState * const dda)
{
    assert(dda);

    const Fang_Face x_face = (dda->step.x < 0.0f)
        ? FANG_FACE_EAST
        : FANG_FACE_WEST;

    const Fang_Face y_face = (dda->step.y < 0.0f)
        ? FANG_FACE_SOUTH
        : FANG_FACE_NORTH;

    if (dda->side.x < dda->side.y)
    {
        dda->pos.x  += dda->step.x;
        dda->side.x += dda->delta.x;
        dda->face    = x_face;
    }
    else if (dda->side.x > dda->side.y)
    {
        dda->pos.y  += dda->step.y;
        dda->side.y += dda->delta.y;
        dda->face    = y_face;
    }
    else /* 0 case */
    {
        if (dda->step.x < dda->step.y)
        {
            dda->pos.x  += dda->step.x;
            dda->side.x += dda->delta.x;
            dda->face    = x_face;
        }
        else if (dda->step.x > dda->step.y)
        {
            dda->pos.y  += dda->step.y;
            dda->side.y += dda->delta.y;
            dda->face    = y_face;
        }
    }

    float result;

    if (dda->face == x_face)
    {
        result = dda->pos.x - dda->start.x + (1.0f - dda->step.x) / 2.0f;

        if (dda->dir.x != 0.0f)
            result /= dda->dir.x;
    }
    else
    {
        result = dda->pos.y - dda->start.y + (1.0f - dda->step.y) / 2.0f;

        if (dda->dir.y != 0.0f)
            result /= dda->dir.y;
    }

    return result;
}
