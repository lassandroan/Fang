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

typedef enum Fang_Direction {
    FANG_DIRECTION_NORTH,
    FANG_DIRECTION_SOUTH,
    FANG_DIRECTION_EAST,
    FANG_DIRECTION_WEST,
} Fang_Direction;

typedef struct Fang_RayHit {
    Fang_Vec2      front_hit;
    float          front_dist;
    Fang_Vec2      back_hit;
    float          back_dist;
    Fang_Point     tile_pos;
    Fang_Direction norm_dir;
} Fang_RayHit;

typedef struct Fang_Ray {
    Fang_RayHit hits[FANG_RAY_MAX_STEPS];
    size_t      hit_count;
} Fang_Ray;

static inline void
Fang_RayCast(
    const Fang_Map    * const map,
    const Fang_Camera * const camera,
          Fang_Ray    * const rays,
    const size_t              count)
{
    assert(map);
    assert(camera);
    assert(rays);
    assert(count);

    const Fang_Vec3 * const dir = &camera->dir;
    const Fang_Vec3 * const cam = &camera->cam;

    const Fang_Vec2 norm_pos = {
        .x = camera->pos.x / map->tile_size,
        .y = camera->pos.y / map->tile_size,
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

        Fang_Vec2 floor_norm = {
            .x = floorf(norm_pos.x),
            .y = floorf(norm_pos.y),
        };

        Fang_Vec2 step_dist;
        Fang_Vec2 side_dist;
        Fang_Direction side_dir_x;
        Fang_Direction side_dir_y;

        if (cam_ray.x < 0.0f)
        {
            step_dist.x = -1.0f;
            side_dist.x = (norm_pos.x - floor_norm.x) * delta_dist.x;
            side_dir_x  = FANG_DIRECTION_EAST;
        }
        else
        {
            step_dist.x = 1.0f;
            side_dist.x = (floor_norm.x + 1.0f - norm_pos.x) * delta_dist.x;
            side_dir_x  = FANG_DIRECTION_WEST;
        }

        if (cam_ray.y < 0.0f)
        {
            step_dist.y = -1.0f;
            side_dist.y = (norm_pos.y - floor_norm.y) * delta_dist.y;
            side_dir_y  = FANG_DIRECTION_SOUTH;
        }
        else
        {
            step_dist.y = 1.0f;
            side_dist.y = (floor_norm.y + 1.0f - norm_pos.y) * delta_dist.y;
            side_dir_y  = FANG_DIRECTION_NORTH;
        }

        size_t hit_count = 0;

        for (size_t step = 0; step < FANG_RAY_MAX_STEPS; ++step)
        {
            Fang_RayHit * const hit = &rays[i].hits[hit_count];

            if (side_dist.x < side_dist.y)
            {
                floor_norm.x += step_dist.x;
                side_dist.x  += delta_dist.x;
                hit->norm_dir = side_dir_x;
            }
            else if (side_dist.x > side_dist.y)
            {
                floor_norm.y += step_dist.y;
                side_dist.y  += delta_dist.y;
                hit->norm_dir = side_dir_y;
            }
            else /* 0 case */
            {
                if (step_dist.x < step_dist.y)
                {
                    floor_norm.x += step_dist.x;
                    side_dist.x  += delta_dist.x;
                    hit->norm_dir = side_dir_x;
                }
                else
                {
                    floor_norm.y += step_dist.y;
                    side_dist.y  += delta_dist.y;
                    hit->norm_dir = side_dir_y;
                }
            }

            const Fang_Point tile_pos = {
                .x = (int)(floor_norm.x * map->tile_size),
                .y = (int)(floor_norm.y * map->tile_size),
            };

            const Fang_TileType wall_type = Fang_MapQueryType(
                map, tile_pos.x, tile_pos.y
            );

            if (wall_type)
            {
                /* Check the axis of collision */
                if (hit->norm_dir == side_dir_x)
                {
                    hit->front_dist = (
                        floor_norm.x - norm_pos.x + (1.0f - step_dist.x) / 2.0f
                    );

                    if (cam_ray.x != 0.0f)
                        hit->front_dist /= cam_ray.x;
                }
                else if (hit->norm_dir == side_dir_y)
                {
                    hit->front_dist = (
                        floor_norm.y - norm_pos.y + (1.0f - step_dist.y) / 2.0f
                    );

                    if (cam_ray.y != 0.0f)
                        hit->front_dist /= cam_ray.y;
                }

                hit->tile_pos = tile_pos;

                hit->front_hit = (Fang_Vec2){
                    .x = camera->pos.x + (hit->front_dist * cam_ray.x)
                       * map->tile_size,
                    .y = camera->pos.y + (hit->front_dist * cam_ray.y)
                       * map->tile_size,
                };

                const Fang_Vec2 old_floor_norm = floor_norm;
                const Fang_Vec2 old_side_dist  = side_dist;

                /* Run an additional increment to calculate the back face hit */
                {
                    Fang_Direction dir = hit->norm_dir;

                    if (side_dist.x < side_dist.y)
                    {
                        floor_norm.x += step_dist.x;
                        side_dist.x  += delta_dist.x;
                        dir = side_dir_x;
                    }
                    else if (side_dist.x > side_dist.y)
                    {
                        floor_norm.y += step_dist.y;
                        side_dist.y  += delta_dist.y;
                        dir = side_dir_y;
                    }
                    else /* 0 case */
                    {
                        if (step_dist.x < step_dist.y)
                        {
                            floor_norm.x += step_dist.x;
                            side_dist.x  += delta_dist.x;
                            dir = side_dir_x;
                        }
                        else
                        {
                            floor_norm.y += step_dist.y;
                            side_dist.y  += delta_dist.y;
                            dir = side_dir_y;
                        }
                    }

                    /* Check the axis of collision */
                    if (dir == side_dir_x)
                    {
                        hit->back_dist = (
                            floor_norm.x - norm_pos.x + (1.0f - step_dist.x) / 2.0f
                        );

                        if (cam_ray.x != 0.0f)
                            hit->back_dist /= cam_ray.x;
                    }
                    else if (dir == side_dir_y)
                    {
                        hit->back_dist = (
                            floor_norm.y - norm_pos.y + (1.0f - step_dist.y) / 2.0f
                        );

                        if (cam_ray.y != 0.0f)
                            hit->back_dist /= cam_ray.y;
                    }

                    hit->back_hit = (Fang_Vec2){
                        .x = camera->pos.x + (hit->back_dist * cam_ray.x)
                           * map->tile_size,
                        .y = camera->pos.y + (hit->back_dist * cam_ray.y)
                           * map->tile_size,
                    };
                }

                floor_norm = old_floor_norm;
                side_dist  = old_side_dist;

                hit_count++;
            }
        }

        rays[i].hit_count = hit_count;
    }
}
