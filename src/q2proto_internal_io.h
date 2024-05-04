/*
Copyright (C) 1997-2001 Id Software, Inc.
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
 * I/O-related definitions & parsing helpers
 */
#ifndef Q2PROTO_INTERNAL_IO_H_
#define Q2PROTO_INTERNAL_IO_H_

#include "q2proto/q2proto.h"

#include "q2proto_internal_defs.h"

/**\def HANDLE_ERROR
 * Handle an error code
 */
#if Q2PROTO_ERROR_FEEDBACK
    #if defined(_MSC_VER)
        #define HANDLE_ERROR(SOURCE, IO_ARG, CODE, MSG, ...) q2protoerr_##SOURCE((IO_ARG), (CODE), (MSG), ##__VA_ARGS__)
    #else
        #define HANDLE_ERROR(SOURCE, IO_ARG, CODE, MSG, ...) q2protoerr_##SOURCE((IO_ARG), (CODE), (MSG) __VA_OPT__(,) __VA_ARGS__)
    #endif
#else
    #define HANDLE_ERROR(SOURCE, IO_ARG, CODE, MSG, ...) (CODE)
#endif
/**\def CHECK
 * Check return code of \c EXPR. Call HANDLE_ERROR and return error code in case of error.
 */
#define CHECKED(SOURCE, IO_ARG, EXPR)                                                     \
    do                                                                                    \
    {                                                                                     \
        q2proto_error_t err = (EXPR);                                                     \
        if (err != Q2P_ERR_SUCCESS)                                                       \
            return HANDLE_ERROR(SOURCE, (IO_ARG), err, "%s: failed %s", __func__, #EXPR); \
    } while (0)
/**\def GET_IO_ERROR
 * Get last error if Q2PROTO_RETURN_IO_ERROR_CODES is enabled.
 */
#if Q2PROTO_RETURN_IO_ERROR_CODES
    #define GET_IO_ERROR(IO_ARG)    (q2protoio_get_error((IO_ARG)))
#else
    #define GET_IO_ERROR(IO_ARG)    Q2P_ERR_SUCCESS
#endif
/**\def CHECKED_IO
 * Perform expression \c EXPR, check I/O error afterwards if Q2PROTO_RETURN_IO_ERROR_CODES is enabled.
 * This is only suitable for use with "externally defined" `q2protoio_` functions, as those are defined to
 * either set an error code returned by `q2protoio_get_error()`, or abort things with eg a `longjmp`.
 */
#if Q2PROTO_RETURN_IO_ERROR_CODES
    #define CHECKED_IO(SOURCE, IO_ARG, EXPR, DESCR)                                                       \
        do                                                                                                \
        {                                                                                                 \
            EXPR;                                                                                         \
            q2proto_error_t err = q2protoio_get_error((IO_ARG));                                          \
            if (err != Q2P_ERR_SUCCESS)                                                                   \
                return HANDLE_ERROR(SOURCE, (IO_ARG), err, "%s: failed to %s", __func__, DESCR);          \
        } while (0)
#else
    #define CHECKED_IO(SOURCE, IO_ARG, EXPR, DESCR) EXPR
#endif
/**\def READ_CHECKED
 * Read a value of type \c TYPE and assign it to \c TARGET, check I/O error afterwards if Q2PROTO_RETURN_IO_ERROR_CODES is enabled.
 * Additional arguments are passed on to read function.
 */
#if defined(_MSC_VER)
    #define READ_CHECKED(SOURCE, IO_ARG, TARGET, TYPE, ...) CHECKED_IO(SOURCE, IO_ARG, TARGET = q2protoio_read_##TYPE(IO_ARG, ##__VA_ARGS__), "read " #TARGET)
#else
    #define READ_CHECKED(SOURCE, IO_ARG, TARGET, TYPE, ...) CHECKED_IO(SOURCE, IO_ARG, TARGET = q2protoio_read_##TYPE(IO_ARG __VA_OPT__(, ) __VA_ARGS__), "read " #TARGET)
#endif
/**\def WRITE_CHECKED
 * Write \c EXPR of type \c TYPE, check I/O error afterwards if Q2PROTO_RETURN_IO_ERROR_CODES is enabled.
 * Additional arguments are passed on to write function.
 */
#if defined(_MSC_VER)
    #define WRITE_CHECKED(SOURCE, IO_ARG, TYPE, EXPR, ...) CHECKED_IO(SOURCE, IO_ARG, q2protoio_write_##TYPE(IO_ARG, EXPR, ##__VA_ARGS__), "write " #EXPR)
#else
    #define WRITE_CHECKED(SOURCE, IO_ARG, TYPE, EXPR, ...) CHECKED_IO(SOURCE, IO_ARG, q2protoio_write_##TYPE(IO_ARG, EXPR __VA_OPT__(, ) __VA_ARGS__), "write " #EXPR)
#endif

/**\name I/O convenience functions
 * @{ */
static inline int8_t q2protoio_read_i8(uintptr_t io_arg)
{
    return (int8_t)q2protoio_read_u8(io_arg);
}

static inline int16_t q2protoio_read_i16(uintptr_t io_arg)
{
    return (int16_t)q2protoio_read_u16(io_arg);
}

static inline int32_t q2protoio_read_i32(uintptr_t io_arg)
{
    return (int32_t)q2protoio_read_u32(io_arg);
}

static inline bool q2protoio_read_bool(uintptr_t io_arg)
{
    return q2protoio_read_u8(io_arg) != 0;
}

static inline int q2protoio_read_q2pro_i23(uintptr_t io_arg, bool *is_diff)
{
    int32_t c = q2protoio_read_i16(io_arg);
    if (GET_IO_ERROR(io_arg) != Q2P_ERR_SUCCESS)
        goto fail;
    if (is_diff)
        *is_diff = (c & 1) == 0;
    if (c & 1)
    {
        int32_t b = q2protoio_read_i8(io_arg);
        if (GET_IO_ERROR(io_arg) != Q2P_ERR_SUCCESS)
            goto fail;
        c = (c & 0xffff) | (b << 16);
        return c >> 1;
    }
    return c >> 1;

fail:
    if (is_diff)
        *is_diff = false;
    return -1;
}

static inline uint64_t q2protoio_read_var_u64(uintptr_t io_arg)
{
    int shift = 0;
    uint64_t v = 0;
    uint8_t b;
    do
    {
        b = q2protoio_read_u8(io_arg);
        if (GET_IO_ERROR(io_arg) != Q2P_ERR_SUCCESS)
            goto fail;
        v |= ((uint64_t)b & 0x7f) << shift;
        shift += 7;
    } while (b & 0x80);
    return v;

fail:
    return (uint64_t)-1;
}

/// Read a single component of a 16-bit encoded coordinate
#define READ_CHECKED_VAR_COORD_COMP_16(SOURCE, IO_ARG, TARGET, COMP) \
    do                                                               \
    {                                                                \
        int16_t coord;                                               \
        READ_CHECKED(SOURCE, (IO_ARG), coord, i16);                  \
        q2proto_var_coord_set_int_comp(TARGET, COMP, coord);         \
    } while (0)

static inline q2proto_error_t read_var_coord_short(uintptr_t io_arg, q2proto_var_coord_t* pos)
{
    READ_CHECKED_VAR_COORD_COMP_16(client_read, io_arg, pos, 0);
    READ_CHECKED_VAR_COORD_COMP_16(client_read, io_arg, pos, 1);
    READ_CHECKED_VAR_COORD_COMP_16(client_read, io_arg, pos, 2);
    return Q2P_ERR_SUCCESS;
}

/// Read a single component of a 16-bit coordinate (no encoding)
#define READ_CHECKED_VAR_COORD_COMP_16_UNSCALED(SOURCE, IO_ARG, TARGET, COMP) \
    do                                                                        \
    {                                                                         \
        int16_t coord;                                                        \
        READ_CHECKED(SOURCE, (IO_ARG), coord, i16);                           \
        q2proto_var_coord_set_short_unscaled_comp(TARGET, COMP, coord);       \
    } while (0)

/// Read a single component of a Q2PRO variably encoded coordinate
#define READ_CHECKED_VAR_COORD_COMP_Q2PRO_I23(SOURCE, IO_ARG, TARGET, COMP) \
    do                                                                      \
    {                                                                       \
        int coord;                                                          \
        READ_CHECKED(SOURCE, (IO_ARG), coord, q2pro_i23, NULL);             \
        q2proto_var_coord_set_int_comp(TARGET, COMP, coord);                \
    } while (0)

static inline q2proto_error_t read_var_coord_q2pro_i23(uintptr_t io_arg, q2proto_var_coord_t* pos)
{
    READ_CHECKED_VAR_COORD_COMP_Q2PRO_I23(client_read, io_arg, pos, 0);
    READ_CHECKED_VAR_COORD_COMP_Q2PRO_I23(client_read, io_arg, pos, 1);
    READ_CHECKED_VAR_COORD_COMP_Q2PRO_I23(client_read, io_arg, pos, 2);
    return Q2P_ERR_SUCCESS;
}

/// Read a single component of a 16-bit encoded angle
#define READ_CHECKED_VAR_ANGLE_COMP_16(SOURCE, IO_ARG, TARGET, COMP) \
    do                                                               \
    {                                                                \
        int16_t a;                                                   \
        READ_CHECKED(SOURCE, (IO_ARG), a, i16);                      \
        q2proto_var_angle_set_short_comp(TARGET, COMP, a);           \
    } while (0)

/// Read a single component of an 8-bit encoded angle
#define READ_CHECKED_VAR_ANGLE_COMP_8(SOURCE, IO_ARG, TARGET, COMP)  \
    do                                                               \
    {                                                                \
        int8_t a;                                                    \
        READ_CHECKED(SOURCE, (IO_ARG), a, i8);                       \
        q2proto_var_angle_set_char_comp(TARGET, COMP, a);            \
    } while (0)

static inline q2proto_error_t read_var_angle16(uintptr_t io_arg, q2proto_var_angle_t *angle)
{
    READ_CHECKED_VAR_ANGLE_COMP_16(client_read, io_arg, angle, 0);
    READ_CHECKED_VAR_ANGLE_COMP_16(client_read, io_arg, angle, 1);
    READ_CHECKED_VAR_ANGLE_COMP_16(client_read, io_arg, angle, 2);
    return Q2P_ERR_SUCCESS;
}

/// Read a single component of an 8-bit small offset
#define READ_CHECKED_VAR_SMALL_OFFSET_COMP(SOURCE, IO_ARG, TARGET, COMP) \
    do                                                                   \
    {                                                                    \
        int8_t o;                                                        \
        READ_CHECKED(SOURCE, (IO_ARG), o, i8);                           \
        q2proto_var_small_offset_set_char_comp(TARGET, COMP, o);         \
    } while (0)

static inline q2proto_error_t read_var_small_offset(uintptr_t io_arg, q2proto_var_small_offset_t* offs)
{
    READ_CHECKED_VAR_SMALL_OFFSET_COMP(client_read, io_arg, offs, 0);
    READ_CHECKED_VAR_SMALL_OFFSET_COMP(client_read, io_arg, offs, 1);
    READ_CHECKED_VAR_SMALL_OFFSET_COMP(client_read, io_arg, offs, 2);
    return Q2P_ERR_SUCCESS;
}

/// Read a single component of an 8-bit small angle
#define READ_CHECKED_VAR_SMALL_ANGLES_COMP(SOURCE, IO_ARG, TARGET, COMP) \
    do                                                                   \
    {                                                                    \
        int8_t a;                                                        \
        READ_CHECKED(SOURCE, (IO_ARG), a, i8);                           \
        q2proto_var_small_angle_set_char_comp(TARGET, COMP, a);          \
    } while (0)

static inline q2proto_error_t read_var_small_angles(uintptr_t io_arg, q2proto_var_small_angle_t* angle)
{
    READ_CHECKED_VAR_SMALL_ANGLES_COMP(client_read, io_arg, angle, 0);
    READ_CHECKED_VAR_SMALL_ANGLES_COMP(client_read, io_arg, angle, 1);
    READ_CHECKED_VAR_SMALL_ANGLES_COMP(client_read, io_arg, angle, 2);
    return Q2P_ERR_SUCCESS;
}

/// Read a single component of an 8-bit blend value
#define READ_CHECKED_VAR_BLEND_COMP(SOURCE, IO_ARG, TARGET, COMP) \
    do                                                            \
    {                                                             \
        int8_t c;                                                 \
        READ_CHECKED(SOURCE, (IO_ARG), c, u8);                    \
        q2proto_var_blend_set_byte_comp(TARGET, COMP, c);         \
    } while (0)

static inline q2proto_error_t read_var_blend(uintptr_t io_arg, q2proto_var_blend_t* blend)
{
    READ_CHECKED_VAR_BLEND_COMP(client_read, io_arg, blend, 0);
    READ_CHECKED_VAR_BLEND_COMP(client_read, io_arg, blend, 1);
    READ_CHECKED_VAR_BLEND_COMP(client_read, io_arg, blend, 2);
    READ_CHECKED_VAR_BLEND_COMP(client_read, io_arg, blend, 3);
    return Q2P_ERR_SUCCESS;
}

static inline q2proto_error_t read_short_coord(uintptr_t io_arg, float coord[3])
{
    for (int i = 0; i < 3; i++)
    {
        int16_t c;
        READ_CHECKED(client_read, io_arg, c, i16);
        coord[i] = _q2proto_valenc_int2coord(c);
    }
    return Q2P_ERR_SUCCESS;
}

static inline q2proto_error_t read_int23_coord(uintptr_t io_arg, float coord[3])
{
    for (int i = 0; i < 3; i++)
    {
        int32_t c;
        READ_CHECKED(client_read, io_arg, c, q2pro_i23, NULL);
        coord[i] = _q2proto_valenc_int2coord(c);
    }
    return Q2P_ERR_SUCCESS;
}

static inline void q2protoio_write_i8(uintptr_t io_arg, int8_t x)
{
    q2protoio_write_u8(io_arg, (uint8_t)x);
}

static inline void q2protoio_write_i16(uintptr_t io_arg, int16_t x)
{
    q2protoio_write_u16(io_arg, (uint16_t)x);
}

static inline void q2protoio_write_q2pro_i23(uintptr_t io_arg, int32_t x, int32_t prev)
{
    int delta = x - prev;
    if(delta >= -0x4000 && delta < 0x4000)
        q2protoio_write_u16(io_arg, (uint16_t)delta << 1);
    else
    {
        uint32_t write_val = (uint32_t)((x << 1) | 1);
        q2protoio_write_u8(io_arg, (uint8_t)(write_val & 0xff));
        q2protoio_write_u8(io_arg, (uint8_t)((write_val >> 8) & 0xff));
        q2protoio_write_u8(io_arg, (uint8_t)((write_val >> 16) & 0xff));
    }
}

static inline void q2protoio_write_i32(uintptr_t io_arg, int32_t x)
{
    q2protoio_write_u32(io_arg, (uint32_t)x);
}

static inline void q2protoio_write_var_u64(uintptr_t io_arg, uint64_t x)
{
    do
    {
        uint8_t b = x & 0x7f;
        x >>= 7;
        if (x != 0)
            b |= 0x80;
        q2protoio_write_u8(io_arg, b);
    } while (x != 0);
}

static inline void q2protoio_write_string(uintptr_t io_arg, const q2proto_string_t* str)
{
    char *p = (char*)q2protoio_write_reserve_raw(io_arg, str->len + 1);
    memcpy(p, str->str, str->len);
    p[str->len] = 0;
}

static inline void q2protoio_write_var_coord_short(uintptr_t io_arg, const q2proto_var_coord_t* pos)
{
    q2protoio_write_i16(io_arg, q2proto_var_coord_get_int_comp(pos, 0));
    q2protoio_write_i16(io_arg, q2proto_var_coord_get_int_comp(pos, 1));
    q2protoio_write_i16(io_arg, q2proto_var_coord_get_int_comp(pos, 2));
    /* Note: q2protoio_get_error() is defined to return the error from the "last I/O operation",
     * so it's theoretically possible that one q2protoio_write_coord_short() fails, but the
     * last one succeeds, keeping the "last error" as success...
     * In practice, however, it's probably reasonable to assume that, if one write fails,
     * the remaining writes will do so, as well. */
}

static inline void q2protoio_write_var_coord_q2pro_i23(uintptr_t io_arg, const q2proto_var_coord_t* pos)
{
    q2protoio_write_q2pro_i23(io_arg, q2proto_var_coord_get_int_comp(pos, 0), 0);
    q2protoio_write_q2pro_i23(io_arg, q2proto_var_coord_get_int_comp(pos, 1), 0);
    q2protoio_write_q2pro_i23(io_arg, q2proto_var_coord_get_int_comp(pos, 2), 0);
}

/** @} */

/**\name Parsing helper
 * @{ */
// helper to combine checking against a protocol flag (U_xxx, PS_xxx) and set the corresponding Q2P_xxx flag
static inline bool delta_bits_check(uint64_t bits, uint64_t check, uint32_t* delta_new, uint32_t new_bits)
{
    if (bits & check)
    {
        *delta_new |= new_bits;
        return true;
    }
    return false;
}
/** @} */

/// Check if the integer encodings of in the "write" part of a q2proto_maybe_diff_coord_t differ
static inline unsigned q2proto_maybe_diff_coord_write_differs_int(const q2proto_maybe_diff_coord_t *coord)
{
    unsigned bits = 0;
    if (q2proto_var_coord_get_int_comp(&coord->write.prev, 0) != q2proto_var_coord_get_int_comp(&coord->write.current, 0))
        bits |= BIT(0);
    if (q2proto_var_coord_get_int_comp(&coord->write.prev, 1) != q2proto_var_coord_get_int_comp(&coord->write.current, 1))
        bits |= BIT(1);
    if (q2proto_var_coord_get_int_comp(&coord->write.prev, 2) != q2proto_var_coord_get_int_comp(&coord->write.current, 2))
        bits |= BIT(2);
    return bits;
}

#endif // Q2PROTO_INTERNAL_IO_H_
