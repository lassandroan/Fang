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

#include "../Fang/Fang.c"
#include <SDL2/SDL.h>

static inline char *
FangSDL_GetResourcePath(
    const char * const filename)
{
    SDL_assert(filename);

    char * const base_path = SDL_GetBasePath();
    const size_t full_len  = SDL_strlen(base_path) + SDL_strlen(filename);
    char * const full_path = SDL_malloc(sizeof(char) * full_len + 1);
    full_path[full_len] = '\0';

    SDL_strlcpy(full_path, base_path, full_len + 1);
    SDL_strlcat(full_path,  filename, full_len + 1);

    SDL_free(base_path);
    return full_path;
}

static inline Fang_Buffer
FangSDL_LoadResource(
    const char * const filename)
{
    SDL_assert(filename);

    Fang_Buffer result = {.data = NULL};

    char * const full_path = FangSDL_GetResourcePath(filename);
    if (!full_path)
        goto Error;

    SDL_RWops * const file = SDL_RWFromFile(full_path, "rb");
    if (!file)
        goto Error;

    const int64_t size = SDL_RWsize(file);
    if (size < 0)
        goto Error;

    result.data = SDL_malloc((size_t)size);
    result.size = (size_t)size;

    if (!result.data)
        goto Error;

    const size_t num_read = SDL_RWread(
        file,
        result.data,
        sizeof(uint8_t),
        (size_t)size
    );

    if (num_read / sizeof(uint8_t) != (size_t)size)
        goto Error;

    SDL_RWclose(file);
    SDL_free(full_path);
    return result;

Error:
    SDL_free(result.data);
    SDL_free(full_path);
    result.data = NULL;
    result.size = 0;
    return result;
}

static inline int
FangSDL_CreateFonts(void)
{
    for (int i = 0; i < FANG_NUM_FONT; ++i)
    {
        const char * const path = Fang_FontPaths[i];

        Fang_Buffer file = FangSDL_LoadResource(path);

        if (!file.data)
            return 1;

        Fang_Image * const font = &Fang_Fonts[i];

        *font = Fang_TGALoad(&file);

        if (!font->pixels)
        {
            SDL_SetError("Unable to load font %s", path);
            return 1;
        }
    }

    return 0;
}

static inline void
FangSDL_DestroyFonts(void)
{
    for (int i = 0; i < FANG_NUM_FONT; ++i)
    {
        assert(Fang_Fonts[i].pixels);

        SDL_free(Fang_Fonts[i].pixels);
        SDL_memset(&Fang_Fonts[i], 0, sizeof(Fang_Image));
    }
}

static inline void
FangSDL_LoadMap(void)
{
    {
        Fang_Buffer file = FangSDL_LoadResource("Maps/test/skybox.tga");

        if (file.data)
        {
            temp_map.skybox = Fang_TGALoad(&file);
            if (!temp_map.skybox.pixels)
                puts("Invalid skybox.tga");
        }
        else
        {
            puts("Could not load skybox_new.tga");
        }
    }

    {
        Fang_Buffer file = FangSDL_LoadResource("Maps/test/floor.tga");

        if (file.data)
        {
            temp_map.floor = Fang_TGALoad(&file);
            if (!temp_map.floor.pixels)
                puts("Invalid floor.tga");
        }
        else
        {
            puts("Could not load floor.tga");
        }
    }
}

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

int Fang_Main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    {
        SDL_version version;
        SDL_VERSION(&version);
        printf(
            "SDL %d.%d.%d\n",
            version.major,
            version.minor,
            version.patch
        );
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER))
        goto Error_SDL;

    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

    SDL_Window * const window = SDL_CreateWindow(
        FANG_TITLE,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        FANG_WINDOW_SIZE * 2,
        FANG_WINDOW_SIZE * 2,
        SDL_WINDOW_SHOWN
            | SDL_WINDOW_ALLOW_HIGHDPI
            | SDL_WINDOW_INPUT_FOCUS
            | SDL_WINDOW_MOUSE_FOCUS
    );

    if (!window)
        goto Error_Window;

    SDL_Renderer * const renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_PRESENTVSYNC
    );

    if (!renderer)
        goto Error_Renderer;

    SDL_Texture * const texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        FANG_WINDOW_SIZE,
        FANG_WINDOW_SIZE
    );

    if (!texture)
        goto Error_Texture;

    if (FangSDL_CreateFonts())
        goto Error_Fonts;

    FangSDL_LoadMap();

    SDL_GameController * controller = NULL;
    FangSDL_ConnectController(&controller);

    SDL_RenderSetIntegerScale(renderer, true);
    SDL_RenderSetLogicalSize(renderer, FANG_WINDOW_SIZE, FANG_WINDOW_SIZE);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    SDL_JoystickEventState(SDL_ENABLE);
    SDL_GameControllerEventState(SDL_ENABLE);

    SDL_RaiseWindow(window);

    Fang_Input input;
    SDL_memset(&input, 0, sizeof(Fang_Input));

    bool quit = false;
    while (!quit)
    {
        Fang_InputReset(&input);

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                {
                    quit = true;
                    break;
                }

                case SDL_JOYDEVICEADDED:
                {
                    FangSDL_ConnectController(&controller);
                    break;
                }

                case SDL_JOYDEVICEREMOVED:
                {
                    FangSDL_DisconnectController(&controller);
                    break;
                }

                case SDL_CONTROLLERAXISMOTION:
                {
                    const float min_axis = (float)INT16_MIN;
                    const float max_axis = (float)INT16_MAX;

                    float axis = (event.caxis.value - min_axis)
                               / (max_axis - min_axis);

                    axis = 2.0f * axis - 1.0f;

                    if (fabsf(axis) <= 0.1f)
                        axis = 0.0f;

                    const SDL_GameControllerAxis type = event.caxis.axis;
                    if (type == SDL_CONTROLLER_AXIS_LEFTX)
                        input.controller.joystick_left.x = axis;
                    else if (type == SDL_CONTROLLER_AXIS_LEFTY)
                        input.controller.joystick_left.y = axis;
                    else if (type == SDL_CONTROLLER_AXIS_RIGHTX)
                        input.controller.joystick_right.x = axis;
                    else if (type == SDL_CONTROLLER_AXIS_RIGHTY)
                        input.controller.joystick_right.y = axis;
                    else if (type == SDL_CONTROLLER_AXIS_TRIGGERLEFT)
                        input.controller.trigger_left = axis;
                    else if (type == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
                        input.controller.trigger_right = axis;

                    break;
                }

                case SDL_CONTROLLERBUTTONDOWN:
                case SDL_CONTROLLERBUTTONUP:
                {
                    Fang_InputButton * button = NULL;

                    const SDL_GameControllerButton type = event.cbutton.button;
                    if (type == SDL_CONTROLLER_BUTTON_START)
                        button = &input.controller.start;
                    else if (type == SDL_CONTROLLER_BUTTON_BACK)
                        button = &input.controller.back;
                    else if (type == SDL_CONTROLLER_BUTTON_LEFTSTICK)
                        button = &input.controller.joystick_left.button;
                    else if (type == SDL_CONTROLLER_BUTTON_RIGHTSTICK)
                        button = &input.controller.joystick_right.button;
                    else if (type == SDL_CONTROLLER_BUTTON_DPAD_UP)
                        button = &input.controller.direction_up;
                    else if (type == SDL_CONTROLLER_BUTTON_DPAD_DOWN)
                        button = &input.controller.direction_down;
                    else if (type == SDL_CONTROLLER_BUTTON_DPAD_LEFT)
                        button = &input.controller.direction_left;
                    else if (type == SDL_CONTROLLER_BUTTON_DPAD_RIGHT)
                        button = &input.controller.direction_right;
                    else if (type == SDL_CONTROLLER_BUTTON_Y)
                        button = &input.controller.action_up;
                    else if (type == SDL_CONTROLLER_BUTTON_A)
                        button = &input.controller.action_down;
                    else if (type == SDL_CONTROLLER_BUTTON_X)
                        button = &input.controller.action_left;
                    else if (type == SDL_CONTROLLER_BUTTON_B)
                        button = &input.controller.action_right;
                    else if (type == SDL_CONTROLLER_BUTTON_LEFTSHOULDER)
                        button = &input.controller.shoulder_left;
                    else if (type == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)
                        button = &input.controller.shoulder_right;

                    if (button)
                    {
                        button->transitions++;
                        button->pressed = (event.cbutton.state == SDL_PRESSED);
                    }

                    break;
                }

                case SDL_MOUSEMOTION:
                {
                    input.mouse.position.x = event.motion.x;
                    input.mouse.position.y = event.motion.y;
                    input.mouse.relative.x = event.motion.xrel;
                    input.mouse.relative.y = event.motion.yrel;
                    break;
                }

                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                {
                    Fang_InputButton * button = NULL;

                    const uint8_t type = event.button.button;
                    if (type == SDL_BUTTON_LEFT)
                        button = &input.mouse.left;
                    else if (type == SDL_BUTTON_MIDDLE)
                        button = &input.mouse.middle;
                    else if (type == SDL_BUTTON_RIGHT)
                        button = &input.mouse.right;

                    if (button)
                    {
                        button->transitions++;
                        button->pressed = (event.button.state == SDL_PRESSED);
                    }

                    input.mouse.position.x = event.button.x;
                    input.mouse.position.y = event.button.y;
                    break;
                }

                case SDL_KEYDOWN:
                case SDL_KEYUP:
                {
                    if (event.key.repeat)
                        break;

                    Fang_InputButton * button = NULL;

                    SDL_Keycode sym = event.key.keysym.sym;
                    if (sym == SDLK_w)
                        button = &input.controller.direction_up;
                    else if (sym == SDLK_s)
                        button = &input.controller.direction_down;
                    else if (sym == SDLK_a)
                        button = &input.controller.direction_left;
                    else if (sym == SDLK_d)
                        button = &input.controller.direction_right;

                    if (button)
                    {
                        button->transitions++;
                        button->pressed = (event.key.state == SDL_PRESSED);
                    }

                    break;
                }

                case SDL_TEXTINPUT:
                {
                    input.text.mode = FANG_INPUTTEXT_TYPING;
                    input.text.cursor = 0;
                    input.text.length = 0;
                    SDL_memcpy(
                        input.text.text,
                        event.text.text,
                        sizeof(char[32])
                    );
                    break;
                }

                case SDL_TEXTEDITING:
                {
                    input.text.mode = FANG_INPUTTEXT_EDITING;
                    input.text.cursor = event.edit.start;
                    input.text.length = event.edit.length;
                    SDL_memcpy(
                        input.text.text,
                        event.edit.text,
                        sizeof(char[32])
                    );
                    break;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        {
            Fang_Framebuffer framebuf;
            framebuf.color.width  = FANG_WINDOW_SIZE;
            framebuf.color.height = FANG_WINDOW_SIZE;
            framebuf.color.stride = SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_RGBA8888);
            framebuf.enable_stencil = false;
            framebuf.transform = Fang_Mat3x3Identity();

            const int error = SDL_LockTexture(
                texture,
                NULL,
                (void**)&framebuf.color.pixels,
                &framebuf.color.pitch
            );

            if (error)
                break;

            Fang_UpdateAndRender(&input, &framebuf);
            SDL_UnlockTexture(texture);
        }

        SDL_SetRenderTarget(renderer, NULL);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    FangSDL_DisconnectController(&controller);

Error_Fonts:
    FangSDL_DestroyFonts();

Error_Texture:
    SDL_DestroyTexture(texture);

Error_Renderer:
    SDL_DestroyRenderer(renderer);

Error_Window:
    SDL_DestroyWindow(window);

Error_SDL:
    puts(SDL_GetError());
    SDL_ClearError();
    SDL_Quit();

    return EXIT_SUCCESS;
}
