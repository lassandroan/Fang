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
 * All fonts should be 8x9 in size, with a 1px barrier in between each
 * character.
**/
enum {
    FANG_FONTAREA_HEIGHT = 9,
    FANG_FONTAREA_WIDTH  = 8,
};

/**
 * The fonts available for the game.
 *
 * Each font in the resources folder should have a type present in this enum.
**/
typedef enum Fang_FontType {
    FANG_FONT_FORMULA,

    FANG_NUM_FONT,
} Fang_FontType;

/**
 * Holds the path that each font is located at.
 *
 * This is used by the platform layer to find fonts within the compiled game's
 * resource folder.
**/
const char * const Fang_FontPaths[FANG_NUM_FONT] = {
    [FANG_FONT_FORMULA] = "Fonts/Formula.tga",
};

/**
 * Static storage for the font textures.
 *
 * The entries here are filled by the platform layer at startup.
**/
Fang_Image Fang_Fonts[FANG_NUM_FONT];

/**
 * Helper function for grabbing the right Fang_Image* by font type ID.
**/
static inline Fang_Image*
Fang_FontGet(
    const Fang_FontType type)
{
    assert(type >= 0);
    assert(type <= FANG_NUM_FONT);

    if (type == FANG_NUM_FONT)
        return &Fang_Fonts[type - 1];

    return &Fang_Fonts[type];
}

/**
 * Returns the subsection of the font image that contains the given character.
 *
 * The character map begins at '!' (33) and extends to DEL (127). Spaces should
 * be handled by the caller.
 *
 * If the character is out of range this will return an empty rectangle.
**/
static inline Fang_Rect
Fang_FontGetCharPosition(
    const char text)
{
    if (text < '!')
        printf("fuck this: %c %d\n", text, text);

    assert(text >= '!');

    if (text < '!')
        return (Fang_Rect){0, 0, 0, 0};

    const float pos = (text - 33.0f) / (127.0f - 33.0f);

    const int total_width = (127 - 33) * (FANG_FONTAREA_WIDTH + 1);

    return (Fang_Rect){
        .x = (int)(total_width * pos) + 1,
        .y = 0,
        .w = FANG_FONTAREA_WIDTH,
        .h = FANG_FONTAREA_HEIGHT,
    };
}
