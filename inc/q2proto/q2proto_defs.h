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
 * Basic definitions
 */
#ifndef Q2PROTO_DEFS_H_
#define Q2PROTO_DEFS_H_

/**\name q2proto configuration
 * @{ */
/**\def Q2PROTO_CONFIG_PROVIDED
 * If this macros is defined, does not attempt to include a configuration header.
 * Useful when eg wrapping the q2proto sources.
 */
/**\def Q2PROTO_CONFIG_H
 * Can be changed to customize the location of the q2proto config header.
 */
#if !defined(Q2PROTO_CONFIG_PROVIDED)
    #if !defined(Q2PROTO_CONFIG_H)
        #define Q2PROTO_CONFIG_H "q2proto_config.h"
    #endif
    #include Q2PROTO_CONFIG_H
#endif

// Feature flags, used to assemble Q2PROTO_FEATURES_xxx values
#define Q2PROTO_FEATURE_FLAG_PLAYER_GUNSKIN               0x1  // player state has gunskin
#define Q2PROTO_FEATURE_FLAG_PLAYER_DAMAGE_BLEND          0x2  // player state has damage_blend
#define Q2PROTO_FEATURE_FLAG_PLAYER_FOG                   0x4  // player state has fog
#define Q2PROTO_FEATURE_FLAG_PLAYER_GUNRATE_VIEWHEIGHT    0x8  // player state has gunrate & viewheight
#define Q2PROTO_FEATURE_FLAG_ENTITY_LOOP_ALPHA_SCALE_FX64 0x10 // entity has sound loop, alpha, scale, 64 effect bits

/**\def Q2PROTO_FEATURES_VANILLA
 * Possible values for Q2PROTO_PLAYER_STATE_FEATURES, Q2PROTO_ENTITY_STATE_FEATURES
 * to indicate vanilla game features. */
#define Q2PROTO_FEATURES_VANILLA           0
/**\def Q2PROTO_FEATURES_Q2PRO_EXTENDED
 * Possible values for Q2PROTO_PLAYER_STATE_FEATURES, Q2PROTO_ENTITY_STATE_FEATURES
 * to indicate Q2PRO extended game features.
 * Additional player state fields:
 * - gunskin
 * Additional entity state fields:
 * - additional 32 effects bits
 * - alpha
 * - loop_attenuation
 * - loop_volume
 * - scale
 */
#define Q2PROTO_FEATURES_Q2PRO_EXTENDED    (Q2PROTO_FEATURE_FLAG_PLAYER_GUNSKIN | Q2PROTO_FEATURE_FLAG_ENTITY_LOOP_ALPHA_SCALE_FX64)
/**\def Q2PROTO_FEATURES_Q2PRO_EXTENDED
 * Possible values for Q2PROTO_PLAYER_STATE_FEATURES, Q2PROTO_ENTITY_STATE_FEATURES
 * to indicate Q2PRO extended game features.
 * Additional player state fields:
 * - gunskin
 * Additional entity state fields:
 * - additional 32 effects bits
 * - alpha
 * - loop_attenuation
 * - loop_volume
 * - scale
 */
#define Q2PROTO_FEATURES_Q2PRO_EXTENDED_V2 (Q2PROTO_FEATURES_Q2PRO_EXTENDED | Q2PROTO_FEATURE_FLAG_PLAYER_DAMAGE_BLEND | Q2PROTO_FEATURE_FLAG_PLAYER_FOG)
/**\def Q2PROTO_FEATURES_Q2PRO_EXTENDED_V2
 * Possible values for Q2PROTO_PLAYER_STATE_FEATURES, Q2PROTO_ENTITY_STATE_FEATURES
 * to indicate Q2PRO extended game V2 features.
 * Additional player state fields:
 * - damage_blend
 */
#define Q2PROTO_FEATURES_RERELEASE         ((Q2PROTO_FEATURES_Q2PRO_EXTENDED_V2 & ~Q2PROTO_FEATURE_FLAG_PLAYER_FOG) | Q2PROTO_FEATURE_FLAG_PLAYER_GUNRATE_VIEWHEIGHT)
/**\def Q2PROTO_FEATURES_RERELEASE
 * Possible values for Q2PROTO_PLAYER_STATE_FEATURES, Q2PROTO_ENTITY_STATE_FEATURES
 * to indicate rerelease game features.
 * Additional player state fields:
 * - gunrate
 * - pm_viewheight
 */
/**\def Q2PROTO_FEATURES_RERELEASE
 * Possible values for Q2PROTO_PLAYER_STATE_FEATURES, Q2PROTO_ENTITY_STATE_FEATURES
 * to indicate support for _all_ possible game features.
 * (Note that you'll need to account for protocol-/game-specific differences,
 * mainly how fog is handled in Q2PROTO_GAME_Q2PRO_EXTENDED_V2 vs Q2PROTO_GAME_RERELEASE.)
 */
#define Q2PROTO_FEATURES_ALL               ~0u
/**\def Q2PROTO_PLAYER_STATE_FEATURES
 * Features to be supported by player state structs. */
#if !defined(Q2PROTO_PLAYER_STATE_FEATURES)
    #define Q2PROTO_PLAYER_STATE_FEATURES Q2PROTO_FEATURES_VANILLA
#endif
/**\def Q2PROTO_ENTITY_STATE_FEATURES
 * Features to be supported by entity state structs. */
#if !defined(Q2PROTO_ENTITY_STATE_FEATURES)
    #define Q2PROTO_ENTITY_STATE_FEATURES Q2PROTO_FEATURES_VANILLA
#endif
/**\def Q2PROTO_RETURN_IO_ERROR_CODES
 * If defined to 1, errors from IO functions are checked and returned from calling functions.
 * Can be disabled if the IO functions never return in case of an error
 * (eg due to <tt>longjmp</tt>ing out in that case).
 * Defaults to 1.
 */
#if !defined(Q2PROTO_RETURN_IO_ERROR_CODES)
    #define Q2PROTO_RETURN_IO_ERROR_CODES 1
#endif
/**\def Q2PROTO_ERROR_FEEDBACK
 * If defined to 1, calls error feedback functions in case of errors, which some additional information.
 * Can be used to output that information and/or perform error handling such as <tt>longjmp</tt>ing out.
 * Defaults to 0.
 */
#if !defined(Q2PROTO_ERROR_FEEDBACK)
    #define Q2PROTO_ERROR_FEEDBACK 0
#endif
/**\def Q2PROTO_SHOWNET
 * If defined to 1, enabled 'shownet' feedback. Requires an externally defined function.
 * Defaults to 0.
 */
#if !defined(Q2PROTO_SHOWNET)
    #define Q2PROTO_SHOWNET 0
#endif
/**\def Q2PROTO_EXTERNALLY_PROVIDED_DECL
 * Declaration for "externally provided" functions.
 * Can be used to eg make these functions \c static, when wrapping everything into a single source.
 */
#if !defined(Q2PROTO_EXTERNALLY_PROVIDED_DECL)
    #define Q2PROTO_EXTERNALLY_PROVIDED_DECL extern
#endif
/**\def Q2PROTO_PUBLIC_API
 * Declaration for Q2PROTO public functions.
 * Can be used to eg make q2proto functions \c static, when including as a single source, multiple times,
 * or tweaking external visibility (\c dllimport, \c dllexport, \c "visibility" attribute and the likes).
 */
#if !defined(Q2PROTO_PUBLIC_API)
    #define Q2PROTO_PUBLIC_API extern
#endif
/**\def Q2PROTO_PRIVATE_API
 * Declaration for Q2PROTO private (internal) functions.
 * Can be used to eg make q2proto functions \c static, when including as a single source, multiple times,
 * or tweaking external visibility (\c dllimport, \c dllexport, \c "visibility" attribute and the likes).
 */
#if !defined(Q2PROTO_PRIVATE_API)
    #define Q2PROTO_PRIVATE_API
#endif
/**\def Q2PROTO_COMPRESSION_DEFLATE
 * Indicates deflate compression support is available and enables relevant q2proto functions,
 * but also requires provision of \c q2protoio_inflate_* and \c q2protoio_deflate_* functions.
 */
#if !defined(Q2PROTO_COMPRESSION_DEFLATE)
    #define Q2PROTO_COMPRESSION_DEFLATE 0
#endif
/** @} */

/* Macros to provide "hidden" struct members.
 * In order to make struct members "private", while exposing their individual members to the compiler
 * (to get alignment, sizes right), obscure the name of "private" struct members when included from
 * outside q2proto.
 * Inside q2proto (when Q2PROTO_BUILD is defined) the un-obscured names are visible. */
#if defined(Q2PROTO_BUILD)
    #define Q2PROTO_PRIVATE_API_MEMBER(NAME)             NAME
    #define Q2PROTO_PRIVATE_API_FUNC_PTR(RET, NAME, ...) RET (*NAME)(__VA_ARGS__)
#else
    #define _Q2PROTO_PRIVATE_API_MEMBER_CONCAT2(X, Y)    X##Y
    #define _Q2PROTO_PRIVATE_API_MEMBER_CONCAT(X, Y)     _Q2PROTO_PRIVATE_API_MEMBER_CONCAT2(X, Y)
    #define Q2PROTO_PRIVATE_API_MEMBER(NAME)             _Q2PROTO_PRIVATE_API_MEMBER_CONCAT(_private_, __LINE__)
    #define Q2PROTO_PRIVATE_API_FUNC_PTR(RET, NAME, ...) void *Q2PROTO_PRIVATE_API_MEMBER(NAME)
#endif

#endif // Q2PROTO_DEFS_H_
