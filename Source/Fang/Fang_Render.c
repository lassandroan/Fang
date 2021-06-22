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
 *
 * The source image may be flipped in the X or Y direction when being drawn.
**/
static void
Fang_DrawImageEx(
          Fang_Framebuffer * const framebuf,
    const Fang_Image       * const image,
    const Fang_Rect        * const source,
    const Fang_Rect        * const dest,
    const bool                     flip_x,
    const bool                     flip_y)
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

    const Fang_Rect image_area = {.w = image->width, .h = image->height};

    const Fang_Rect source_area = (source)
        ? Fang_RectClip(source, &image_area)
        : image_area;

    const Fang_Rect framebuf_area = {
        .w = framebuf->color.width,
        .h = framebuf->color.height,
    };

    const Fang_Rect dest_area = (dest)
        ? *dest
        : framebuf_area;

    for (int x = dest_area.x; x < dest_area.x + dest_area.w; ++x)
    {
        if (x < 0 || x >= framebuf->color.width)
            continue;

        for (int y = dest_area.y; y < dest_area.y + dest_area.h; ++y)
        {
            if (y < 0 || y >= framebuf->color.height)
                continue;

            float r_x = (float)(x - dest_area.x) / (float)dest_area.w;
            float r_y = (float)(y - dest_area.y) / (float)dest_area.h;

            r_x = max(min(r_x, 1.0f), 0.0f);
            r_y = max(min(r_y, 1.0f), 0.0f);

            if (flip_x)
                r_x = 1.0f - r_x;

            if (flip_y)
                r_y = 1.0f - r_y;

            const Fang_Point tex_pos = {
                .x = (flip_x)
                    ? (int)(r_x * (source_area.w - 1)) + source_area.x
                    : (int)(r_x * (source_area.w - 0)) + source_area.x,
                .y = (flip_y)
                    ? (int)(r_y * (source_area.h - 1)) + source_area.y
                    : (int)(r_y * (source_area.h - 0)) + source_area.y,
            };

            const Fang_Color dest_color = Fang_ImageQuery(image, &tex_pos);

            Fang_FramebufferPutPixel(
                framebuf,
                &(Fang_Point){x, y},
                &dest_color
            );
        }
    }
}

/**
 * Shortcut function for Fang_DrawImageEx() without flipping the source image.
**/
static inline void
Fang_DrawImage(
          Fang_Framebuffer * const framebuf,
    const Fang_Image       * const image,
    const Fang_Rect        * const source,
    const Fang_Rect        * const dest)
{
    Fang_DrawImageEx(framebuf, image, source, dest, false, false);
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

/**
 * Draws the skybox of a given map, translated based on the camera's rotation.
**/
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

    for (int i = 1; i >= -1; i -= 2)
    {
        Fang_DrawImageEx(
            framebuf,
            &map->skybox,
            NULL,
            &(Fang_Rect){
                .x = dest.x + (dest.w * i),
                .y = dest.y,
                .w = dest.w,
                .h = dest.h,
            },
            true,
            false
        );
    }

    Fang_DrawImage(
        framebuf,
        &map->skybox,
        NULL,
        &dest
    );
}

/**
 * Draws the skybox of a given map, translated based on the camera's position
 * and rotation.
**/
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
            const Fang_Point tex_pos = {
                .x = (int)(map->floor.width  * (floor_pos.x - (int)floor_pos.x))
                   & (map->floor.width  - 1),
                .y = (int)(map->floor.height * (floor_pos.y - (int)floor_pos.y))
                   & (map->floor.height - 1),
            };

            floor_pos.x += floor_step.x;
            floor_pos.y += floor_step.y;

            const uint32_t pixel = *(uint32_t*)&map->floor.pixels[
                (map->floor.pitch * tex_pos.y)
              + (tex_pos.x * map->floor.stride)
            ];

            Fang_Color dest_color = Fang_ColorFromRGBA(pixel);

            dest_color = Fang_ColorBlend(
                &(Fang_Color){
                    .r = map->fog.r,
                    .g = map->fog.g,
                    .b = map->fog.b,
                    .a = (uint8_t)(
                        clamp(row_dist / map->fog_distance, 0.0f, 1.0f)
                        * 255.0f
                    ),
                },
                &dest_color
            );

            Fang_FramebufferPutPixel(
                framebuf,
                &(Fang_Point){x, y},
                &dest_color
            );
        }
    }
}

/**
 * Draws the results of a raycast against map tiles.
 *
 * The provided camera should be the starting point of the rays, and is used to
 * project the viewable map tiles into the framebuffer viewport.
**/
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

    const Fang_Rect viewport = Fang_FramebufferGetViewport(framebuf);

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

            const Fang_Image wall_tex = Fang_TileTextures[0];

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
                    &viewport
                );

                if (k == 0)
                    front_face = surface;
                else
                    back_face  = surface;

                /* NOTE: Cull backfaces until transparent texture support */
                if (k == 1)
                    continue;

                const Fang_Vec2 face_hit = (k == 0)
                    ? hit->front_hit
                    : hit->back_hit;

                const Fang_Face face = hit->norm_dir;

                float tex_x =
                    (face == FANG_FACE_NORTH || face == FANG_FACE_SOUTH)
                        ? fmodf(face_hit.x, map->tile_size) / map->tile_size
                        : fmodf(face_hit.y, map->tile_size) / map->tile_size;

                tex_x = clamp(tex_x, 0.0f, 1.0f);

                if (face == FANG_FACE_EAST || face == FANG_FACE_NORTH)
                    tex_x = 1.0f - tex_x;

                framebuf->state.current_depth = face_dist;

                Fang_DrawImage(
                    framebuf,
                    &wall_tex,
                    &(Fang_Rect){
                        .x = (int)floorf(tex_x * (FANG_TILEAREA_WIDTH - 1))
                           + ((int)face * (FANG_TILEAREA_WIDTH - 1)),
                        .y = 0,
                        .w = 1,
                        .h = FANG_TILEAREA_HEIGHT,
                    },
                    &surface
                );

                const bool enable_depth_prev = framebuf->state.enable_depth;
                framebuf->state.enable_depth = false;

                Fang_FillRect(
                    framebuf,
                    &surface,
                    &(Fang_Color){
                        .r = map->fog.r,
                        .g = map->fog.g,
                        .b = map->fog.b,
                        .a = (uint8_t)(
                            clamp(face_dist / map->fog_distance, 0.0f, 1.0f)
                            * 255.0f
                        ),
                    }
                );

                framebuf->state.enable_depth = enable_depth_prev;
            }

            /* Draw top or bottom of tile based on front/back faces */
            {
                int start_y,
                    end_y;

                Fang_Vec2 hit_start,
                          hit_end;

                float dist_start,
                      dist_end;

                Fang_Face face;

                /* Draw top */
                if (front_face.y > back_face.y)
                {
                    hit_start = hit->back_hit;
                    hit_end   = hit->front_hit;

                    dist_start = hit->back_dist;
                    dist_end   = hit->front_dist;

                    start_y = back_face.y;
                    end_y   = front_face.y;

                    face = FANG_FACE_TOP;
                }
                /* Draw bottom */
                else if (front_face.y + front_face.h
                     <=   back_face.y +  back_face.h)
                {
                    hit_start = hit->front_hit;
                    hit_end   = hit->back_hit;

                    dist_start = hit->front_dist;
                    dist_end   = hit->back_dist;

                    start_y = front_face.y + front_face.h;
                    end_y   = back_face.y + back_face.h;

                    face = FANG_FACE_BOTTOM;
                }
                else
                {
                    continue;
                }

                for (int y = start_y; y < end_y; ++y)
                {
                    const float r_y = (float)(y - start_y)
                                    / (float)(end_y - start_y);

                    Fang_Point tex_pos;
                    {
                        Fang_Vec2 norm_hit_start = Fang_Vec2Divf(
                            hit_start, map->tile_size
                        );

                        Fang_Vec2 norm_hit_end = Fang_Vec2Divf(
                            hit_end, map->tile_size
                        );

                        float u = ((1.0f - r_y) * norm_hit_start.x)
                                + (r_y * norm_hit_end.x);

                        float v = ((1.0f - r_y) * norm_hit_start.y)
                                + (r_y * norm_hit_end.y);

                        float integral;
                        u = clamp(modff(u, &integral), 0.0f, 1.0f);
                        v = clamp(modff(v, &integral), 0.0f, 1.0f);

                        if (y == start_y)
                            u = 1.0f;

                        tex_pos.x = (int)(u * (FANG_TILEAREA_WIDTH - 1));
                        tex_pos.y = (int)(v * (FANG_TILEAREA_HEIGHT - 1));
                    }

                    tex_pos.x += (int)face * FANG_TILEAREA_WIDTH;

                    Fang_Color dest_color = Fang_ImageQuery(
                        &wall_tex, &tex_pos
                    );

                    const float dist = ((1.0f - r_y) * dist_start)
                                     + (r_y * dist_end);

                    dest_color = Fang_ColorBlend(
                        &(Fang_Color){
                            .r = map->fog.r,
                            .g = map->fog.g,
                            .b = map->fog.b,
                            .a = (uint8_t)(
                                clamp(dist / map->fog_distance, 0.0f, 1.0f)
                                * 255.0f
                            ),
                        },
                        &dest_color
                    );

                    framebuf->state.current_depth = dist;

                    Fang_FramebufferPutPixel(
                        framebuf,
                        &(Fang_Point){
                            .x = (int)i,
                            .y = y,
                        },
                        &dest_color
                    );
                }
            }
        }
    }
}

/**
 * Renders a first-person view of the provided map.
 *
 * The map is rendered in order of:
 * - Skybox
 * - Floor
 * - Tiles
**/
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

    const bool enable_depth_prev = framebuf->state.enable_depth;
    framebuf->state.enable_depth = false;

    if (map->skybox.pixels)
        Fang_DrawMapSkybox(map, framebuf, camera);

    if (map->floor.pixels)
        Fang_DrawMapFloor(map, framebuf, camera);

    framebuf->state.enable_depth = enable_depth_prev;

    Fang_DrawMapTiles(map, framebuf, camera, rays, count);
}

/**
 * Draws a 2D representation of the map.
 *
 * Depth buffering is disabled during this function.
**/
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

    const Fang_Rect       bounds = Fang_FramebufferGetViewport(framebuf);
    const Fang_FrameState state  = Fang_FramebufferSetViewport(framebuf, area);

    framebuf->state.enable_depth = false;

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

    framebuf->state = state;
}


static void
Fang_DrawEntities(
          Fang_Framebuffer * const framebuf,
    const Fang_Camera      * const camera,
    const Fang_Entity      * const entities,
    const size_t                   count)
{
    assert(framebuf);
    assert(camera);
    assert(entities);

    const Fang_Rect viewport = Fang_FramebufferGetViewport(framebuf);

    for (size_t i = 0; i < count; ++i)
    {
        const Fang_Entity * const entity = &entities[i];

        const Fang_Rect surface = Fang_CameraProjectBody(
            camera,
            &entity->body,
            &viewport,
            &framebuf->state.current_depth
        );

        if (surface.h <= 0)
            continue;

        if (surface.x + surface.w <= 0 || surface.x > viewport.w)
            continue;

        if (surface.y + surface.h <= 0 || surface.y > viewport.h)
            continue;

        framebuf->state.current_depth /= 16.0f;

        Fang_FillRect(
            framebuf,
            &surface,
            &FANG_PURPLE
        );
    }
}
