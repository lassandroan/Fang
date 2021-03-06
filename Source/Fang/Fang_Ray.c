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

typedef struct Fang_RayHit {
    Fang_Tile * tile;
    Fang_Vec2   front_hit;
    float       front_dist;
    Fang_Vec2   back_hit;
    float       back_dist;
    Fang_Face   norm_dir;
} Fang_RayHit;

typedef struct Fang_Ray {
    Fang_RayHit hits[FANG_RAY_MAX_STEPS];
    size_t      hit_count;
} Fang_Ray;

static inline void
Fang_CastRays(
    const Fang_Camera * const camera,
    const Fang_Chunks * const chunks,
          Fang_Ray    * const rays,
    const size_t              ray_count)
{
    assert(camera);
    assert(chunks);
    assert(rays);
    assert(ray_count);

    const Fang_Vec3 * const dir = &camera->dir;
    const Fang_Vec2 * const cam = &camera->cam;
    const Fang_Vec2         pos = (Fang_Vec2){
        .x = camera->pos.x,
        .y = camera->pos.y
    };

    memset(rays, 0, sizeof(Fang_Ray) * ray_count);

    const Fang_Tile * const initial_tile = Fang_GetChunkTile(chunks, &pos);

    const bool standing_on_tile = (initial_tile)
        ? initial_tile->offset + initial_tile->height <= camera->pos.z
        : false;

    for (size_t i = 0; i < ray_count; ++i)
    {
        /* X coordinate in camera space, normalized -1.0f..1.0f */
        const float plane_x = (
            2.0f * (1.0f - (float)i / (float)ray_count) - 1.0f
        );

        /* Map ray start position onto camera plane */
        const Fang_Vec2 cam_ray = {
            .x = dir->x + cam->x * plane_x,
            .y = dir->y + cam->y * plane_x,
        };

        Fang_DDAState dda;
        Fang_InitDDA(&dda, &pos, &cam_ray);

        size_t hit_count = 0;

        /* Add initial hit if player is on top of a tile */
        if (standing_on_tile)
        {
            Fang_RayHit * const hit = &rays[i].hits[hit_count];

            const Fang_DDAState old_dda = dda;

            /* Front-face is not needed for rendering */
            hit->tile      = (Fang_Tile*)initial_tile;
            hit->back_dist = Fang_StepDDA(&dda);
            hit->back_hit  = (Fang_Vec2){
                .x = dda.pos.x - dda.start.x,
                .y = dda.pos.y - dda.start.y,
            };

            dda = old_dda;
            hit_count++;
        }

        for (size_t step = hit_count; step < FANG_RAY_MAX_STEPS; ++step)
        {
            Fang_RayHit * const hit = &rays[i].hits[hit_count];

            hit->front_dist = Fang_StepDDA(&dda);

            hit->tile = (Fang_Tile*)Fang_GetChunkTile(chunks, &dda.pos);

            if (hit->tile)
            {
                const Fang_DDAState old_dda = dda;

                hit->norm_dir  = dda.face;
                hit->front_hit = (Fang_Vec2){
                    .x = pos.x + (hit->front_dist * cam_ray.x),
                    .y = pos.y + (hit->front_dist * cam_ray.y),
                };

                hit->back_dist = Fang_StepDDA(&dda);
                hit->back_hit  = (Fang_Vec2){
                    .x = pos.x + (hit->back_dist * cam_ray.x),
                    .y = pos.y + (hit->back_dist * cam_ray.y),
                };

                dda = old_dda;
                hit_count++;
            }
        }

        rays[i].hit_count = hit_count;
    }
}
