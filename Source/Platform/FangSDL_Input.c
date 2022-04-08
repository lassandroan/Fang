// Copyright (C) 2022  Antonio Lassandro

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

static inline void
FangSDL_ConnectController(
    SDL_GameController ** const controller)
{
    SDL_assert(controller);

    if (*controller && SDL_GameControllerGetAttached(*controller))
        SDL_GameControllerClose(*controller);

    const int id = SDL_NumJoysticks() - 1;

    if (id < 0)
        return;

    if (!SDL_IsGameController(id))
        return;

    *controller = SDL_GameControllerOpen(id);
}

static inline void
FangSDL_DisconnectController(
    SDL_GameController ** const controller)
{
    SDL_assert(controller);

    if (!*controller)
        return;

    if (SDL_GameControllerGetAttached(*controller))
        SDL_GameControllerClose(*controller);

    *controller = NULL;
}

static inline void
FangSDL_HandleControllerButtonEvent(
          Fang_Input                * const input,
    const SDL_ControllerButtonEvent * const event)
{
    SDL_assert(input);
    SDL_assert(event);

    Fang_InputButton * button = NULL;

    const SDL_GameControllerButton type = event->button;
    if (type == SDL_CONTROLLER_BUTTON_START)
        button = &input->controller.start;
    else if (type == SDL_CONTROLLER_BUTTON_BACK)
        button = &input->controller.back;
    else if (type == SDL_CONTROLLER_BUTTON_LEFTSTICK)
        button = &input->controller.joystick_left.button;
    else if (type == SDL_CONTROLLER_BUTTON_RIGHTSTICK)
        button = &input->controller.joystick_right.button;
    else if (type == SDL_CONTROLLER_BUTTON_DPAD_UP)
        button = &input->controller.direction_up;
    else if (type == SDL_CONTROLLER_BUTTON_DPAD_DOWN)
        button = &input->controller.direction_down;
    else if (type == SDL_CONTROLLER_BUTTON_DPAD_LEFT)
        button = &input->controller.direction_left;
    else if (type == SDL_CONTROLLER_BUTTON_DPAD_RIGHT)
        button = &input->controller.direction_right;
    else if (type == SDL_CONTROLLER_BUTTON_Y)
        button = &input->controller.action_up;
    else if (type == SDL_CONTROLLER_BUTTON_A)
        button = &input->controller.action_down;
    else if (type == SDL_CONTROLLER_BUTTON_X)
        button = &input->controller.action_left;
    else if (type == SDL_CONTROLLER_BUTTON_B)
        button = &input->controller.action_right;
    else if (type == SDL_CONTROLLER_BUTTON_LEFTSHOULDER)
        button = &input->controller.shoulder_left;
    else if (type == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)
        button = &input->controller.shoulder_right;

    if (button)
    {
        button->transitions++;
        button->pressed = (event->state == SDL_PRESSED);
    }
}

static inline void
FangSDL_HandleControllerAxisEvent(
          Fang_Input              * const input,
    const SDL_ControllerAxisEvent * const event)
{
    SDL_assert(input);
    SDL_assert(event);

    const float min_axis = (float)INT16_MIN;
    const float max_axis = (float)INT16_MAX;

    float axis = (event->value - min_axis)
               / (max_axis - min_axis);

    axis = 2.0f * axis - 1.0f;

    if (fabsf(axis) <= 0.1f)
        axis = 0.0f;

    const SDL_GameControllerAxis type = event->axis;

    if (type == SDL_CONTROLLER_AXIS_LEFTX)
        input->controller.joystick_left.x = axis;
    else if (type == SDL_CONTROLLER_AXIS_LEFTY)
        input->controller.joystick_left.y = axis;
    else if (type == SDL_CONTROLLER_AXIS_RIGHTX)
        input->controller.joystick_right.x = axis;
    else if (type == SDL_CONTROLLER_AXIS_RIGHTY)
        input->controller.joystick_right.y = axis;
    else if (type == SDL_CONTROLLER_AXIS_TRIGGERLEFT)
        input->controller.trigger_left = axis;
    else if (type == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
        input->controller.trigger_right = axis;
}

static inline void
FangSDL_HandleMouseMotionEvent(
          Fang_Input           * const input,
    const SDL_MouseMotionEvent * const event)
{
    SDL_assert(input);
    SDL_assert(event);

    input->mouse.position.x = event->x;
    input->mouse.position.y = event->y;
    input->mouse.relative.x = event->xrel;
    input->mouse.relative.y = event->yrel;
}

static inline void
FangSDL_HandleMouseButtonEvent(
          Fang_Input           * const input,
    const SDL_MouseButtonEvent * const event)
{
    SDL_assert(input);
    SDL_assert(event);

    Fang_InputButton * button = NULL;

    const uint8_t type = event->button;
    if (type == SDL_BUTTON_LEFT)
        button = &input->mouse.left;
    else if (type == SDL_BUTTON_MIDDLE)
        button = &input->mouse.middle;
    else if (type == SDL_BUTTON_RIGHT)
        button = &input->mouse.right;

    if (button)
    {
        button->transitions++;
        button->pressed = (event->state == SDL_PRESSED);
    }

    input->mouse.position.x = event->x;
    input->mouse.position.y = event->y;
}

static inline void
FangSDL_HandleKeyboardEvent(
          Fang_Input        * const input,
    const SDL_KeyboardEvent * const event)
{
    SDL_assert(input);
    SDL_assert(event);

    if (event->repeat)
        return;

    Fang_InputButton * button = NULL;

    SDL_Keycode sym = event->keysym.sym;
    if (sym == SDLK_w)
        button = &input->controller.direction_up;
    else if (sym == SDLK_s)
        button = &input->controller.direction_down;
    else if (sym == SDLK_a)
        button = &input->controller.direction_left;
    else if (sym == SDLK_d)
        button = &input->controller.direction_right;
    else if (sym == SDLK_q)
        button = &input->controller.shoulder_left;
    else if (sym == SDLK_e)
        button = &input->controller.shoulder_right;
    else if (sym == SDLK_SPACE)
        button = &input->controller.action_down;
    else if (sym == SDLK_LSHIFT)
        button = &input->controller.joystick_left.button;

    if (button)
    {
        button->transitions++;
        button->pressed = (event->state == SDL_PRESSED);
    }
}

static inline void
FangSDL_HandleTextInputEvent(
          Fang_Input         * const input,
    const SDL_TextInputEvent * const event)
{
    SDL_assert(input);
    SDL_assert(event);

    input->text.mode   = FANG_INPUTTEXT_TYPING;
    input->text.cursor = 0;
    input->text.length = 0;
    SDL_memcpy(input->text.text, event->text, sizeof(char[32]));
}

static inline void
FangSDL_HandleTextEditingEvent(
          Fang_Input           * const input,
    const SDL_TextEditingEvent * const event)
{
    SDL_assert(input);
    SDL_assert(event);

    input->text.mode   = FANG_INPUTTEXT_EDITING;
    input->text.cursor = event->start;
    input->text.length = event->length;
    SDL_memcpy(input->text.text, event->text, sizeof(char[32]));
}

static inline void
FangSDL_PollEvents(
    Fang_Input         *  const input,
    SDL_GameController ** const controller)
{
    SDL_assert(input);

    Fang_ClearInput(input);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_JOYDEVICEADDED:
            {
                FangSDL_ConnectController(controller);
                break;
            }

            case SDL_JOYDEVICEREMOVED:
            {
                FangSDL_DisconnectController(controller);
                break;
            }

            case SDL_CONTROLLERAXISMOTION:
            {
                FangSDL_HandleControllerAxisEvent(input, &event.caxis);
                break;
            }

            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
            {
                FangSDL_HandleControllerButtonEvent(input, &event.cbutton);
                break;
            }

            case SDL_MOUSEMOTION:
            {
                FangSDL_HandleMouseMotionEvent(input, &event.motion);
                break;
            }

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            {
                FangSDL_HandleMouseButtonEvent(input, &event.button);
                break;
            }

            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                FangSDL_HandleKeyboardEvent(input, &event.key);
                break;
            }

            case SDL_TEXTINPUT:
            {
                FangSDL_HandleTextInputEvent(input, &event.text);
                break;
            }

            case SDL_TEXTEDITING:
            {
                FangSDL_HandleTextEditingEvent(input, &event.edit);
                break;
            }
        }
    }
}

static inline void
FangSDL_InitInput(
    Fang_Input         *  const input,
    SDL_GameController ** const controller)
{
    SDL_assert(input);
    SDL_assert(controller);
    SDL_assert(*controller == NULL);

    SDL_JoystickEventState(SDL_ENABLE);
    SDL_GameControllerEventState(SDL_ENABLE);
    SDL_SetRelativeMouseMode(1);

    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

    FangSDL_ConnectController(controller);
    Fang_ClearInput(input);
}
