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

#include <stdbool.h>
#include <stdlib.h>

#include <SDL2/SDL.h>

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

    if (SDL_Init(SDL_INIT_VIDEO))
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

    SDL_RenderSetIntegerScale(renderer, true);
    SDL_RenderSetLogicalSize(renderer, 512, 512);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    bool quit = false;
    while (!quit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    quit = true;
                    break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }

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
