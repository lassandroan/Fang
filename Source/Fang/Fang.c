// Copyright (C) 2021 Antonio Lassandro

// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.

// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public
// License for more details.

// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "Fang_Color.c"
#include "Fang_Rect.c"
#include "Fang_Image.c"
#include "Fang_Framebuffer.c"
#include "Fang_Render.c"

static inline void
Fang_UpdateAndRender(
    Fang_Framebuffer * const framebuf)
{
    assert(framebuf);

    Fang_FramebufferClear(framebuf);
}
