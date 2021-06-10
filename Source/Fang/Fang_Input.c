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

/**
 * A simple input button.
 *
 * To facilitate actions like double-tapping a button, the number of state
 * transitions (i.e. movements from 0 -> 1 or vice versa) is saved with the
 * button. This is separate from whether the button is currently being pressed.
**/
typedef struct Fang_InputButton {
    bool pressed;
    int  transitions;
} Fang_InputButton;

/**
 * A dual-axis analog stick with an additional button.
 *
 * This assumes that the user can press the joystick inwards to activate an
 * additional button. If no such button exists on the user's controller this
 * functionality is ignored.
**/
typedef struct Fang_InputJoystick {
    float x, y;
    Fang_InputButton button;
} Fang_InputJoystick;

/**
 * A mouse input device.
 *
 * This will be directly tied to an actual, physical mouse. It may be a tracked
 * mouse or a touchpad (though actual touch gestures are not supported). If a
 * mouse is not connected this functionality is ignored.
**/
typedef struct Fang_InputMouse {
    Fang_InputButton left;
    Fang_InputButton right;
    Fang_InputButton middle;
    Fang_Point       position;
    Fang_Point       relative;
} Fang_InputMouse;

/**
 * A game controller input device.
 *
 * This can either be an actual, connected gamepad, or it could be the keyboard.
 * The platform layer handles mapping the keyboard keys to the appropriate
 * attributes of the controller.
 *
 * Note that since keyboard keys cannot have analog representations, holding
 * down a key will set its analog input to either -1.0f or 1.0f with no
 * in-between.
**/
typedef struct Fang_InputController {
    Fang_InputButton start;
    Fang_InputButton back;

    Fang_InputJoystick joystick_left;
    Fang_InputJoystick joystick_right;

    float trigger_left;
    float trigger_right;
    Fang_InputButton   shoulder_left;
    Fang_InputButton   shoulder_right;

    Fang_InputButton direction_up;
    Fang_InputButton direction_down;
    Fang_InputButton direction_left;
    Fang_InputButton direction_right;

    Fang_InputButton action_up;
    Fang_InputButton action_down;
    Fang_InputButton action_left;
    Fang_InputButton action_right;
} Fang_InputController;

/**
 * The modes that the text-input state may be in.
**/
typedef enum Fang_InputTextMode {
    FANG_INPUTTEXT_INACTIVE,
    FANG_INPUTTEXT_TYPING,
    FANG_INPUTTEXT_EDITING,
} Fang_InputTextMode;

/**
 * A structure representing a text input event.
 *
 * If the active attribute is true, then the following attributes will represent
 * the user's text entry or editing (the text entered, the cursor position,
 * and a selection length). The cursor and length values will only be valid
 * during editing.
**/
typedef struct Fang_InputText {
    int32_t cursor;
    int32_t length;
    char    text[32];
    Fang_InputTextMode mode;
} Fang_InputText;

/**
 * A structure representing the input state for a given frame.
 *
 * @see Fang_InputClear()
**/
typedef struct Fang_Input {
    Fang_InputText       text;
    Fang_InputMouse      mouse;
    Fang_InputController controller;
} Fang_Input;

/**
 * Returns whether the button was pressed during the frame.
**/
static inline bool
Fang_InputPressed(
    const Fang_InputButton * const button)
{
    assert(button);
    return button->pressed && button->transitions;
}

/**
 * Returns whether the button was released during the frame.
**/
static inline bool
Fang_InputReleased(
    const Fang_InputButton * const button)
{
    assert(button);
    return !button->pressed && button->transitions;
}

/**
 * Resets the transition counts and relative positions for the inputs.
 *
 * Buttons and relative positions (such as for the mouse) will be reset back to
 * 0, but analog values will remain the same. This function should be called
 * once per frame before handling OS input events.
 *
 * This function will also reset the input's text mode back to
 * FANG_INPUTTEXT_INACTIVE. It will not clear the text that is already in the
 * text buffer.
**/
static void
Fang_InputReset(
    Fang_Input * const input)
{
    assert(input);

    input->controller.start.transitions = 0;
    input->controller.back.transitions = 0;
    input->controller.joystick_left.button.transitions = 0;
    input->controller.joystick_right.button.transitions = 0;
    input->controller.shoulder_left.transitions = 0;
    input->controller.shoulder_right.transitions = 0;
    input->controller.direction_up.transitions = 0;
    input->controller.direction_down.transitions = 0;
    input->controller.direction_left.transitions = 0;
    input->controller.direction_right.transitions = 0;
    input->controller.action_up.transitions = 0;
    input->controller.action_down.transitions = 0;
    input->controller.action_left.transitions = 0;
    input->controller.action_right.transitions = 0;

    input->text.mode = FANG_INPUTTEXT_INACTIVE;
}
