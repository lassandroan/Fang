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

typedef enum Fang_AudioId {
    /* Music Tracks */
    FANG_AUDIO_METRONOME,

    /* Sound Effects */
    FANG_AUDIO_DTMF,
    FANG_AUDIO_TONE,
    FANG_AUDIO_RISSET,

    FANG_NUM_AUDIOS,
    FANG_AUDIO_NONE,
} Fang_AudioId;

typedef struct Fang_Audio {
    float * data;
    int     samples;
} Fang_Audio;

typedef struct Fang_Audios {
    Fang_Audio audios[FANG_NUM_AUDIOS];
} Fang_Audios;

static Fang_Audio
Fang_LoadWAV(const char*);

static inline bool
Fang_AudioValid(
    const Fang_Audio * const audio)
{
    return (audio && audio->data && audio->samples);
}

static inline Fang_Audio *
Fang_GetAudio(
          Fang_Audios  * const audios,
    const Fang_AudioId         audio_id)
{
    assert(audios);

    if (audio_id >= FANG_NUM_AUDIOS)
        return NULL;

    Fang_Audio * const result = &audios->audios[audio_id];

    if (!Fang_AudioValid(result))
        return NULL;

    return result;
}

static inline void
Fang_FreeAudio(
          Fang_Audios  * const audios,
    const Fang_AudioId         id)
{
    assert(audios);
    assert(id < FANG_NUM_AUDIOS);

    Fang_Audio * const result = &audios->audios[id];

    if (result->data)
        free(result->data);

    memset(result, 0, sizeof(Fang_Audio));
}

static inline int
Fang_LoadAudio(
          Fang_Audios  * const audios,
    const Fang_AudioId         id)
{
    assert(audios);
    assert(id < FANG_NUM_AUDIOS);

    Fang_Audio * const result = &audios->audios[id];

    typedef struct {
        const char * const path;
    } Info;

    static const Info audio_info[FANG_NUM_AUDIOS] = {
        [FANG_AUDIO_METRONOME] = {
            .path = "Audio/Metronome.wav",
        },
        [FANG_AUDIO_DTMF] = {
            .path = "Audio/DTMF.wav",
        },
        [FANG_AUDIO_TONE] = {
            .path = "Audio/Tone.wav",
        },
        [FANG_AUDIO_RISSET] = {
            .path = "Audio/Risset.wav",
        },
    };

    {
        const Info info = audio_info[id];

        if (info.path)
        {
            *result = Fang_LoadWAV(info.path);

            if (!Fang_AudioValid(result))
                return 1;
        }
    }

    return 0;
}

static inline int
Fang_LoadAudios(
    Fang_Audios * const audios)
{
    assert(audios);

    int error = 0;

    for (Fang_AudioId i = 0; i < FANG_NUM_AUDIOS; ++i)
        error |= Fang_LoadAudio(audios, i);

    return error;
}

static inline void
Fang_FreeAudios(
    Fang_Audios * const audios)
{
    assert(audios);

    for (Fang_AudioId i = 0; i < FANG_NUM_AUDIOS; ++i)
        Fang_FreeAudio(audios, i);
}
