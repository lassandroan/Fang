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
 * A structure to hold colors used in interface components.
**/
typedef struct Fang_InterfaceColors {
    Fang_Color background;
    Fang_Color foreground;
    Fang_Color highlight;
    Fang_Color disabled;
} Fang_InterfaceColors;

/**
 * A structure to hold theme properties used in interface components.
**/
typedef struct Fang_InterfaceTheme {
    Fang_InterfaceColors colors;
    Fang_Texture         font;
} Fang_InterfaceTheme;

/**
 * This interface state holds the identifiers used for the immediate-mode,
 * graphical user interface elements.
 *
 * The id acts as a counter that the IMGUI functions can use to assign
 * themselves an id number. This id number is then compared against the hot,
 * next, and active attributes so the element can decide how it should handle
 * user interaction.
 *
 * If an element is the 'active' item, it has full control and focus from the
 * user. Any interaction that occurs should be considered to be for that given
 * UI element during this frame. For example, if button were the active item it
 * would mean a mouse-release event in that frame end a 'click' for button.
 *
 * The 'hot' item is the item who has user focus but is not currently being
 * interacted with. To take the previous example, if a mouse-down event happened
 * on a 'hot' button then it would begin a 'click' for the button. However, as
 * noted before, the click would not 'complete' until the button was active and
 * received a mouse-up.
 *
 * The 'next' item is the item who is trying to gain user focus. For our example
 * button, it would set itself to the 'next' item if the mouse was within its
 * bounds. In the following frame - if no other element tried to claim 'next' -
 * the button would become the 'hot' item.
**/
typedef struct Fang_Interface {
    uint32_t id;
    uint32_t hot;
    uint32_t next;
    uint32_t active;

    Fang_InterfaceTheme theme;

          Fang_Framebuffer * framebuf;
    const Fang_Atlas       * textures;
    const Fang_Input       * input;
} Fang_Interface;

/**
 * Sets the interface object up for a new frame.
 *
 * The current 'next' item will become the 'hot' item, and the 'next' item will
 * be set to 0. The 'id' will also be set to 0.
 *
 * This should be called at the beginning of each frame.
**/
static inline void
Fang_InterfaceUpdate(
    Fang_Interface * const interface)
{
    assert(interface);

    if (!interface->active)
        interface->hot = interface->next;

    interface->id   = 0;
    interface->next = 0;
}

static inline bool
Fang_InterfaceButton(
          Fang_Interface * const interface,
    const char           * const text,
    const Fang_Rect      * const bounds)
{
    assert(interface);
    assert(bounds);

    const uint32_t id = ++interface->id;

    bool result = false;

    {
        const Fang_Input * const input = interface->input;
        assert(input);

        const bool hot    = interface->hot    == id;
        const bool active = interface->active == id;

        if (active)
        {
            if (Fang_InputReleased(&input->mouse.left)
            ||  Fang_InputReleased(&input->controller.action_down))
            {
                if (hot)
                    result = true;

                interface->active = 0;
            }
        }
        else if (hot)
        {
            if (Fang_InputPressed(&input->mouse.left)
            ||  Fang_InputPressed(&input->controller.action_down))
                interface->active = id;
        }

        if (Fang_RectContains(bounds, &input->mouse.position))
            interface->next = id;
    }

    {
        Fang_Framebuffer * const framebuf = interface->framebuf;
        assert(framebuf);

        const bool hot    = interface->hot    == id;
        const bool active = interface->active == id;

        if (active)
            Fang_FillRect(framebuf, bounds, &interface->theme.colors.highlight);
        else if (hot)
            Fang_DrawRect(framebuf, bounds, &interface->theme.colors.foreground);
        else
            Fang_DrawRect(framebuf, bounds, &interface->theme.colors.disabled);

        if (text)
        {
            const Fang_Rect text_area = Fang_RectResize(
                bounds, -4, -bounds->h / 2
            );

            Fang_DrawText(
                framebuf,
                text,
                Fang_AtlasQuery(interface->textures, interface->theme.font),
                text_area.h,
                &(Fang_Point){
                    .x = (
                        (text_area.x + (text_area.w / 2))
                      - ((int)strlen(text) * text_area.h) / 2
                    ),
                    .y = text_area.y,
                }
            );
        }
    }

    return result;
}

static inline bool
Fang_InterfaceSlider(
          Fang_Interface * const interface,
          float          * const value,
    const char           * const text,
    const Fang_Rect      * const bounds)
{
    assert(interface);
    assert(bounds);

    const uint32_t id = ++interface->id;

    bool result = false;

    {
        const Fang_Input * const input = interface->input;
        assert(input);

        const bool hot    = interface->hot    == id;
        const bool active = interface->active == id;

        if (active)
        {
            if (Fang_InputReleased(&input->mouse.left)
            ||  Fang_InputReleased(&input->controller.action_down))
            {
                interface->active = 0;
            }
            else
            {
                result = true;

                const int left     = bounds->x;
                const int right    = bounds->x + bounds->w;
                const int position = input->mouse.position.x;

                if (position <= left)
                {
                    *value = 0.0f;
                }
                else if (position >= right)
                {
                    *value = 1.0f;
                }
                else
                {
                    *value = (float)(position - left)
                           / (float)(right - left);
                }
            }
        }
        else if (hot)
        {
            if (Fang_InputPressed(&input->mouse.left)
            ||  Fang_InputPressed(&input->controller.action_down))
                interface->active = id;
        }

        if (Fang_RectContains(bounds, &input->mouse.position))
            interface->next = id;
    }

    {
        Fang_Framebuffer * const framebuf = interface->framebuf;
        assert(framebuf);

        const bool active = interface->active == id;
        const bool hot    = interface->hot    == id;

        Fang_Color color;

        if (active)
            color = interface->theme.colors.foreground;
        else if (hot)
            color = interface->theme.colors.highlight;
        else
            color = interface->theme.colors.disabled;

        Fang_DrawRect(framebuf, bounds, &color);

        Fang_Rect fill_area = Fang_RectResize(bounds, -1, -1);

        Fang_FillRect(
            framebuf,
            &(Fang_Rect){
                .x = fill_area.x,
                .y = fill_area.y,
                .w = (int)roundf(fill_area.w * (*value)),
                .h = fill_area.h,
            },
            &color
        );

        {
            char value_text[5];

            if (active)
                snprintf(value_text, sizeof(value_text), "%.2f", *value);

            const char * const display_text = (active) ? value_text : text;

            if (display_text)
            {
                const Fang_Rect text_area = Fang_RectResize(
                    bounds, -4, -bounds->h / 2
                );

                Fang_DrawText(
                    framebuf,
                    display_text,
                    Fang_AtlasQuery(interface->textures, interface->theme.font),
                    text_area.h,
                    &(Fang_Point){
                        .x = (
                            (text_area.x + (text_area.w / 2))
                          - ((int)strlen(display_text) * text_area.h) / 2
                        ),
                        .y = text_area.y,
                    }
                );
            }
        }
    }

    return result;
}
