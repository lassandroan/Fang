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

static Fang_Audio
Fang_ParseWAV(
    Fang_File * const file)
{
    assert(file);

    Fang_Audio result = {.data = NULL};

    const uint8_t * data = file->data;

    struct {
        uint32_t chunk_id;
        uint32_t chunk_size;
        uint32_t wave_id;
    } riff_chunk;

    Fang_parse(data, riff_chunk.chunk_id,   uint32_t);
    Fang_parse(data, riff_chunk.chunk_size, uint32_t);
    Fang_parse(data, riff_chunk.wave_id,    uint32_t);

    if (!Fang_idcmp(riff_chunk.chunk_id, "RIFF"))
        goto Error_Invalid;

    if (!Fang_idcmp(riff_chunk.wave_id, "WAVE"))
        goto Error_Invalid;

    if (!riff_chunk.chunk_size)
        goto Error_Invalid;

    struct {
        uint32_t chunk_id;
        uint32_t chunk_size;
        uint16_t format;
        uint16_t channels;
        uint32_t sample_rate;
        uint32_t data_rate;
        uint16_t block_align;
        uint16_t bits_per_sample;
    } format_chunk;

    Fang_parse(data, format_chunk.chunk_id, uint32_t);
    Fang_parse(data, format_chunk.chunk_size, uint32_t);
    Fang_parse(data, format_chunk.format, uint16_t);
    Fang_parse(data, format_chunk.channels, uint16_t);
    Fang_parse(data, format_chunk.sample_rate, uint32_t);
    Fang_parse(data, format_chunk.data_rate, uint32_t);
    Fang_parse(data, format_chunk.block_align, uint16_t);
    Fang_parse(data, format_chunk.bits_per_sample, uint16_t);

    if (!Fang_idcmp(format_chunk.chunk_id, "fmt "))
        goto Error_Invalid;

    switch (format_chunk.chunk_size)
    {
        case 16:
        case 18:
        case 40:
            break;

        default:
            goto Error_Invalid;
    }

    if (format_chunk.channels != 1)
        goto Error_Unsupported;

    if (format_chunk.sample_rate != 44100)
        goto Error_Unsupported;

    const uint16_t bytes_per_sample = format_chunk.bits_per_sample / 8;

    if (format_chunk.data_rate != format_chunk.sample_rate * bytes_per_sample)
        goto Error_Invalid;

    struct {
        uint32_t chunk_id;
        uint32_t chunk_size;
    } data_chunk;

    Fang_parse(data, data_chunk.chunk_id,   uint32_t);
    Fang_parse(data, data_chunk.chunk_size, uint32_t);

    if (!Fang_idcmp(data_chunk.chunk_id, "data"))
        goto Error_Invalid;

    // Ignore padding byte at end if present
    if (data_chunk.chunk_size % 2 != 0)
        data_chunk.chunk_size -= 1;

    result.samples = (int)data_chunk.chunk_size
                   / ((int)format_chunk.bits_per_sample / 8);

    assert(result.samples > 0);

    result.data = malloc(sizeof(float) * (size_t)result.samples);

    if (!result.data)
        goto Error_Allocation;

    /* Only uncompressed WAVs supported */
    if (format_chunk.format != 0x0001)
        goto Error_Unsupported;

    if (format_chunk.bits_per_sample != 16)
        goto Error_Invalid;

    {
        int16_t * in  = (int16_t*)data;
        float   * out = result.data;

        for (int i = 0; i < result.samples; ++i)
        {
            const int16_t sample = *(in++);

            *(out++) = ((float)sample - (float)INT16_MIN)
                     / ((float)INT16_MAX - (float)INT16_MIN);
        }
    }

    {
        const int fade_len = 500;

        assert(result.samples > fade_len);

        float * head = result.data;
        float * tail = &result.data[result.samples - 1];

        for (float fade = 0.0f; fade <= 1.0f; fade += 1.0f / (float)fade_len)
        {
            *(head++) *= fade;
            *(tail--) *= fade;
        }
    }

    return result;

Error_Invalid:
Error_Unsupported:
Error_Allocation:
    free(result.data);
    memset(&result, 0, sizeof(result));
    return result;
}

static Fang_Audio
Fang_LoadWAV(
    const char * const filepath)
{
    assert(filepath);

    Fang_Audio result = {.data = NULL};
    Fang_File  file   = {.data = NULL};

    if (Fang_LoadFile(filepath, &file) != 0)
        return result;

    result = Fang_ParseWAV(&file);
    assert(Fang_AudioValid(&result));
    Fang_FreeFile(&file);
    return result;
}
