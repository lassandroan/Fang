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

Fang_FileError
Fang_LoadFile(
    const char      * const filename,
          Fang_File * const result)
{
    SDL_assert(filename);
    SDL_assert(result);
    SDL_assert(!result->data);
    SDL_assert(!result->size);

    Fang_FileError error = FANG_FILE_ERROR_NONE;

    SDL_RWops * file = NULL;

    char * const full_path = FangSDL_GetResourcePath(filename);
    if (!full_path)
        goto Error_CantOpen;

    file = SDL_RWFromFile(full_path, "rb");
    if (!file)
        goto Error_CantOpen;

    const int64_t size = SDL_RWsize(file);
    if (size < 0)
        goto Error_CantStat;

    result->data = SDL_malloc((size_t)size);
    result->size = (size_t)size;

    if (!result->data)
        goto Error_CantMake;

    const size_t num_read = SDL_RWread(
        file,
        result->data,
        sizeof(uint8_t),
        (size_t)size
    );

    if (num_read / sizeof(uint8_t) != (size_t)size)
        goto Error_CantRead;

    SDL_RWclose(file);
    SDL_free(full_path);
    return FANG_FILE_ERROR_NONE;

Error_CantOpen:
    error = FANG_FILE_ERROR_CANT_OPEN;
    goto Error;

Error_CantStat:
    error = FANG_FILE_ERROR_UNKNOWN_SIZE;
    goto Error;

Error_CantMake:
    error = FANG_FILE_ERROR_BAD_ALLOCATION;
    goto Error;

Error_CantRead:
    error = FANG_FILE_ERROR_BAD_READ;
    goto Error;

Error:
    if (file)
        SDL_RWclose(file);

    SDL_free(full_path);
    SDL_free(result->data);

    result->data = NULL;
    result->size = 0;
    return error;
}

void
Fang_FreeFile(
    Fang_File * const file)
{
    SDL_assert(file);
    SDL_assert(file->data);
    SDL_assert(file->size);

    SDL_free(file->data);
    file->data = NULL;
    file->size = 0;
}
