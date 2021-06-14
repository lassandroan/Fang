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

enum {
  FANG_WINDOW_SIZE = 512,
  FANG_TILE_SIZE   = 32,
};

#include "Platform/Fang_SDL.c"

int main(int argc, char **argv)
{
    // NOTE: When building with Sublime the output panel doesn't seem to update
    //       properly without an unbuffered stdout
    #ifdef FANG_UNBUFFERED_STDOUT
      setbuf(stdout, NULL);
    #endif

    return Fang_Main(argc, argv);
}
