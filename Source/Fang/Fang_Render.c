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
 * Draws a line in the framebuffer using Bresenham's Algorithm.
 *
 * The target framebuffer must have a valid color image.
**/
static void
Fang_DrawLine(
          Fang_Framebuffer * const framebuf,
    const Fang_Point       * const start,
    const Fang_Point       * const end,
    const Fang_Color       * const color)
{
    assert(framebuf);
    assert(start);
    assert(end);
    assert(color);

    const Fang_Point delta = {
        .x =  abs(end->x - start->x),
        .y = -abs(end->y - start->y),
    };

    const Fang_Point step = {
        .x = (start->x < end->x) ? 1 : (start->x > end->x) ? -1 : 0,
        .y = (start->y < end->y) ? 1 : (start->y > end->y) ? -1 : 0,
    };

    Fang_Point point = *start;
    int        err   = delta.x + delta.y;

    while (true)
    {
        Fang_FramebufferPutPixel(framebuf, &point, color);

        if ((err * 2) >= delta.y)
        {
            if (point.x == end->x)
                break;

            err     += delta.y;
            point.x += step.x;
        }

        if ((err * 2) <= delta.x)
        {
            if (point.y == end->y)
                break;

            err     += delta.x;
            point.y += step.y;
        }
    }
}

/**
 * Draws a vertical line across the framebuffer.
**/
static inline void
Fang_DrawVerticalLine(
          Fang_Framebuffer * const framebuf,
    const int                      x,
    const Fang_Color       * const color)
{
    assert(framebuf);
    assert(color);

    for (int y = 0; y < framebuf->color.height; ++y)
        Fang_FramebufferPutPixel(framebuf, &(Fang_Point){x, y}, color);
}

/**
 * Draws a horizontal line across the framebuffer.
**/
static inline void
Fang_DrawHorizontalLine(
          Fang_Framebuffer * const framebuf,
    const int                      y,
    const Fang_Color       * const color)
{
    assert(framebuf);
    assert(color);

    for (int x = 0; x < framebuf->color.width; ++x)
        Fang_FramebufferPutPixel(framebuf, &(Fang_Point){x, y}, color);
}

/**
 * Draws a 1px thick outline of a rectangle in the framebuffer.
 *
 * The target framebuffer must have a valid color image.
**/
static void
Fang_DrawRect(
          Fang_Framebuffer * const framebuf,
    const Fang_Rect        * const rect,
    const Fang_Color       * const color)
{
    assert(framebuf);
    assert(rect);
    assert(color);

    for (int h = 0; h < rect->h; h += rect->h - 1)
    {
        for (int w = 0; w < rect->w; ++w)
        {
            Fang_FramebufferPutPixel(
                framebuf, &(Fang_Point){rect->x + w, rect->y + h}, color
            );
        }
    }

    for (int h = 0; h < rect->h; ++h)
    {
        for (int w = 0; w < rect->w; w += rect->w - 1)
        {
            Fang_FramebufferPutPixel(
                framebuf, &(Fang_Point){rect->x + w, rect->y + h}, color
            );
        }
    }
}

/**
 * Draws a solid rectangle in the framebuffer.
 *
 * The target framebuffer must have a valid color image.
**/
static void
Fang_FillRect(
          Fang_Framebuffer * const framebuf,
    const Fang_Rect        * const rect,
    const Fang_Color       * const color)
{
    assert(framebuf);
    assert(rect);
    assert(color);

    for (int h = 0; h < rect->h; ++h)
    {
        for (int w = 0; w < rect->w; ++w)
        {
            Fang_FramebufferPutPixel(
                framebuf, &(Fang_Point){rect->x + w, rect->y + h}, color
            );
        }
    }
}

/**
 * Draws an image (or subsection) to the given area in the framebuffer.
 *
 * If the source is NULL, the image size is used with an origin of 0, 0. If the
 * destination is NULL, the framebuffer color image size is used with an origin
 * of 0, 0.
 *
 * If the sizes of the source and destination rectangles do not match, the image
 * will be scaled to fit the destination rectangle. This scaling is linear, no
 * resampling is performed.
**/
static void
Fang_DrawImage(
          Fang_Framebuffer * const framebuf,
    const Fang_Image       * const image,
    const Fang_Rect        * const source,
    const Fang_Rect        * const dest)
{
    assert(framebuf);
    assert(image);
    assert(image->pixels);

    assert(
        image->stride == 1
     || image->stride == 3
     || image->stride == 4
    );

    assert(framebuf->color.stride == 4);

    Fang_Rect source_area = (source)
        ? *source
        : (Fang_Rect){.w = image->width, .h = image->height};

    source_area = Fang_RectClip(
        &source_area,
        &(Fang_Rect){
            .w = image->width,
            .h = image->height,
        }
    );

    Fang_Rect dest_area = (dest)
        ? *dest
        : (Fang_Rect){.w = framebuf->color.width, .h = framebuf->color.height};

    for (int x = dest_area.x; x < dest_area.x + dest_area.w; ++x)
    {
        if (x < 0 || x >= framebuf->color.width)
            continue;

        for (int y = dest_area.y; y < dest_area.y + dest_area.h; ++y)
        {
            if (y < 0 || y >= framebuf->color.height)
                continue;

            float r_x = (float)(x - dest_area.x)
                      / (float)(dest_area.x + dest_area.w - dest_area.x);

            float r_y = (float)(y - dest_area.y)
                      / (float)(dest_area.y + dest_area.h - dest_area.y);

            r_x = max(min(r_x, 1.0f), 0.0f);
            r_y = max(min(r_y, 1.0f), 0.0f);

            const int t_x = (int)((r_x * source_area.w) + source_area.x);
            const int t_y = (int)((r_y * source_area.h) + source_area.y);

            uint32_t pixel = 0;

            for (int p = 0; p < image->stride; ++p)
            {
                pixel |= *(
                    image->pixels + p
                  + (t_x * image->stride)
                  + (t_y * image->pitch)
                );

                if (p < image->stride - 1)
                    pixel <<= 8;
            }

            for (int p = image->stride; p < 4; ++p)
            {
                pixel <<= 8;
                pixel |= 0x000000FF;
            }

            const Fang_Color dest_color = Fang_ColorFromRGBA(pixel);

            Fang_FramebufferPutPixel(
                framebuf,
                &(Fang_Point){x, y},
                &dest_color
            );
        }
    }
}


/**
 * Draws a line of text into the framebuffer using the given font type.
**/
static void
Fang_DrawText(
          Fang_Framebuffer * const framebuf,
    const char             *       text,
    const Fang_FontType            type,
    const int                      fontheight,
    const Fang_Point       * const origin)
{
    assert(framebuf);
    assert(text);

    Fang_Point position = (origin) ? *origin : (Fang_Point){0, 0};

    const float ratio = (float)fontheight / (float)FANG_FONTAREA_HEIGHT;

    while (*text)
    {
        if (*text == ' ')
        {
            text++;
            position.x += (int)((FANG_FONTAREA_WIDTH + 1) * ratio);
            continue;
        }

        const Fang_Rect character = Fang_FontGetCharPosition(*text);

        Fang_DrawImage(
            framebuf,
            Fang_FontGet(type),
            &character,
            &(Fang_Rect){
                .x = position.x,
                .y = position.y,
                .w = (int)(character.w * ratio),
                .h = (int)(character.h * ratio),
            }
        );

        text++;
        position.x += (int)((FANG_FONTAREA_WIDTH + 1) * ratio);
    }
}


static void
Fang_DrawMapSkybox(
          Fang_Map         * const map,
          Fang_Framebuffer * const framebuf,
    const Fang_Camera      * const camera)
{
    assert(map);
    assert(framebuf);
    assert(camera);

    const Fang_Rect viewport = Fang_FramebufferGetViewport(framebuf);

    const int pitch = (int)roundf(camera->cam.z * viewport.h);

    const float angle = Fang_Vec2Angle(
        *(Fang_Vec2*)(&camera->dir),
        (Fang_Vec2){.x = 0.0f, .y = -1.0f}
    );

    const float ratio = -(angle / ((float)M_PI / 2.0f)) * 2.0f;

    const Fang_Rect dest = {
        .x = (int)(viewport.w * ratio),
        .y = 0,
        .w = viewport.w * 4,
        .h = viewport.h / 2 + pitch,
    };

    Fang_DrawImage(
        framebuf,
        &map->skybox,
        NULL,
        &dest
    );
}

static void
Fang_DrawMapFloor(
          Fang_Map         * const map,
          Fang_Framebuffer * const framebuf,
    const Fang_Camera      * const camera)
{
    assert(map);
    assert(framebuf);
    assert(camera);

    assert(map->floor.pixels);
    assert(map->floor.stride == 4);

    const Fang_Rect viewport = Fang_FramebufferGetViewport(framebuf);

    if (camera->pos.z <= viewport.h / -2.0f)
        return;

    const int pitch = (int)roundf(camera->cam.z * viewport.h);

    const Fang_Vec2 ray_start = {
        .x = camera->dir.x + camera->cam.x,
        .y = camera->dir.y + camera->cam.y,
    };

    const Fang_Vec2 ray_end = {
        .x = camera->dir.x - camera->cam.x,
        .y = camera->dir.y - camera->cam.y,
    };

    for (int y = viewport.h / 2 + pitch; y < viewport.h; ++y)
    {
        if (y < 0)
            continue;

        const int p = y - viewport.h / 2 - pitch;

        if (!p)
            continue;

        const float row_dist = ((viewport.h / 2) + camera->pos.z) / p;

        const Fang_Vec2 floor_step = {
            .x = row_dist * (ray_end.x - ray_start.x) / viewport.w,
            .y = row_dist * (ray_end.y - ray_start.y) / viewport.w,
        };

        Fang_Vec2 floor_pos = {
            .x = (camera->pos.x / map->tile_size) + row_dist * ray_start.x,
            .y = (camera->pos.y / map->tile_size) + row_dist * ray_start.y,
        };

        for (int x = 0; x < viewport.w; ++x)
        {
            const Fang_Point texture_pos = {
                .x = (int)(map->floor.width  * (floor_pos.x - (int)floor_pos.x))
                   & (map->floor.width  - 1),
                .y = (int)(map->floor.height * (floor_pos.y - (int)floor_pos.y))
                   & (map->floor.height - 1),
            };

            floor_pos.x += floor_step.x;
            floor_pos.y += floor_step.y;

            const uint32_t pixel = *(uint32_t*)&map->floor.pixels[
                (map->floor.pitch * texture_pos.y)
              + (texture_pos.x * map->floor.stride)
            ];

            const Fang_Color color = Fang_ColorFromRGBA(pixel);

            Fang_FramebufferPutPixel(
                framebuf,
                &(Fang_Point){x, y},
                &color
            );
        }
    }
}

static void
Fang_DrawMapTiles(
          Fang_Map         * const map,
          Fang_Framebuffer * const framebuf,
    const Fang_Camera      * const camera,
    const Fang_Ray         * const rays,
    const size_t                   count)
{
    assert(map);
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

            if (!Fang_MapQueryType(map, hit->tile_pos.x, hit->tile_pos.y))
                continue;

            const Fang_TileSize wall_size = Fang_MapQuerySize(
                map, hit->tile_pos.x, hit->tile_pos.y
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
                    map->tile_size,
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
                    map, hit->tile_pos.x, hit->tile_pos.y
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
                        map, hit->tile_pos.x, hit->tile_pos.y
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
Fang_DrawMap(
          Fang_Map         * const map,
          Fang_Framebuffer * const framebuf,
    const Fang_Camera      * const camera,
    const Fang_Ray         * const rays,
    const size_t                   count)
{
    assert(map);
    assert(framebuf);
    assert(camera);
    assert(rays);
    assert(count);

    if (map->skybox.pixels)
        Fang_DrawMapSkybox(map, framebuf, camera);

    if (map->floor.pixels)
        Fang_DrawMapFloor(map, framebuf, camera);

    Fang_DrawMapTiles(map, framebuf, camera, rays, count);
}

static void
Fang_DrawMinimap(
    const Fang_Map         * const map,
          Fang_Framebuffer * const framebuf,
    const Fang_Camera      * const camera,
    const Fang_Ray         * const rays,
    const size_t                   count,
    const Fang_Rect        * const area)
{
    assert(map);
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

            const int x = row * map->tile_size;
            const int y = col * map->tile_size;

            if (!Fang_MapQueryType(map, x, y))
                continue;

            const Fang_Color color = Fang_MapQueryColor(map, x, y);

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
            camera->pos.x / (float)(map->tile_size * map->width) * bounds.w
        ),
        .y = (int)(
            camera->pos.y / (float)(map->tile_size * map->height) * bounds.h
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
                    ray_pos.x / (float)(map->tile_size * map->width) * bounds.w
                ),
                .y = (int)(
                    ray_pos.y / (float)(map->tile_size * map->height) * bounds.h
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
