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
 * Parses and loads a TGA file from memory.
 *
 * This function does not support the following TGA features:
 * - Bit depths other than 8, 24, or 32
 * - Indexed/color-mapped files
 * - Interleaving
 * - Black and white images with a depth higher than 8
 * - Images with an origin other than top-left
**/
static Fang_Image
Fang_TGALoad(
    Fang_File * const file)
{
    assert(file);

    const uint8_t * data = file->data;

    enum {
        TGA_IMAGE_NONE    =  0, /* Empty                           */
        TGA_IMAGE_MAP     =  1, /* Color-mapped                    */
        TGA_IMAGE_RGB     =  2, /* Unmapped color                  */
        TGA_IMAGE_GREY    =  3, /* Greyscale                       */
        TGA_IMAGE_RLEMAP  =  9, /* Run-Length encoded color-mapped */
        TGA_IMAGE_RLERGB  = 10, /* Run-Length encoded color        */
        TGA_IMAGE_RLEGREY = 11, /* Run-Length encoded greyscale    */

        TGA_MASK_INTERLEAVE = 0xC0,
        TGA_INTERLEAVE_NONE = 0x00,
        TGA_INTERLEAVE_2WAY = 0x40,
        TGA_INTERLEAVE_4WAY = 0x80,

        TGA_MASK_ORIGIN     = 0x30,
        TGA_ORIGIN_LEFT     = 0x00,
        TGA_ORIGIN_RIGHT    = 0x10,
        TGA_ORIGIN_LOWER    = 0x00,
        TGA_ORIGIN_UPPER    = 0x20,
    };

    struct {
        uint8_t  id_len;
        uint8_t  map_included;
        uint8_t  image_type;
        uint16_t map_origin;
        uint16_t map_length;
        uint8_t  map_depth;
        uint16_t image_x;
        uint16_t image_y;
        uint16_t image_width;
        uint16_t image_height;
        uint8_t  image_depth;
        uint8_t  image_descriptor;
    } header;

    memcpy(&header.id_len,           data, sizeof( uint8_t)); data += sizeof( uint8_t);
    memcpy(&header.map_included,     data, sizeof( uint8_t)); data += sizeof( uint8_t);
    memcpy(&header.image_type,       data, sizeof( uint8_t)); data += sizeof( uint8_t);
    memcpy(&header.map_origin,       data, sizeof(uint16_t)); data += sizeof(uint16_t);
    memcpy(&header.map_length,       data, sizeof(uint16_t)); data += sizeof(uint16_t);
    memcpy(&header.map_depth,        data, sizeof( uint8_t)); data += sizeof( uint8_t);
    memcpy(&header.image_x,          data, sizeof(uint16_t)); data += sizeof(uint16_t);
    memcpy(&header.image_y,          data, sizeof(uint16_t)); data += sizeof(uint16_t);
    memcpy(&header.image_width,      data, sizeof(uint16_t)); data += sizeof(uint16_t);
    memcpy(&header.image_height,     data, sizeof(uint16_t)); data += sizeof(uint16_t);
    memcpy(&header.image_depth,      data, sizeof( uint8_t)); data += sizeof( uint8_t);
    memcpy(&header.image_descriptor, data, sizeof( uint8_t)); data += sizeof( uint8_t);

    Fang_Image result = {.pixels = NULL};

    bool bw  = false,
         rle = false;

    switch (header.image_type)
    {
        case TGA_IMAGE_RLERGB: /* fallthrough */
            rle = true;
        case TGA_IMAGE_RGB:
            break;

        case TGA_IMAGE_RLEGREY: /* fallthrough */
            rle = bw = true;
        case TGA_IMAGE_GREY:
            if (header.image_depth != 8)
                goto Error_Unsupported;

            break;

        default:
            goto Error_Unsupported;
    }

    if (header.image_depth != 8
    &&  header.image_depth != 24
    &&  header.image_depth != 32)
        goto Error_Unsupported;

    if (header.map_included)
        goto Error_Unsupported;

    if ((header.image_descriptor & TGA_MASK_INTERLEAVE) != TGA_INTERLEAVE_NONE)
        goto Error_Unsupported;

    if (header.image_descriptor & TGA_ORIGIN_LOWER)
        goto Error_Unsupported;

    if (header.image_descriptor & TGA_ORIGIN_RIGHT)
        goto Error_Unsupported;

    result.width  = (int)header.image_width;
    result.height = (int)header.image_height;
    result.stride = (int)((header.image_depth + 7) >> 3);
    result.pitch  = result.stride * (int)header.image_width;
    result.pixels = malloc((size_t)(result.pitch * result.height));

    if (!result.pixels)
        goto Error_Allocation;

    /* Image ID and color map unused */
    data += header.id_len;
    data += header.map_length;

    if (rle)
    {
        uint8_t * pixel = calloc(1, (size_t)result.stride);

        if (!pixel)
            goto Error_Allocation;

        uint8_t * dest = result.pixels;

        int count = 0,
            rep   = 0;

        for (int h = 0; h < result.height; ++h)
        {
            int x = 0;
            while (true)
            {
                if (count)
                {
                    const int n = min(count, result.width - x);

                    memcpy(
                        dest + x * result.stride,
                        data,
                        (size_t)(n * result.stride)
                    );

                    data += n * result.stride;

                    count -= n;
                    x     += n;

                    if (x == result.width)
                        break;
                }
                else if (rep)
                {
                    int n = min(rep, result.width - x);
                    rep -= n;

                    while (n--)
                    {
                        memcpy(
                            dest + x * result.stride,
                            pixel,
                            (size_t)result.stride
                        );

                        x++;
                    }

                    if (x == result.width)
                        break;
                }

                const uint8_t val = *data++;
                if (val & 0x80)
                {
                    memcpy(pixel, data, (size_t)result.stride);

                    data += result.stride;
                    rep   = (val & 0x7F) + 1;
                }
                else
                {
                    count = val + 1;
                }
            }

            dest += result.pitch;
        }

        free(pixel);
    }
    else /* !rle */
    {
        uint8_t * dest = result.pixels;

        for (int h = 0; h < result.height; ++h)
        {
            memcpy(dest, data, (size_t)result.pitch);
            dest += result.pitch;
            data += result.pitch;
        }
    }

    // Swap from BGR(A) to RGB(A)
    if (result.stride == 3 || result.stride == 4)
    {
        for (int i = 0; i < result.pitch * result.height; i += result.stride)
        {
            uint8_t * const first = (result.pixels + i);
            uint8_t * const third = (result.pixels + i + 2);

            const uint8_t temp = *first;
            *first = *third;
            *third = temp;
        }
    }

    return result;

Error_Allocation:
Error_Unsupported:
    result.width  = 0;
    result.height = 0;
    result.stride = 0;
    result.pitch  = 0;

    free(result.pixels);
    result.pixels = NULL;

    return result;
}
