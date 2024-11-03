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
    case Q2P_NUM_PROTOCOLS:
        break;
    case Q2P_PROTOCOL_OLD_DEMO:
        return PROTOCOL_OLD_DEMO;
    case Q2P_PROTOCOL_VANILLA:
        return PROTOCOL_VANILLA;
    case Q2P_PROTOCOL_R1Q2:
        return PROTOCOL_R1Q2;
    case Q2P_PROTOCOL_Q2PRO:
        return PROTOCOL_Q2PRO;
    case Q2P_PROTOCOL_Q2PRO_EXTENDED_DEMO:
        return PROTOCOL_Q2PRO_DEMO_EXT;
    case Q2P_PROTOCOL_Q2PRO_EXTENDED_V2_DEMO:
        return PROTOCOL_Q2PRO_DEMO_EXT_LIMITS_2;
    case Q2P_PROTOCOL_Q2PRO_EXTENDED_DEMO_PLAYERFOG:
        return PROTOCOL_Q2PRO_DEMO_EXT_PLAYERFOG;
    case Q2P_PROTOCOL_Q2REPRO:
        return PROTOCOL_Q2REPRO;
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
    case PROTOCOL_Q2PRO_DEMO_EXT:
        return Q2P_PROTOCOL_Q2PRO_EXTENDED_DEMO;
    case PROTOCOL_Q2PRO_DEMO_EXT_LIMITS_2:
        return Q2P_PROTOCOL_Q2PRO_EXTENDED_V2_DEMO;
    case PROTOCOL_Q2PRO_DEMO_EXT_PLAYERFOG:
        return Q2P_PROTOCOL_Q2PRO_EXTENDED_DEMO_PLAYERFOG;
    case PROTOCOL_Q2REPRO:
        return Q2P_PROTOCOL_Q2REPRO;
    }

    return Q2P_PROTOCOL_INVALID;
}

static const q2proto_protocol_t q2proto_vanilla_protocols_array[] = {Q2P_PROTOCOL_Q2PRO, Q2P_PROTOCOL_R1Q2, Q2P_PROTOCOL_VANILLA};

const q2proto_protocol_t *q2proto_get_vanilla_protocols(void)
{
    return q2proto_vanilla_protocols_array;
}

const size_t q2proto_get_num_vanilla_protocols(void)
{
    return sizeof(q2proto_vanilla_protocols_array) / sizeof(q2proto_vanilla_protocols_array[0]);
}
