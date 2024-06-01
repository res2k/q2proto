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
 * Types and functions for dealing with coordinates and angles
 */
#ifndef Q2PROTO_COORDS_H_
#define Q2PROTO_COORDS_H_

#include "q2proto_defs.h"

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

/// Simple three-component float vector
typedef float q2proto_vec3_t[3];

/* Generate declarations & (some) definitions for setter & getter functions
 * for variant coordinates.
 *
 * Declares:
 * - Component setter: `q2proto_<VEC_TYPE>_set_<TYPE_NAME>_comp`
 * - Component getter: `<TYPE_TYPE> q2proto_<VEC_TYPE>_get_<TYPE_NAME>_comp`
 * Declares & defines:
 * - Full setter: `q2proto_<VEC_TYPE>_set_<TYPE_NAME>`
 * - Full getter: `q2proto_<VEC_TYPE>_get_<TYPE_NAME>`
 */
#define _GENERATE_VARIANT_FUNCTIONS(VEC_TYPE, TYPE_NAME, TYPE_TYPE, NUM_COMPS)                                           \
    void q2proto_##VEC_TYPE##_set_##TYPE_NAME##_comp(q2proto_##VEC_TYPE##_t *coord, int comp, TYPE_TYPE x);              \
    TYPE_TYPE q2proto_##VEC_TYPE##_get_##TYPE_NAME##_comp(const q2proto_##VEC_TYPE##_t *coord, int comp);                \
    static inline void q2proto_##VEC_TYPE##_set_##TYPE_NAME(q2proto_##VEC_TYPE##_t *vec, const TYPE_TYPE in[NUM_COMPS])  \
    {                                                                                                                    \
        for (int c = 0; c < NUM_COMPS; c++)                                                                              \
            q2proto_##VEC_TYPE##_set_##TYPE_NAME##_comp(vec, c, in[c]);                                                  \
    }                                                                                                                    \
    static inline void q2proto_##VEC_TYPE##_get_##TYPE_NAME(const q2proto_##VEC_TYPE##_t *vec, TYPE_TYPE out[NUM_COMPS]) \
    {                                                                                                                    \
        for (int c = 0; c < NUM_COMPS; c++)                                                                              \
            out[c] = q2proto_##VEC_TYPE##_get_##TYPE_NAME##_comp(vec, c);                                                \
    }

/**\name Float/short variants
 * Coordinates and angles may be transmitted/stored as either float or short values.
 * These variant types retain their original representation, but can convert as
 * necessary.
 * \remark Try to work with <tt>float</tt>s as much as you can.
 * While automatic conversion from \c float to \c short is supported, it's lossy!
 * @{ */
/// 'Variant coordinate', storing a coordinate as either float, or encoded into 16 bit
typedef struct q2proto_var_coord_s {
    // Stores types of components
    uint8_t Q2PROTO_PRIVATE_MEMBER(float_bits);
    // Used by coord_delta functions to store set components
    uint8_t Q2PROTO_PRIVATE_MEMBER(delta_bits_space);
    // Component values
    union
    {
        float f;
        int32_t i;
    } Q2PROTO_PRIVATE_MEMBER(comps)[3];
} q2proto_var_coord_t;


/** 'Variant coordinate' functions for float values
 * @{ */
_GENERATE_VARIANT_FUNCTIONS(var_coord, float, float, 3)
/** @}  */
/** 'Variant coordinate' functions for pre-encoded integer values
 * @{ */
_GENERATE_VARIANT_FUNCTIONS(var_coord, int, int32_t, 3)
/** @}  */
/** 'Variant coordinate' functions for pre-encoded short values
 * @{ */
_GENERATE_VARIANT_FUNCTIONS(var_coord, short, int16_t, 3)
/** @}  */
/** 'Variant coordinate' functions for integer values (no encoding)
 * @{ */
_GENERATE_VARIANT_FUNCTIONS(var_coord, int_unscaled, int32_t, 3)
/** @}  */
/** 'Variant coordinate' functions for short values (no encoding)
 * @{ */
_GENERATE_VARIANT_FUNCTIONS(var_coord, short_unscaled, int16_t, 3)
/** @}  */

/// 'Variant angle', storing an angle as either float, or encoded into 16 bit
typedef struct q2proto_var_angle_s {
    // Stores types of components
    uint8_t Q2PROTO_PRIVATE_MEMBER(float_bits);
    // Used by coord_delta functions to store set components
    uint8_t Q2PROTO_PRIVATE_MEMBER(delta_bits_space);
    // Component values
    union
    {
        float f;
        int16_t s;
        int8_t c;
    } Q2PROTO_PRIVATE_MEMBER(comps)[3];
} q2proto_var_angle_t;

/** 'Variant angle' functions for float values
 * @{ */
_GENERATE_VARIANT_FUNCTIONS(var_angle, float, float, 3)
/** @}  */
/** 'Variant angle' functions for pre-encoded 16-bit values
 * @{ */
_GENERATE_VARIANT_FUNCTIONS(var_angle, short, int16_t, 3)
/** @}  */
/** 'Variant angle' functions for pre-encoded 8-bit values
 * @{ */
_GENERATE_VARIANT_FUNCTIONS(var_angle, char, int8_t, 3)
/** @}  */

/// Variant for "small" offsets with limited range and precision (viewoffset, gunoffset), can be encoded into 8 bit
typedef struct q2proto_var_small_offset_s {
    // Stores types of components
    uint8_t Q2PROTO_PRIVATE_MEMBER(type_bits);
    // Component values
    union
    {
        float f;
        int8_t c;
        int16_t s;
    } Q2PROTO_PRIVATE_MEMBER(comps)[3];
} q2proto_var_small_offset_t;

/** 'Small offset' functions for float values
 * @{ */
_GENERATE_VARIANT_FUNCTIONS(var_small_offset, float, float, 3)
/** @}  */
/** 'Small offset' functions for pre-encoded 8-bit values
 * @{ */
_GENERATE_VARIANT_FUNCTIONS(var_small_offset, char, int8_t, 3)
/** @}  */

/// Variant for "small" angles with limited range and precision (kick_angles, gunangles), can be encoded into 8 bit
typedef struct q2proto_var_small_angle_s {
    // Stores types of components
    uint8_t Q2PROTO_PRIVATE_MEMBER(type_bits);
    // Component values
    union
    {
        float f;
        int8_t c;
        int16_t s;
    } Q2PROTO_PRIVATE_MEMBER(comps)[3];
} q2proto_var_small_angle_t;

/** 'Small angle' functions for float values
 * @{ */
_GENERATE_VARIANT_FUNCTIONS(var_small_angle, float, float, 3)
/** @}  */
/** 'Small angle' functions for pre-encoded 8-bit values
 * @{ */
_GENERATE_VARIANT_FUNCTIONS(var_small_angle, char, int8_t, 3)
/** @}  */

/// Variant for "blend" values (RGBA, each component stored as a float or as a byte)
typedef struct q2proto_var_blend_s {
    // Stores types of components
    uint8_t Q2PROTO_PRIVATE_MEMBER(float_bits);
    // Used by blend_delta functions to store set components
    uint8_t Q2PROTO_PRIVATE_MEMBER(delta_bits_space);
    // Component values
    union
    {
        float f;
        uint8_t c;
    } Q2PROTO_PRIVATE_MEMBER(comps)[4];
} q2proto_var_blend_t;

/** 'Blend' functions for float values
 * @{ */
_GENERATE_VARIANT_FUNCTIONS(var_blend, float, float, 4)
/** @}  */
/** 'Blend' functions for byte values
 * @{ */
_GENERATE_VARIANT_FUNCTIONS(var_blend, byte, uint8_t, 4)
/** @}  */

/**\name Delta coordinates & angles
 * They store coordinates or angles in different types, using q2proto_var_coord_t and
 * q2proto_angle_delta_t, but also provide access to extra delta bits indicating
 * which components actually changed.
 * @{ */
/// Delta coordinate type
typedef struct q2proto_coord_delta_s {
    union {
        /// Actual coordinate values
        q2proto_var_coord_t values;
        struct {
            uint8_t Q2PROTO_PRIVATE_MEMBER(rsvd0); // float_bits
            uint8_t delta_bits;
            float Q2PROTO_PRIVATE_MEMBER(rsvd1)[3]; // comps
        };
    };
} q2proto_coord_delta_t;

/**\def Q2PROTO_SET_COORD_DELTA
 * Fills \c COORD_DELTA with values from \c TO and determines delta bits by comparing
 * with \c FROM.
 */
#define Q2PROTO_SET_COORD_DELTA(COORD_DELTA, TO, FROM, COORD_TYPE)           \
    do                                                                       \
    {                                                                        \
        (COORD_DELTA).delta_bits = 0;                                        \
        if ((TO)[0] != (FROM)[0])                                            \
            (COORD_DELTA).delta_bits |= BIT(0);                              \
        if ((TO)[1] != (FROM)[1])                                            \
            (COORD_DELTA).delta_bits |= BIT(1);                              \
        if ((TO)[2] != (FROM)[2])                                            \
            (COORD_DELTA).delta_bits |= BIT(2);                              \
        if ((COORD_DELTA).delta_bits != 0)                                   \
            q2proto_var_coord_set_##COORD_TYPE(&(COORD_DELTA).values, (TO)); \
    } while (0)

/**\def Q2PROTO_APPLY_COORD_DELTA
 * Change all components of \c TO with values from \c COORD_DELTA according to set delta bits.
 */
#define Q2PROTO_APPLY_COORD_DELTA(TO, COORD_DELTA, COORD_TYPE)                             \
    do                                                                                     \
    {                                                                                      \
        if ((COORD_DELTA).delta_bits & BIT(0))                                             \
            (TO)[0] = q2proto_var_coord_get_##COORD_TYPE##_comp(&(COORD_DELTA).values, 0); \
        if ((COORD_DELTA).delta_bits & BIT(1))                                             \
            (TO)[1] = q2proto_var_coord_get_##COORD_TYPE##_comp(&(COORD_DELTA).values, 1); \
        if ((COORD_DELTA).delta_bits & BIT(2))                                             \
            (TO)[2] = q2proto_var_coord_get_##COORD_TYPE##_comp(&(COORD_DELTA).values, 2); \
    } while (0)

/// Delta angle type
typedef struct q2proto_angle_delta_s {
    union {
        /// Actual angle values
        q2proto_var_angle_t values;
        struct {
            uint8_t Q2PROTO_PRIVATE_MEMBER(rsvd0); // float_bits
            uint8_t delta_bits;
            float Q2PROTO_PRIVATE_MEMBER(rsvd1)[3]; // comps
        };
    };
} q2proto_angle_delta_t;

/**\def Q2PROTO_SET_ANGLE_DELTA
 * Fills \c ANGLE_DELTA with values from \c TO and determines delta bits by comparing
 * with \c FROM.
 */
#define Q2PROTO_SET_ANGLE_DELTA(ANGLE_DELTA, TO, FROM, ANGLE_TYPE)           \
    do                                                                       \
    {                                                                        \
        (ANGLE_DELTA).delta_bits = 0;                                        \
        if ((TO)[0] != (FROM)[0])                                            \
            (ANGLE_DELTA).delta_bits |= BIT(0);                              \
        if ((TO)[1] != (FROM)[1])                                            \
            (ANGLE_DELTA).delta_bits |= BIT(1);                              \
        if ((TO)[2] != (FROM)[2])                                            \
            (ANGLE_DELTA).delta_bits |= BIT(2);                              \
        if ((ANGLE_DELTA).delta_bits != 0)                                   \
            q2proto_var_angle_set_##ANGLE_TYPE(&(ANGLE_DELTA).values, (TO)); \
    } while (0)

/**\def Q2PROTO_APPLY_ANGLE_DELTA
 * Change all components of \c TO with values from \c ANGLE_DELTA according to set delta bits.
 */
#define Q2PROTO_APPLY_ANGLE_DELTA(TO, ANGLE_DELTA, ANGLE_TYPE)                             \
    do                                                                                     \
    {                                                                                      \
        if ((ANGLE_DELTA).delta_bits & BIT(0))                                             \
            (TO)[0] = q2proto_var_angle_get_##ANGLE_TYPE##_comp(&(ANGLE_DELTA).values, 0); \
        if ((ANGLE_DELTA).delta_bits & BIT(1))                                             \
            (TO)[1] = q2proto_var_angle_get_##ANGLE_TYPE##_comp(&(ANGLE_DELTA).values, 1); \
        if ((ANGLE_DELTA).delta_bits & BIT(2))                                             \
            (TO)[2] = q2proto_var_angle_get_##ANGLE_TYPE##_comp(&(ANGLE_DELTA).values, 2); \
    } while (0)
/** @} */

/// Delta blend type
typedef struct q2proto_blend_delta_s {
    union {
        /// Actual coordinate values
        q2proto_var_blend_t values;
        struct {
            uint8_t Q2PROTO_PRIVATE_MEMBER(rsvd0); // float_bits
            uint8_t delta_bits;
            float Q2PROTO_PRIVATE_MEMBER(rsvd1)[4]; // comps
        };
    };
} q2proto_blend_delta_t;

/**
 * Coordinates that may be transferred as a difference to another value (depending on protocol).
 * The contents differ by use; the read member is used when reading coordinates, the write member
 * is used when writing coordinates.
 */
typedef struct q2proto_maybe_diff_coord_s {
    union
    {
        /// Coordinate that was read
        struct {
            /**
             * Bit mask, indicating which components are differences. If bit N is set, component N is a difference;
             * otherwise, it's an absolute value.
             */
            uint8_t diff_bits;
            /// Difference or absolute values. Note that not each component may have a value in all cases.
            q2proto_coord_delta_t value;
        } read;
        /// Coordinate to write.
        struct {
            /// "Previous" value of coordinate to write. May be used to write a difference value.
            q2proto_var_coord_t prev;
            /// "Current" value of coordinate to write.
            q2proto_var_coord_t current;
        } write;
    };
} q2proto_maybe_diff_coord_t;

//@{
/// Apply the "read" part maybe_diff to coord
static inline void q2proto_maybe_read_diff_apply_float(const q2proto_maybe_diff_coord_t *maybe_diff, q2proto_vec3_t coord)
{
    for (int i = 0; i < 3; i++)
    {
        if (!(maybe_diff->read.value.delta_bits & (1 << i))) continue;
        float v = q2proto_var_coord_get_float_comp(&maybe_diff->read.value.values, i);
        if (maybe_diff->read.diff_bits & (1 << i))
            coord[i] += v;
        else
            coord[i] = v;
    }
}
static inline void q2proto_maybe_read_diff_apply_int(const q2proto_maybe_diff_coord_t *maybe_diff, int32_t coord[3])
{
    for (int i = 0; i < 3; i++)
    {
        if (!(maybe_diff->read.value.delta_bits & (1 << i))) continue;
        int32_t v = q2proto_var_coord_get_int_comp(&maybe_diff->read.value.values, i);
        if (maybe_diff->read.diff_bits & (1 << i))
            coord[i] += v;
        else
            coord[i] = v;
    }
}
static inline void q2proto_maybe_read_diff_apply_short(const q2proto_maybe_diff_coord_t *maybe_diff, int16_t coord[3])
{
    for (int i = 0; i < 3; i++)
    {
        if (!(maybe_diff->read.value.delta_bits & (1 << i))) continue;
        int16_t v = q2proto_var_coord_get_short_comp(&maybe_diff->read.value.values, i);
        if (maybe_diff->read.diff_bits & (1 << i))
            coord[i] += v;
        else
            coord[i] = v;
    }
}
//@}

#undef _GENERATE_VARIANT_FUNCTIONS

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // Q2PROTO_COORDS_H_