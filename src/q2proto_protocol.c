/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2003-2011 Richard Stanway
Copyright (C) 2003-2024 Andrey Nazarov
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

#define Q2PROTO_BUILD
#include "q2proto/q2proto_protocol.h"

#include "q2proto_internal_protocol.h"

int q2proto_get_protocol_netver(q2proto_protocol_t protocol)
{
    switch(protocol)
    {
    case Q2P_PROTOCOL_INVALID:
        break;
    case Q2P_PROTOCOL_OLD_DEMO:
        return PROTOCOL_OLD_DEMO;
    case Q2P_PROTOCOL_VANILLA:
        return PROTOCOL_VANILLA;
    case Q2P_PROTOCOL_R1Q2:
        return PROTOCOL_R1Q2;
    case Q2P_PROTOCOL_Q2PRO:
        return PROTOCOL_Q2PRO;
    }

    return 0;
}

q2proto_protocol_t q2proto_protocol_from_netver(int version)
{
    switch(version)
    {
    case PROTOCOL_OLD_DEMO:
        return Q2P_PROTOCOL_OLD_DEMO;
    case PROTOCOL_VANILLA:
        return Q2P_PROTOCOL_VANILLA;
    case PROTOCOL_R1Q2:
        return Q2P_PROTOCOL_R1Q2;
    case PROTOCOL_Q2PRO:
        return Q2P_PROTOCOL_Q2PRO;
    }

    return Q2P_PROTOCOL_INVALID;
}

const q2proto_protocol_t q2proto_default_accepted_protocols[] = {Q2P_PROTOCOL_VANILLA};
const size_t q2proto_num_default_accepted_protocols = sizeof(q2proto_default_accepted_protocols) / sizeof(q2proto_default_accepted_protocols[0]);