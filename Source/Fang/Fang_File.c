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
 * A container for file data with a length (in bytes).
**/
typedef struct Fang_File {
    void   * data;
    size_t   size;
} Fang_File;

/**
 * Error values representing what went wrong during the loading of a resource.
**/
typedef enum Fang_FileError {
    FANG_FILE_ERROR_NONE,
    FANG_FILE_ERROR_CANT_OPEN,
    FANG_FILE_ERROR_UNKNOWN_SIZE,
    FANG_FILE_ERROR_BAD_ALLOCATION,
    FANG_FILE_ERROR_BAD_READ,

    FANG_NUM_FILE_ERROR,
} Fang_FileError;

/**
 * Loads a game file.
 *
 * This function is defined by the platform layer, but will typically prepend
 * the supplied file name with the resource path (such as the app bundle, data
 * directory, etc.) and then read the file at that location.
**/
FANG_PLATFORM_CALL
Fang_FileError Fang_LoadFile(const char * /* filename */, Fang_File *);

/**
 * Frees a game file that was allocated via Fang_LoadFile().
**/
FANG_PLATFORM_CALL
void Fang_FreeFile(Fang_File *);
