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

typedef struct Fang_Camera {
    Fang_Vec3 pos;
    Fang_Vec3 dir;
    Fang_Vec3 cam;
} Fang_Camera;

static inline void
Fang_CameraRotate(
          Fang_Camera * const camera,
    const float               angle,
    const float               pitch)
{
    assert(camera);

    const Fang_Vec2 rotation = {
        .x = cosf(angle),
        .y = sinf(angle),
    };

    Fang_Vec3 * const dir = &camera->dir;
    Fang_Vec3 * const cam = &camera->cam;

    dir->x = dir->x * rotation.x - dir->y * rotation.y;
    dir->y = dir->x * rotation.y + dir->y * rotation.x;
    cam->x = cam->x * rotation.x - cam->y * rotation.y;
    cam->y = cam->x * rotation.y + cam->y * rotation.x;
    cam->z = clamp(cam->z + pitch, -1.0f, 1.0f);

    *(Fang_Vec2*)dir = Fang_Vec2Normalize(*(Fang_Vec2*)dir);
    *(Fang_Vec2*)cam = Fang_Vec2Normalize(*(Fang_Vec2*)cam);

    cam->x *= 0.5f;
    cam->y *= 0.5f;
}

static inline Fang_Rect
Fang_CameraProjectSurface(
    const Fang_Camera * const camera,
    const Fang_Rect   * const surface,
    const float               dist,
    const int                 tile_size,
    const Fang_Rect   * const viewport)
{
    assert(camera);
    assert(surface);
    assert(viewport);

    const float ratio  = viewport->h / dist;
    const float height = (surface->y / (float)tile_size) * ratio;
    const float size   = (surface->h / (float)tile_size) * ratio - ratio;

    return (Fang_Rect){
        .x = surface->x,
        .y = (int)(
            (viewport->h / 2)
          - (ratio / 2.0f)
          - height
          - size
          + (camera->cam.z * viewport->h)
          + (camera->pos.z / dist)
        ),
        .w = surface->w,
        .h = (int)(ratio + size),
    };
}

static inline Fang_Rect
Fang_CameraProjectBody(
    const Fang_Camera * const camera,
    const Fang_Body   * const body,
    const Fang_Rect   * const viewport,
          float       * const out_dist)
{
    assert(camera);
    assert(body);
    assert(viewport);

    const float inv_det = {
        1.0f / (camera->cam.x * camera->dir.y - camera->dir.x * camera->cam.y)
    };

    const Fang_Vec2 dist = {
        .x = body->pos.x - camera->pos.x,
        .y = body->pos.y - camera->pos.y,
    };

    const Fang_Vec2 transform = {
        .x = inv_det * ( camera->dir.y * dist.x - camera->dir.x * dist.y),
        .y = inv_det * (-camera->cam.y * dist.x + camera->cam.x * dist.y),
    };

    *out_dist = transform.y;

    if (transform.y <= 0.0f)
        return (Fang_Rect){.h = 0};

    const float size = (viewport->h / transform.y) * body->size;

    return (Fang_Rect){
        .x = (int)(
            (viewport->w / 2.0f) * (1.0f - transform.x / transform.y)
          - (size / 2.0f)
        ),
        .y = (int)(
            (viewport->h / 2.0f) - (size / 2.0f)
          + (camera->cam.z * viewport->h)
          + (camera->pos.z / transform.y)
        ),
        .w = (int)size,
        .h = (int)size,
    };
}
