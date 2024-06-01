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
/**\def Q2PROTO_RETURN_IO_ERROR_CODES
 * If defined to 1, errors from IO functions are checked and returned from calling functions.
 * Can be disabled if the IO functions never return in case of an error
 * (eg due to <tt>longjmp</tt>ing out in that case).
 * Defaults to 1.
 */
#if !defined(Q2PROTO_RETURN_IO_ERROR_CODES)
    #define Q2PROTO_RETURN_IO_ERROR_CODES  1
#endif
/**\def Q2PROTO_ERROR_FEEDBACK
 * If defined to 1, calls error feedback functions in case of errors, which some additional information.
 * Can be used to output that information and/or perform error handling such as <tt>longjmp</tt>ing out.
 * Defaults to 0.
 */
#if !defined(Q2PROTO_ERROR_FEEDBACK)
    #define Q2PROTO_ERROR_FEEDBACK  0
#endif
/**\def Q2PROTO_SHOWNET
 * If defined to 1, enabled 'shownet' feedback. Requires an externally defined function.
 * Defaults to 0.
 */
#if !defined(Q2PROTO_SHOWNET)
    #define Q2PROTO_SHOWNET  0
#endif
/**\def Q2PROTO_EXTERNALLY_PROVIDED_DECL
 * Declaration for "externally provided" functions.
 * Can be used to eg make these functions \c static, when wrapping everything into a single source.
 */
#if !defined(Q2PROTO_EXTERNALLY_PROVIDED_DECL)
    #define Q2PROTO_EXTERNALLY_PROVIDED_DECL   extern
#endif
/** @} */

/* Macros to provide "hidden" struct members.
 * In order to make struct members "private", while exposing their individual members to the compiler
 * (to get alignment, sizes right), obscure the name of "private" struct members when included from
 * outside q2proto.
 * Inside q2proto (when Q2PROTO_BUILD is defined) the un-obscured names are visible. */
#if defined(Q2PROTO_BUILD)
#define Q2PROTO_PRIVATE_MEMBER(NAME)                NAME
#define Q2PROTO_PRIVATE_FUNC_PTR(RET, NAME, ...)    RET (*NAME)(__VA_ARGS__)
#else
#define _Q2PROTO_PRIVATE_MEMBER_CONCAT2(X, Y)       X ## Y
#define _Q2PROTO_PRIVATE_MEMBER_CONCAT(X, Y)        _Q2PROTO_PRIVATE_MEMBER_CONCAT2(X, Y)
#define Q2PROTO_PRIVATE_MEMBER(NAME)                _Q2PROTO_PRIVATE_MEMBER_CONCAT(_private_, __LINE__)
#define Q2PROTO_PRIVATE_FUNC_PTR(RET, NAME, ...)    void* Q2PROTO_PRIVATE_MEMBER(NAME)
#endif

#endif // Q2PROTO_DEFS_H_