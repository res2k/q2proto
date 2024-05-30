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
 * Server-side functions
 */
#ifndef Q2PROTO_SERVER_H_
#define Q2PROTO_SERVER_H_

#include "q2proto_defs.h"
#include "q2proto_error.h"
#include "q2proto_protocol.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**\name Server-side communications
 * @{ */
/**
 * Produce extra arguments for server "challenge".
 * (Currently the list of supported protocols, per the \a accepted_protocols argument.)
 * \param buf Buffer to receive extra arguments
 * \param buf_size Size of extra arguments buffer
 * \param accepted_protocols Array of accepted protocols, for inclusion in extra arguments
 * \param num_accepted_protocols Size of \a accepted_protocols
 * \returns Error code
 */
q2proto_error_t q2proto_get_challenge_extras(char *buf, size_t buf_size, const q2proto_protocol_t *accepted_protocols, size_t num_accepted_protocols);
/** @} */

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // Q2PROTO_SERVER_H_
