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

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#ifndef min
  #define min(x, y) (x < y ? x : y)
#endif

#ifndef max
  #define max(x, y) (x > y ? x : y)
#endif

#include "Fang_Memory.c"
#include "Fang_Color.c"
#include "Fang_Rect.c"
#include "Fang_Input.c"
#include "Fang_Image.c"
#include "Fang_TGA.c"
#include "Fang_Font.c"
#include "Fang_Framebuffer.c"
#include "Fang_Render.c"
#include "Fang_Interface.c"

Fang_Interface interface;

static inline void
Fang_UpdateAndRender(
    const Fang_Input       * const input,
          Fang_Framebuffer * const framebuf)
{
    assert(input);
    assert(framebuf);

    interface.input    = input;
    interface.framebuf = framebuf;
    Fang_InterfaceUpdate(&interface);

    Fang_FramebufferClear(framebuf);

    Fang_DrawText(
        framebuf,
        "FANG",
        FANG_FONT_FORMULA,
        &(Fang_Point){
            .x = (FANG_WINDOW_SIZE / 2) - (FANG_FONTAREA_WIDTH * 2),
            .y = (FANG_WINDOW_SIZE / 2) - (FANG_FONTAREA_HEIGHT * 2),
        }
    );
}
