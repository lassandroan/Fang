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

#include "FangSDL_File.c"
#include "FangSDL_Input.c"
#include "FangSDL_Audio.c"

Fang_Input           input;
SDL_GameController * controller;

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

    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER))
        goto Error_SDL;

    SDL_Window * const window = SDL_CreateWindow(
        FANG_TITLE,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        512,
        512,
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

    SDL_Texture * const target = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        FANG_WINDOW_SIZE,
        FANG_WINDOW_SIZE
    );

    if (!target)
        goto Error_Texture;

    SDL_RenderSetIntegerScale(renderer, true);
    SDL_RenderSetLogicalSize(renderer, FANG_WINDOW_SIZE, FANG_WINDOW_SIZE);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_RaiseWindow(window);

    FangSDL_InitInput(&input, &controller);

    {
        const Fang_InitResult init = Fang_Init();
        FangSDL_ConnectAudio(init.sounds);
    }

    while (!SDL_QuitRequested())
    {
        FangSDL_PollEvents(&input, &controller);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        const Fang_Image * const frame = Fang_Update(&input, SDL_GetTicks());
        SDL_assert(Fang_ImageValid(frame));

        SDL_UpdateTexture(target, NULL, frame->pixels, frame->pitch);
        SDL_RenderCopy(renderer, target, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    FangSDL_DisconnectController(&controller);

    Fang_Quit();

Error_Texture:
    SDL_DestroyTexture(target);

Error_Renderer:
    SDL_DestroyRenderer(renderer);

Error_Window:
    SDL_DestroyWindow(window);

    FangSDL_DisconnectAudio();

Error_SDL:
    puts(SDL_GetError());
    SDL_ClearError();
    SDL_Quit();

    return EXIT_SUCCESS;
}
