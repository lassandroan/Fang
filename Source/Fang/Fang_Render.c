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
    assert(framebuf->color.stride == 4);
    assert(image);

    /* If the image is invalid we will supply a default size for
       Fang_ImageQuery() to use in creating the 'XOR Texture'.

       Note that any smaller height value will cause the XOR texture to warp on
       walls.
    */
    const Fang_Rect image_area = (image->pixels)
        ? (Fang_Rect){.w = image->width, .h = image->height}
        : (Fang_Rect){.w = 0, .h = FANG_FACE_SIZE};

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
    const Fang_Image       * const font,
    const int                      fontheight,
    const Fang_Point       * const origin)
{
    assert(framebuf);
    assert(text);
    assert(font);

    Fang_Point position = (origin) ? *origin : (Fang_Point){0, 0};

    const float ratio = (float)fontheight / (float)FANG_FONT_HEIGHT;

    while (*text)
    {
        char character = *text;

        if (character == ' ')
        {
            text++;
            position.x += (int)((FANG_FONT_WIDTH + 1) * ratio);
            continue;
        }

        if (character < '!')
            character = '?';

        Fang_Rect target_area = {0, 0, 0, 0};
        {
            const float pos = (character - (float)'!') / (127.0f - (float)'!');

            const int total_width = (127 - '!') * (FANG_FONT_WIDTH + 1);

            target_area = (Fang_Rect){
                .x = (int)(total_width * pos) + 1,
                .y = 0,
                .w = FANG_FONT_WIDTH,
                .h = FANG_FONT_HEIGHT,
            };
        }

        Fang_DrawImage(
            framebuf,
            font,
            &target_area,
            &(Fang_Rect){
                .x = position.x,
                .y = position.y,
                .w = (int)(target_area.w * ratio),
                .h = (int)(target_area.h * ratio),
            }
        );

        text++;
        position.x += (int)((FANG_FONT_WIDTH + 1) * ratio);
    }
}

/**
 * Draws the skybox of a given map, translated based on the camera's rotation.
**/
static void
Fang_DrawMapSkybox(
          Fang_Framebuffer * const framebuf,
          Fang_Map         * const map,
    const Fang_Camera      * const camera)
{
    assert(framebuf);
    assert(map);
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
 * Draws the floor of a given map, translated based on the camera's position
 * and rotation.
**/
static void
Fang_DrawMapFloor(
          Fang_Framebuffer * const framebuf,
          Fang_Map         * const map,
    const Fang_Camera      * const camera)
{
    assert(framebuf);
    assert(map);
    assert(camera);

    assert(map->floor.pixels);
    assert(map->floor.stride == 4);

    const Fang_Rect viewport = Fang_FramebufferGetViewport(framebuf);

    if (camera->pos.z <= viewport.h / -2.0f)
        return;

    /* Calculate vertical offset in screen space */
    const float pitch  = camera->cam.z * viewport.h;
    const float height = camera->pos.z * FANG_PROJECTION_RATIO;
    const int   offset = (int)(pitch + height);

    if (height <= 0.0f)
        return;

    const Fang_Vec2 ray_start = {
        .x = camera->dir.x + camera->cam.x,
        .y = camera->dir.y + camera->cam.y,
    };

    const Fang_Vec2 ray_end = {
        .x = camera->dir.x - camera->cam.x,
        .y = camera->dir.y - camera->cam.y,
    };

    for (int y = viewport.h / 2 + offset; y < viewport.h; ++y)
    {
        if (y < 0)
            continue;

        /* Vertical position in screen space shifted by our height/pitch */
        const int p = (int)(y - (viewport.h / 2) - offset);

        if (!p)
            continue;

        /* Calculate row distance, based on perspective at our given height */
        float row_dist = ((viewport.h / 2.0f) / p) / (1.0f / height);

        framebuf->state.current_depth = row_dist
                                      * height
                                      * (1.0f / FANG_PROJECTION_RATIO);

        const Fang_Vec2 floor_step = {
            .x = row_dist * (ray_end.x - ray_start.x) / viewport.w,
            .y = row_dist * (ray_end.y - ray_start.y) / viewport.w,
        };

        Fang_Vec2 floor_pos = {
            .x = (camera->pos.x / 2.0f) + row_dist * ray_start.x,
            .y = (camera->pos.y / 2.0f) + row_dist * ray_start.y,
        };

        for (int x = 0; x < viewport.w; ++x)
        {
            Fang_Point tex_pos = {
                .x = (int)roundf(
                    map->floor.width * (floor_pos.x - (int)floor_pos.x)
                ),
                .y = (int)roundf(
                    map->floor.height * (floor_pos.y - (int)floor_pos.y)
                ),
            };

            tex_pos.x &= (map->floor.width  - 1);
            tex_pos.y &= (map->floor.height - 1);

            floor_pos.x += floor_step.x;
            floor_pos.y += floor_step.y;

            Fang_Color dest_color = Fang_ImageQuery(&map->floor, &tex_pos);

            /* Shortening the floor fog seems to align closer to tile fog */
            dest_color = Fang_ColorBlend(
                &(Fang_Color){
                    .r = map->fog.r,
                    .g = map->fog.g,
                    .b = map->fog.b,
                    .a = (uint8_t)(
                        clamp(
                            (row_dist * 2.0f) / map->fog_distance,
                            0.0f,
                            1.0f
                        ) * 255.0f
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
          Fang_Framebuffer * const framebuf,
          Fang_Map         * const map,
    const Fang_Camera      * const camera,
    const Fang_Ray         * const rays,
    const size_t                   count)
{
    assert(framebuf);
    assert(map);
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

            const Fang_Tile tile = Fang_MapQuerySize(
                map, hit->tile_pos.x, hit->tile_pos.y
            );

            const Fang_Image * const wall_tex = Fang_MapQueryTexture(
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

                const Fang_Tile projected_tile = Fang_CameraProjectTile(
                    camera, &tile, face_dist, &viewport
                );

                const Fang_Rect dest_rect = {
                    .x = (int)i,
                    .y = projected_tile.y,
                    .w = 1,
                    .h = projected_tile.h,
                };

                if (k == 0)
                    front_face = dest_rect;
                else
                    back_face  = dest_rect;

                if (dest_rect.y >= viewport.h)
                    continue;

                if (dest_rect.y + dest_rect.h <= 0)
                    continue;

                /* NOTE: Cull backfaces until transparent texture support */
                if (k == 1)
                    continue;

                const Fang_Vec2 face_hit = (k == 0)
                    ? hit->front_hit
                    : hit->back_hit;

                const Fang_Face face = hit->norm_dir;

                float tex_x =
                    (face == FANG_FACE_NORTH || face == FANG_FACE_SOUTH)
                        ? fmodf(face_hit.x, 1.0f)
                        : fmodf(face_hit.y, 1.0f);

                tex_x = clamp(tex_x, 0.0f, 1.0f);

                if (face == FANG_FACE_EAST || face == FANG_FACE_NORTH)
                    tex_x = 1.0f - tex_x;

                framebuf->state.current_depth = face_dist;

                Fang_DrawImage(
                    framebuf,
                    wall_tex,
                    &(Fang_Rect){
                        .x = (int)floorf(tex_x * (FANG_FACE_SIZE - 1))
                           + (int)      (face  * (FANG_FACE_SIZE - 1)),
                        .y = 0,
                        .w = 1,
                        .h = FANG_FACE_SIZE,
                    },
                    &dest_rect
                );

                Fang_FillRect(
                    framebuf,
                    &dest_rect,
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

                if (start_y <= 0 && end_y <= 0)
                    continue;

                if (start_y >= viewport.h)
                    continue;

                for (int y = start_y; y < end_y; ++y)
                {
                    const float r_y = (float)(y - start_y)
                                    / (float)(end_y - start_y);

                    if (y < 0 || y >= viewport.h)
                        continue;

                    Fang_Point tex_pos;
                    {
                        float u = ((1.0f - r_y) * hit_start.x)
                                + (r_y * hit_end.x);

                        float v = ((1.0f - r_y) * hit_start.y)
                                + (r_y * hit_end.y);

                        float integral;
                        u = clamp(modff(u, &integral), 0.0f, 1.0f);
                        v = clamp(modff(v, &integral), 0.0f, 1.0f);

                        if (y == start_y)
                            u = 1.0f;

                        tex_pos.x = (int)(u * (FANG_FACE_SIZE  - 1));
                        tex_pos.y = (int)(v * (FANG_FACE_SIZE - 1));
                    }

                    tex_pos.x += (int)face * FANG_FACE_SIZE;

                    Fang_Color dest_color = Fang_ImageQuery(
                        wall_tex, &tex_pos
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
          Fang_Framebuffer * const framebuf,
          Fang_Map         * const map,
    const Fang_Camera      * const camera,
    const Fang_Ray         * const rays,
    const size_t                   count)
{
    assert(framebuf);
    assert(map);
    assert(camera);
    assert(rays);
    assert(count);

    framebuf->state.current_depth = FLT_MAX;

    if (map->skybox.pixels)
        Fang_DrawMapSkybox(framebuf, map, camera);

    if (map->floor.pixels)
    {
        const Fang_FrameState state = framebuf->state;
        framebuf->state.enable_depth = false;
        Fang_DrawMapFloor(framebuf, map, camera);
        framebuf->state = state;
    }

    Fang_DrawMapTiles(framebuf, map, camera, rays, count);
}

/**
 * Draws a 2D representation of the map.
 *
 * Depth buffering is disabled during this function.
**/
static void
Fang_DrawMinimap(
          Fang_Framebuffer * const framebuf,
    const Fang_Map         * const map,
    const Fang_Camera      * const camera,
    const Fang_Ray         * const rays,
    const size_t                   count)
{
    assert(framebuf);
    assert(map);
    assert(camera);
    assert(rays);
    assert(count);

    const Fang_Rect bounds = Fang_FramebufferGetViewport(framebuf);

    Fang_FillRect(framebuf, &bounds, &FANG_BLACK);

    for (int row = 0; row < map->height; ++row)
    {
        const float rowf = (float)row / (float)map->height;

        Fang_DrawHorizontalLine(
            framebuf, (int)(rowf * framebuf->color.height), &FANG_GREY
        );

        for (int col = 0; col < map->width; ++col)
        {
            const float colf = col / (float)map->width;

            Fang_DrawVerticalLine(
                framebuf, (int)(colf * framebuf->color.width), &FANG_GREY
            );

            if (!Fang_MapQueryType(map, row, col))
                continue;

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

            Fang_FillRect(framebuf, &map_tile_bounds, &FANG_WHITE);
        }
    }

    const Fang_Point camera_pos = {
        .x = (int)((camera->pos.x / map->width) * bounds.w),
        .y = (int)((camera->pos.y / map->height) * bounds.h),
    };

    for (size_t i = 0; i < count; ++i)
    {
        const Fang_Ray * const ray = &rays[i];

        if (!ray->hit_count)
            continue;

        const Fang_Vec2 ray_pos = ray->hits[ray->hit_count - 1].front_hit;

        Fang_DrawLine(
            framebuf,
            &camera_pos,
            &(Fang_Point){
                .x = (int)((ray_pos.x / map->width) * bounds.w),
                .y = (int)((ray_pos.y / map->height) * bounds.h),
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
}


static void
Fang_DrawEntities(
          Fang_Framebuffer * const framebuf,
          Fang_Map         * const map,
    const Fang_Camera      * const camera,
    const Fang_Entity      * const entities,
    const size_t                   count)
{
    assert(framebuf);
    assert(map);
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

        if (surface.x + surface.w <= 0 || surface.x >= viewport.w)
            continue;

        if (surface.y + surface.h <= 0 || surface.y >= viewport.h)
            continue;

        Fang_FillRect(
            framebuf,
            &surface,
            &FANG_PURPLE
        );
    }
}
