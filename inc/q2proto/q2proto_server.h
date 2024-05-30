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
#include "q2proto_gametype.h"
#include "q2proto_protocol.h"
#include "q2proto_string.h"

#include <stdbool.h>
#include <stdint.h>

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

/// Server information
typedef struct q2proto_server_info_s
{
    /// Type of game run by server
    q2proto_game_type_t game_type;
    /// Default packet length value, used for \c packet_length member of q2proto_connect_t.
    int default_packet_length;
} q2proto_server_info_t;

/// Connection information
typedef struct q2proto_connect_s {
    /// Protocol
    q2proto_protocol_t protocol;
    /// Protocol version
    int version;
    /// Port
    int qport;
    /// Challenge
    int32_t challenge;

    /// Initial user info
    q2proto_string_t userinfo;

    /// Maximum packet length
    int packet_length;
    /// zlib compression available?
    bool has_zlib;

    /// Q2PRO netchan type
    int q2pro_nctype;
} q2proto_connect_t;

/**
 * Parse the arguments to a "connect" command, sent by a connecting client to the server.
 * \param connect_args "connect" arguments string.
 * \param accepted_protocols Accepted protocols. Checks requested protocol against this list.
 * \param num_accepted_protocols Number of accepted protocols.
 * \param server_info Server info. Used for some default packet length and to restrict accepted protocols.
 * \param parsed_connect Parsed connect info.
 * \returns Error code.
 */
q2proto_error_t q2proto_parse_connect(const char *connect_args, const q2proto_protocol_t *accepted_protocols, size_t num_accepted_protocols, const q2proto_server_info_t *server_info, q2proto_connect_t *parsed_connect);
/** @} */

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // Q2PROTO_SERVER_H_
