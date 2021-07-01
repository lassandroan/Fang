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

enum {
    FANG_RAY_MAX_STEPS = 64,
};

typedef enum Fang_Face {
    FANG_FACE_NORTH  = 0,
    FANG_FACE_SOUTH  = 1,
    FANG_FACE_EAST   = 2,
    FANG_FACE_WEST   = 3,
    FANG_FACE_TOP    = 4,
    FANG_FACE_BOTTOM = 5,
} Fang_Face;

typedef struct Fang_RayHit {
    const Fang_Tile * tile;
    Fang_Vec2  front_hit;
    float      front_dist;
    Fang_Vec2  back_hit;
    float      back_dist;
    Fang_Face  norm_dir;
} Fang_RayHit;

typedef struct Fang_Ray {
    Fang_RayHit hits[FANG_RAY_MAX_STEPS];
    size_t      hit_count;
} Fang_Ray;

static inline void
Fang_RayCast(
    const Fang_Camera * const camera,
    const Fang_Map    * const map,
          Fang_Ray    * const rays,
    const size_t              count)
{
    assert(camera);
    assert(map);
    assert(rays);
    assert(count);

    const Fang_Vec3 * const dir = &camera->dir;
    const Fang_Vec3 * const cam = &camera->cam;
    const Fang_Vec2         pos = (Fang_Vec2){
        .x = camera->pos.x,
        .y = camera->pos.y
    };

    memset(rays, 0, sizeof(Fang_Ray) * count);

    for (size_t i = 0; i < count; ++i)
    {
        /* X coordinate in camera space, normalized -1.0f..1.0f */
        const float ray_cam = 2.0f * (1.0f - (float)i / (float)count) - 1.0f;

        /* Map ray start position onto camera plane */
        const Fang_Vec2 cam_ray = {
            .x = dir->x + cam->x * ray_cam,
            .y = dir->y + cam->y * ray_cam,
        };

        const Fang_Vec2 delta_dist = {
            .x = (cam_ray.y == 0.0f)
                ? 0.0f
                : cam_ray.x == 0.0f ? 0.0f : fabsf(1.0f / cam_ray.x),
            .y = (cam_ray.x == 0.0f)
                ? 0.0f
                : (cam_ray.y == 0.0f) ? 0.0f : fabsf(1.0f / cam_ray.y),
        };

        Fang_Vec2 int_pos = {
            .x = floorf(pos.x),
            .y = floorf(pos.y),
        };

        Fang_Vec2 step_dist;
        Fang_Vec2 side_dist;
        Fang_Face side_face_x;
        Fang_Face side_face_y;

        if (cam_ray.x < 0.0f)
        {
            step_dist.x = -1.0f;
            side_dist.x = (pos.x - int_pos.x) * delta_dist.x;
            side_face_x  = FANG_FACE_EAST;
        }
        else
        {
            step_dist.x = 1.0f;
            side_dist.x = (int_pos.x + 1.0f - pos.x) * delta_dist.x;
            side_face_x  = FANG_FACE_WEST;
        }

        if (cam_ray.y < 0.0f)
        {
            step_dist.y = -1.0f;
            side_dist.y = (pos.y - int_pos.y) * delta_dist.y;
            side_face_y  = FANG_FACE_SOUTH;
        }
        else
        {
            step_dist.y = 1.0f;
            side_dist.y = (int_pos.y + 1.0f - pos.y) * delta_dist.y;
            side_face_y  = FANG_FACE_NORTH;
        }

        size_t hit_count = 0;

        for (size_t step = 0; step < FANG_RAY_MAX_STEPS; ++step)
        {
            Fang_RayHit * const hit = &rays[i].hits[hit_count];

            if (side_dist.x < side_dist.y)
            {
                int_pos.x    += step_dist.x;
                side_dist.x  += delta_dist.x;
                hit->norm_dir = side_face_x;
            }
            else if (side_dist.x > side_dist.y)
            {
                int_pos.y    += step_dist.y;
                side_dist.y  += delta_dist.y;
                hit->norm_dir = side_face_y;
            }
            else /* 0 case */
            {
                if (step_dist.x < step_dist.y)
                {
                    int_pos.x    += step_dist.x;
                    side_dist.x  += delta_dist.x;
                    hit->norm_dir = side_face_x;
                }
                else
                {
                    int_pos.y    += step_dist.y;
                    side_dist.y  += delta_dist.y;
                    hit->norm_dir = side_face_y;
                }
            }

            hit->tile = Fang_MapQuery(map, (int)int_pos.x, (int)int_pos.y);

            if (hit->tile)
            {
                /* Check the axis of collision */
                if (hit->norm_dir == side_face_x)
                {
                    hit->front_dist = (
                        int_pos.x - pos.x + (1.0f - step_dist.x) / 2.0f
                    );

                    if (cam_ray.x != 0.0f)
                        hit->front_dist /= cam_ray.x;
                }
                else if (hit->norm_dir == side_face_y)
                {
                    hit->front_dist = (
                        int_pos.y - pos.y + (1.0f - step_dist.y) / 2.0f
                    );

                    if (cam_ray.y != 0.0f)
                        hit->front_dist /= cam_ray.y;
                }

                hit->front_hit = (Fang_Vec2){
                    .x = pos.x + (hit->front_dist * cam_ray.x),
                    .y = pos.y + (hit->front_dist * cam_ray.y),
                };

                const Fang_Vec2 old_int_pos = int_pos;
                const Fang_Vec2 old_side_dist = side_dist;

                /* Run an additional increment to calculate the back face hit */
                {
                    Fang_Face face = hit->norm_dir;

                    if (side_dist.x < side_dist.y)
                    {
                        int_pos.x += step_dist.x;
                        side_dist.x += delta_dist.x;
                        face = side_face_x;
                    }
                    else if (side_dist.x > side_dist.y)
                    {
                        int_pos.y += step_dist.y;
                        side_dist.y += delta_dist.y;
                        face = side_face_y;
                    }
                    else /* 0 case */
                    {
                        if (step_dist.x < step_dist.y)
                        {
                            int_pos.x += step_dist.x;
                            side_dist.x += delta_dist.x;
                            face = side_face_x;
                        }
                        else
                        {
                            int_pos.y += step_dist.y;
                            side_dist.y += delta_dist.y;
                            face = side_face_y;
                        }
                    }

                    /* Check the axis of collision */
                    if (face == side_face_x)
                    {
                        hit->back_dist = (
                            int_pos.x - pos.x + (1.0f - step_dist.x) / 2.0f
                        );

                        if (cam_ray.x != 0.0f)
                            hit->back_dist /= cam_ray.x;
                    }
                    else if (face == side_face_y)
                    {
                        hit->back_dist = (
                            int_pos.y - pos.y + (1.0f - step_dist.y) / 2.0f
                        );

                        if (cam_ray.y != 0.0f)
                            hit->back_dist /= cam_ray.y;
                    }

                    hit->back_hit = (Fang_Vec2){
                        .x = pos.x + (hit->back_dist * cam_ray.x),
                        .y = pos.y + (hit->back_dist * cam_ray.y),
                    };
                }

                int_pos   = old_int_pos;
                side_dist = old_side_dist;

                hit_count++;
            }
        }

        rays[i].hit_count = hit_count;
    }
}
