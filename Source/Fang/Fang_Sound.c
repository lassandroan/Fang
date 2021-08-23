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

typedef size_t Fang_SoundId;

/**
 * The type of sound that the sound effect is.
 *
 * "Global" sounds are those such as user interface sound effects. They are
 * monophonic and are not affected by any audio processing effects. "Positional"
 * sounds are those that exist in the game world, and as such are subject to
 * stereophonic panning and world sound effects like reverberation.
**/
typedef enum Fang_SoundType {
    FANG_SOUNDTYPE_GLOBAL,
    FANG_SOUNDTYPE_POSITIONAL,
} Fang_SoundType;

typedef struct Fang_Sound {
    Fang_SoundId   id;
    Fang_AudioId   audio;
    Fang_SoundType type;
    Fang_Vec2      world_position;
    int            sample_position;
} Fang_Sound;

typedef struct Fang_Music {
    Fang_AudioId audio;
    int          sample_position;
} Fang_Music;

typedef struct Fang_Musics {
    Fang_Music current;
    Fang_Music next;
    float      fade;
} Fang_Musics;

typedef struct Fang_Sounds {
    Fang_Audios  audios;
    Fang_Sound   sounds[FANG_MAX_SOUNDS];
    Fang_SoundId last_out;
    Fang_Musics  musics;
    struct {
        Fang_Vec2 position;
        Fang_Vec2 direction;
    } listener;
} Fang_Sounds;

static inline Fang_Sound *
Fang_GetSound(
          Fang_Sounds  * const sounds,
    const Fang_SoundId         sound_id)
{
    assert(sounds);
    assert(sound_id < FANG_MAX_SOUNDS);

    Fang_Sound * const result = &sounds->sounds[sound_id];

    if (result->audio == FANG_AUDIO_NONE)
        return NULL;

    return result;
}

static inline Fang_SoundId
Fang_QueueSound(
          Fang_Sounds * const sounds,
    const Fang_Sound  * const initial)
{
    assert(sounds);
    assert(sounds->last_out < FANG_MAX_SOUNDS);
    assert(initial);

    const Fang_SoundId result = sounds->last_out;

    {
        Fang_Sound * const sound = &sounds->sounds[result];

        memcpy(sound, initial, sizeof(Fang_Sound));

        sound->id = result;
        sound->sample_position = 0;
    }

    const bool last_out_valid = result < FANG_MAX_SOUNDS - 1;
    const bool last_out_open  = !Fang_GetSound(
        sounds, sounds->last_out + 1
    );

    if (last_out_valid && last_out_open)
    {
        sounds->last_out++;
    }
    else
    {
        for (Fang_SoundId i = 0; i < FANG_MAX_SOUNDS; ++i)
        {
            if (i == result)
                continue;

            if (!Fang_GetSound(sounds, i))
            {
                sounds->last_out = i;
                break;
            }
        }
    }

    // The sound limit has been reached!
    assert(sounds->last_out != result);

    return result;
}

static inline void
Fang_DequeueSound(
          Fang_Sounds * const sounds,
    const Fang_SoundId        sound_id)
{
    assert(sounds);

    Fang_Sound * const sound = Fang_GetSound(sounds, sound_id);

    if (!sound)
        return;

    memset(sound, 0, sizeof(Fang_Sound));
    sound->audio = FANG_AUDIO_NONE;

    sounds->last_out = sound_id;
}

static inline void
Fang_QueueMusic(
          Fang_Sounds  * const sounds,
    const Fang_AudioId         audio)
{
    assert(sounds);

    if (sounds->musics.current.audio == audio)
        return;

    assert(sounds->musics.next.audio == FANG_AUDIO_NONE);
    assert(sounds->musics.fade <= 0.0f);

    sounds->musics.next.audio           = audio;
    sounds->musics.next.sample_position = 0;

    sounds->musics.fade = FANG_MUSIC_FADE;
}

static inline void
Fang_StopMusic(
    Fang_Sounds * const sounds)
{
    assert(sounds);

    Fang_QueueMusic(sounds, FANG_AUDIO_NONE);
}

static inline void
Fang_BufferSounds(
          Fang_Sounds * const sounds,
          float       * const buffer,
    const int                 samples)
{
    assert(sounds);
    assert(buffer);
    assert(samples % 2 == 0);

    for (int i = 0; i < samples; ++i)
        buffer[i] = 0.0f;

    const float direction_angle = Fang_Vec2Angle(sounds->listener.direction);

    for (Fang_SoundId j = 0; j < FANG_MAX_SOUNDS; ++j)
    {
        Fang_Sound * const sound = Fang_GetSound(sounds, j);

        if (!sound)
            continue;

        float pan_l = 1.0f;
        float pan_r = 1.0f;

        if (sound->type == FANG_SOUNDTYPE_POSITIONAL)
        {
            const Fang_Vec2 position = Fang_Vec2Sub(
                sounds->listener.position,
                sound->world_position
            );

            const float angle = Fang_Vec2Angle(position) - direction_angle;

            pan_l = clamp(pan_l - sinf(angle), 0.0f, 1.0f);
            pan_r = clamp(pan_r + sinf(angle), 0.0f, 1.0f);
        }

        Fang_Audio * const audio = Fang_GetAudio(&sounds->audios, sound->audio);

        assert(audio);

        for (int i = 0; i < samples; i += 2)
        {
            const float sample = audio->data[sound->sample_position++];

            buffer[i + 0] += sample * pan_l;
            buffer[i + 1] += sample * pan_r;

            if (sound->sample_position >= audio->samples)
            {
                Fang_DequeueSound(sounds, j);
                break;
            }
        }
    }

    {
        Fang_Musics * const musics = &sounds->musics;

        Fang_Audio * current = Fang_GetAudio(
            &sounds->audios, musics->current.audio
        );

        Fang_Audio * next = Fang_GetAudio(
            &sounds->audios, musics->next.audio
        );

        if (!current && !next)
            return;

        if (current == next)
            next = NULL;

        bool fade_ended = false;

        if (musics->fade > 0.0f)
        {
            for (int i = 0; i < samples; i += 2)
            {
                float fade     = musics->fade / FANG_MUSIC_FADE;
                float fade_inv = 1.0f - fade;

                if (current)
                {
                    const float sample = (
                        current->data[musics->current.sample_position] * fade
                    );

                    buffer[i + 0] += sample;
                    buffer[i + 1] += sample;

                    musics->current.sample_position++;

                    if (musics->current.sample_position >= current->samples - 1)
                        musics->current.sample_position = 0;
                }

                if (next)
                {
                    const float sample = (
                        next->data[musics->next.sample_position] * fade_inv
                    );

                    buffer[i + 0] += sample;
                    buffer[i + 1] += sample;

                    musics->next.sample_position++;

                    if (musics->next.sample_position >= next->samples - 1)
                        musics->next.sample_position = 0;
                }

                musics->fade -= 1.0f / 16000.0f;

                if (musics->fade <= 0.0f)
                {
                    fade_ended   = true;
                    musics->fade = 0.0f;
                    break;
                }
            }
        }
        else if (current)
        {
            for (int i = 0; i < samples; ++i)
            {
                const float sample = (
                    current->data[musics->current.sample_position]
                );

                buffer[i + 0] += sample;
                buffer[i + 1] += sample;

                musics->current.sample_position++;

                if (musics->current.sample_position >= current->samples - 1)
                    musics->current.sample_position = 0;
            }
        }

        if (fade_ended)
        {
            musics->current.audio           = musics->next.audio;
            musics->current.sample_position = musics->next.sample_position;

            musics->next.audio           = FANG_AUDIO_NONE;
            musics->next.sample_position = 0;

            musics->fade = 0.0f;

            current = next;
            next    = NULL;
        }
    }
}
