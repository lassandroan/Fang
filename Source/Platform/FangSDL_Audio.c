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

static SDL_AudioDeviceID audio_device;

static void
FangSDL_AudioCallback(
          void    * const userdata,
          uint8_t * const data,
    const int             len)
{
    SDL_assert(userdata);

    Fang_BufferSounds(
        (Fang_Sounds*)userdata,
        (float*)data,
        len / (int)sizeof(float)
    );
}

static inline int
FangSDL_ConnectAudio(
    Fang_Sounds * const sounds)
{
    SDL_assert(audio_device);
    SDL_assert(sounds);

    const SDL_AudioSpec want = {
        .freq     = 44100,
        .format   = AUDIO_F32,
        .channels = 2,
        .samples  = 128,
        .callback = FangSDL_AudioCallback,
        .userdata = sounds,
    };

    SDL_AudioSpec have;

    audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);

    if (!audio_device)
        return -1;

    SDL_assert(have.freq     == want.freq);
    SDL_assert(have.format   == want.format);
    SDL_assert(have.channels == want.channels);
    SDL_assert(have.samples  == want.samples);

    SDL_PauseAudioDevice(audio_device, 0);

    return 0;
}

static inline void
FangSDL_DisconnectAudio(void)
{
    SDL_assert(audio_device);

    SDL_CloseAudioDevice(audio_device);
}
