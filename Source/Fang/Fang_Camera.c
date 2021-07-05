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

    const Fang_Vec3 dir = camera->dir;
    const Fang_Vec3 cam = camera->cam;

    camera->dir.x = dir.x * rotation.x - dir.y * rotation.y;
    camera->dir.y = dir.x * rotation.y + dir.y * rotation.x;
    camera->cam.x = cam.x * rotation.x - cam.y * rotation.y;
    camera->cam.y = cam.x * rotation.y + cam.y * rotation.x;
    camera->cam.z = clamp(camera->cam.z + pitch, -1.0f, 1.0f);
}

static inline Fang_Rect
Fang_CameraProjectTile(
    const Fang_Camera   * const camera,
    const Fang_Tile     * const tile,
          float                 dist,
    const Fang_Rect     * const viewport)
{
    assert(camera);
    assert(viewport);

    dist = viewport->h / dist;

    const float offset = (tile->y       * FANG_PROJECTION_RATIO) * dist;
    const float size   = (tile->h       * FANG_PROJECTION_RATIO) * dist;
    const float height = (camera->pos.z * FANG_PROJECTION_RATIO) * dist;
    const float pitch  = (camera->cam.z * viewport->h);

    return (Fang_Rect){
        .y = (int)roundf((viewport->h / 2) - offset - size + height + pitch),
        .h = (int)roundf(size),
    };
}

static inline Fang_Rect
Fang_CameraProjectBody(
    const Fang_Camera * const camera,
    const Fang_Body   * const body,
    const Fang_Rect   * const viewport,
          float       * const out_depth)
{
    assert(camera);
    assert(body);
    assert(viewport);

    const Fang_Vec2 diff = {
        .x = body->pos.x - camera->pos.x,
        .y = body->pos.y - camera->pos.y,
    };

    const Fang_Vec2 plane_pos = {
        .x = ( camera->dir.y * diff.x - camera->dir.x * diff.y),
        .y = (-camera->cam.y * diff.x + camera->cam.x * diff.y),
    };

    if (plane_pos.y <= 0.0f)
        return (Fang_Rect){.h = 0};

    *out_depth = (plane_pos.y * FANG_PROJECTION_RATIO);

    const float dist = (viewport->h / plane_pos.y)
                     * (1.0f / FANG_PROJECTION_RATIO);

    const float size   = (body->size    * FANG_PROJECTION_RATIO) * dist / 2.0f;
    const float offset = (camera->pos.z * FANG_PROJECTION_RATIO) * dist;
    const float pitch  = (camera->cam.z * viewport->h);

    return (Fang_Rect){
        .x = (int)(
            (viewport->w / 2.0f)
          * (1.0f - plane_pos.x / plane_pos.y)
          - (size / 2.0f)
        ),
        .y = (int)(
            (viewport->h / 2.0f)
          - size
          + offset
          + pitch
          - (body->pos.z * FANG_PROJECTION_RATIO * dist)
        ),
        .w = (int)size,
        .h = (int)size,
    };
}
