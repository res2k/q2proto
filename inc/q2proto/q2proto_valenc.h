/*
Copyright (C) 2024 Frank Richter

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/**\file
 * Internal value (coordinate etc) encoding and decoding.
 * (Some of these functions are used by the packing include files, hence they must be visible to
 * user code.)
 */
#ifndef Q2PROTO_VALENC_H_
#define Q2PROTO_VALENC_H_

#include "q2proto_defs.h"

#if defined(__cplusplus)
extern "C" {
#endif

static inline int _q2proto_valenc_clamped_mul(float x, int scale, int min, int max)
{
    x *= scale;
    x = x < min ? min : x;
    x = x > max ? max : x;
    return (int)x;
}

// Decode coordinate from integer
static inline float _q2proto_valenc_int2coord(int32_t x)
{
    return x * 0.125f;
}

// Encode coordinate to integer
static inline int32_t _q2proto_valenc_coord2int(float x)
{
    return _q2proto_valenc_clamped_mul(x, 8, INT32_MIN, INT32_MAX);
}

// Decode angle (in degrees) from 16-bit integer
static inline float _q2proto_valenc_short2angle(int16_t x)
{
    return x * (360.0f / 65536);
}

// Encode angle (in degrees) to 16-bit integer
static inline int16_t _q2proto_valenc_angle2short(float x)
{
    return (int)(x * 65536 / 360) & 65535;
}

// Decode angle (in degrees) from 8-bit integer
static inline float _q2proto_valenc_char2angle(int8_t x)
{
    return x * (360.0f / 256);
}

// Encode angle (in degrees) to 8-bit integer
static inline int8_t _q2proto_valenc_angle2char(float x)
{
    return (int)(x * 256 / 360) & 255;
}

// Decode a "small" coordinate from 8-bit integer
static inline float _q2proto_valenc_char2smalloffset(int8_t x)
{
    return x * 0.25f;
}

// Encode a "small" coordinate (-32...31.75) to 8-bit integer
static inline int8_t _q2proto_valenc_smalloffset2char(float x)
{
    return _q2proto_valenc_clamped_mul(x, 4, INT8_MIN, INT8_MAX);
}

// Decode a "small" angle from 8-bit integer
static inline float _q2proto_valenc_char2smallangle(int8_t x)
{
    return x * 0.25f;
}

// Encode a "small" angle (-32...31.75 degrees) to 8-bit integer
static inline int8_t _q2proto_valenc_smallangle2char(float x)
{
    return _q2proto_valenc_clamped_mul(x, 4, INT8_MIN, INT8_MAX);
}

// Decode a blend component from unsigned 8-bit integer
static inline float _q2proto_valenc_byte2blend(uint8_t x)
{
    return x / 255.f;
}

// Encode a blend component (0...1) to unsigned 8-bit integer
static inline uint8_t _q2proto_valenc_blend2byte(float x)
{
    return _q2proto_valenc_clamped_mul(x, 255, 0, UINT8_MAX);
}

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // Q2PROTO_VALENC_H_