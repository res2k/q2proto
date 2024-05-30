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
#include "q2proto/q2proto_server.h"

#include "q2proto_internal.h"

static int compare_ints(const void *a, const void *b)
{
    int arg1 = *(const int *)a;
    int arg2 = *(const int *)b;

    return (arg1 > arg2) - (arg1 < arg2);
}

q2proto_error_t q2proto_get_challenge_extras(char *buf, size_t buf_size, const q2proto_protocol_t *accepted_protocols, size_t num_accepted_protocols)
{
    if(!buf || !buf_size)
        return Q2P_ERR_INVALID_ARGUMENT;

    if (!num_accepted_protocols)
    {
        // nothing to do...
        buf[0] = 0;
        return Q2P_ERR_SUCCESS;
    }

    /* Sort protocol versions, lowest to highest.
     * (Not sure it's actually required, but it's the traditional way) */
    int *protocol_vers = (int *)alloca(num_accepted_protocols * sizeof(int));
    for (size_t i = 0; i < num_accepted_protocols; i++)
    {
        protocol_vers[i] = q2proto_get_protocol_netver(accepted_protocols[i]);
    }
    qsort(protocol_vers, num_accepted_protocols, sizeof(int), compare_ints);

    q2proto_snprintf_update(&buf, &buf_size, "p=%d", protocol_vers[0]);
    for (size_t i = 1; i < num_accepted_protocols; i++)
    {
        q2proto_snprintf_update(&buf, &buf_size, ",%d", protocol_vers[i]);
    }
    return Q2P_ERR_SUCCESS;
}
