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

static inline Fang_TileType
Fang_MapQueryType(
    int x,
    int y)
{
    static const Fang_TileType map[8][8] = {
        {1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 2, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 2, 0, 1},
        {1, 1, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1},
    };

    x /= FANG_TILE_SIZE;
    y /= FANG_TILE_SIZE;

    if (x < 0 || x >= 8)
        return FANG_TILETYPE_NONE;

    if (y < 0 || y >= 8)
        return FANG_TILETYPE_NONE;

    return map[y][x];
}

static inline Fang_TileSize
Fang_MapQuerySize(
    const int x,
    const int y)
{
    const Fang_TileType type = Fang_MapQueryType(x, y);

    switch (type)
    {
        case FANG_TILETYPE_SOLID:
            return (Fang_TileSize){0, FANG_TILE_SIZE};

        case FANG_TILETYPE_FLOATING:
            return (Fang_TileSize){FANG_TILE_SIZE * 2, FANG_TILE_SIZE};

        default:
            return (Fang_TileSize){0, 0};
    }
}

static inline Fang_Color
Fang_MapQueryColor(
    const int x,
    const int y)
{
    const Fang_TileType type = Fang_MapQueryType(x, y);

    switch (type)
    {
        case FANG_TILETYPE_SOLID:
            return FANG_WHITE;

        case FANG_TILETYPE_FLOATING:
            return FANG_GREY;

        default:
            return FANG_TRANSPARENT;
    }
}

static void
Fang_MapRenderWalls(
          Fang_Framebuffer * const framebuf,
    const Fang_Camera      * const camera,
    const Fang_Ray         * const rays,
    const size_t                   count)
{
    assert(framebuf);
    assert(camera);
    assert(rays);
    assert(count);

    for (size_t i = 0; i < count; ++i)
    {
        const Fang_Ray * const ray = &rays[i];

        for (size_t j = ray->hit_count; j-- > 0;)
        {
            const Fang_RayHit * const hit = &ray->hits[j];

            if (hit->front_dist <= 0.0f)
                continue;

            if (!Fang_MapQueryType(hit->tile_pos.x, hit->tile_pos.y))
                continue;

            const Fang_TileSize wall_size = Fang_MapQuerySize(
                hit->tile_pos.x, hit->tile_pos.y
            );

            Fang_Rect front_face;
            Fang_Rect  back_face;

            /* Calculate and draw front and back faces of tile */
            for (size_t k = 0; k < 2; ++k)
            {
                const float face_dist = (k == 0)
                    ? hit->front_dist
                    : hit->back_dist;

                const Fang_Rect surface = Fang_CameraProjectSurface(
                    camera,
                    &(Fang_Rect){
                        .x = (int)i,
                        .y = wall_size.height,
                        .w = 1,
                        .h = wall_size.size,
                    },
                    face_dist,
                    &(Fang_Rect){0, 0, FANG_WINDOW_SIZE, FANG_WINDOW_SIZE}
                );

                if (k == 0)
                    front_face = surface;
                else
                    back_face = surface;

                /* NOTE: Cull backfaces until transparent texture support */
                if (k == 1)
                    continue;

                const Fang_Color color = Fang_MapQueryColor(
                    hit->tile_pos.x, hit->tile_pos.y
                );

                Fang_FillRect(framebuf, &surface, &color);
            }

            /* Draw top or bottom of tile based on front/back faces */
            {
                Fang_Rect face = {.x = 0};

                /* Draw top */
                if (wall_size.height + wall_size.size < camera->pos.z)
                {
                    const int face_diff = front_face.y - back_face.y;

                    face = (Fang_Rect){
                        .x = (int)i,
                        .y = back_face.y,
                        .w = 1,
                        .h = face_diff,
                    };
                }
                /* Draw bottom */
                else if (wall_size.height > camera->pos.z)
                {
                    const int face_diff = ( back_face.y +  back_face.h)
                                        - (front_face.y + front_face.h);

                    face = (Fang_Rect){
                        .x = (int)i,
                        .y = front_face.y + front_face.h,
                        .w = 1,
                        .h = face_diff,
                    };
                }

                if (face.h)
                {
                    Fang_Color color = Fang_MapQueryColor(
                        hit->tile_pos.x, hit->tile_pos.y
                    );

                    color.r /= 2;
                    color.g /= 2;
                    color.b /= 2;

                    Fang_FillRect(framebuf, &face, &color);
                }
            }
        }
    }
}

static void
Fang_MapRender(
          Fang_Framebuffer * const framebuf,
    const Fang_Camera      * const camera,
    const Fang_Ray         * const rays,
    const size_t                   count)
{
    assert(framebuf);
    assert(camera);
    assert(rays);
    assert(count);

    Fang_MapRenderWalls(framebuf, camera, rays, count);
}

static void
Fang_MinimapRender(
          Fang_Framebuffer * const framebuf,
    const Fang_Camera      * const camera,
    const Fang_Ray         * const rays,
    const size_t                   count,
    const Fang_Rect        * const area)
{
    assert(framebuf);
    assert(camera);
    assert(rays);
    assert(count);

    const Fang_Mat3x3 transform_prev = framebuf->transform;

    const Fang_Rect bounds = Fang_FramebufferGetViewport(framebuf);

    Fang_FramebufferSetViewport(framebuf, area);

    Fang_FillRect(framebuf, &bounds, &FANG_BLACK);

    for (int row = 0; row < 8; ++row)
    {
        const float rowf = row / 8.0f;

        Fang_DrawHorizontalLine(
            framebuf, (int)(rowf * framebuf->color.height), &FANG_GREY
        );

        for (int col = 0; col < 8; ++col)
        {
            const float colf = col / 8.0f;

            Fang_DrawVerticalLine(
                framebuf, (int)(colf * framebuf->color.width), &FANG_GREY
            );

            const int x = row * FANG_TILE_SIZE;
            const int y = col * FANG_TILE_SIZE;

            if (!Fang_MapQueryType(x, y))
                continue;

            const Fang_Color color = Fang_MapQueryColor(x, y);

            Fang_Rect map_tile_bounds = Fang_RectResize(
                &(Fang_Rect){
                    .x = (int)(rowf * bounds.w),
                    .y = (int)(colf * bounds.h),
                    .w = bounds.w / 8,
                    .h = bounds.h / 8,
                },
                -2,
                -2
            );

            Fang_FillRect(framebuf, &map_tile_bounds, &color);
        }
    }

    const Fang_Point camera_pos = {
        .x = (int)(
            camera->pos.x / (FANG_TILE_SIZE * 8.0f) * bounds.w
        ),
        .y = (int)(
            camera->pos.y / (FANG_TILE_SIZE * 8.0f) * bounds.h
        ),
    };

    for (size_t i = 0; i < count; ++i)
    {
        const Fang_Ray * const ray = &rays[i];

        const Fang_Vec2 ray_pos = ray->hits[ray->hit_count - 1].front_hit;

        Fang_DrawLine(
            framebuf,
            &camera_pos,
            &(Fang_Point){
                .x = (int)(
                    ray_pos.x / (FANG_TILE_SIZE * 8.0f) * bounds.w
                ),
                .y = (int)(
                    ray_pos.y / (FANG_TILE_SIZE * 8.0f) * bounds.h
                ),
            },
            &FANG_BLUE
        );
    }

    Fang_FillRect(
        framebuf,
        &(Fang_Rect){
            .x = camera_pos.x - 5,
            .y = camera_pos.y - 5,
            .w = 10,
            .h = 10,
        },
        &FANG_RED
    );

    framebuf->transform = transform_prev;
}
