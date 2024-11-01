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
 * Protocol enum & helpers
 */
#ifndef Q2PROTO_PROTOCOL_H_
#define Q2PROTO_PROTOCOL_H_

#include "q2proto_defs.h"

#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

/// Supported protocols
typedef enum q2proto_protocol_e {
    /// Invalid protocol
    Q2P_PROTOCOL_INVALID = 0,
    /// Protocol 26, used by original release demos
    Q2P_PROTOCOL_OLD_DEMO,
    /// Vanilla 3.20 protocol
    Q2P_PROTOCOL_VANILLA,
    /// R1Q2 protocol
    Q2P_PROTOCOL_R1Q2,
    /// Q2PRO protocol
    Q2P_PROTOCOL_Q2PRO,
    /// Q2PRO extended demo (not used for network communication)
    Q2P_PROTOCOL_Q2PRO_EXTENDED_DEMO,
    /// Q2PRO extended v2 demo (not used for network communication)
    Q2P_PROTOCOL_Q2PRO_EXTENDED_V2_DEMO,
    /// Q2PRO extended v2 demo w/ playerfog (not used for network communication)
    Q2P_PROTOCOL_Q2PRO_EXTENDED_DEMO_PLAYERFOG,
    /// Q2rePRO (Q2PRO fork w/ rerelease game support) protocol
    Q2P_PROTOCOL_Q2REPRO,
} q2proto_protocol_t;

/// Map from q2proto_protocol_t value to protocol version number communicated over network
Q2PROTO_PUBLIC_API int q2proto_get_protocol_netver(q2proto_protocol_t protocol);
/// Map from protocol version number communicated over network to q2proto_protocol_t value
Q2PROTO_PUBLIC_API q2proto_protocol_t q2proto_protocol_from_netver(int version);

/// Get array with protocols, suitable for vanilla games
Q2PROTO_PUBLIC_API const q2proto_protocol_t *q2proto_get_vanilla_protocols(void);
/// Get number of items in array returned by q2proto_get_vanilla_protocols
Q2PROTO_PUBLIC_API const size_t q2proto_get_num_vanilla_protocols(void);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // Q2PROTO_PROTOCOL_H_
