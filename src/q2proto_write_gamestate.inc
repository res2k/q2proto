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
 * write_gamestate function implementation
 */
#ifndef Q2PROTO_BUILD
#define Q2PROTO_BUILD
#endif // Q2PROTO_BUILD
#include "q2proto/q2proto.h"

static q2proto_error_t WRITE_GAMESTATE_FUNCTION_NAME(q2proto_servercontext_t *context, uintptr_t io_arg, const q2proto_gamestate_t *gamestate)
{
    // Write configstrings
    while(context->gamestate_pos < gamestate->num_configstrings)
    {
        const q2proto_svc_configstring_t *cfgstr = gamestate->configstrings + context->gamestate_pos;
        size_t configstring_size = 1 /* command */ + 2 /* index */ + cfgstr->value.len + 1 /* string */;
        if(q2protoio_write_available(io_arg) < configstring_size)
            return Q2P_ERR_NOT_ENOUGH_PACKET_SPACE;

        q2proto_common_server_write_configstring(io_arg, cfgstr);
        context->gamestate_pos++;
    }

    // Write spawn baselines
    size_t baseline_num;
    while((baseline_num = context->gamestate_pos - gamestate->num_configstrings) < gamestate->num_spawnbaselines)
    {
        const q2proto_svc_spawnbaseline_t *baseline = gamestate->spawnbaselines + baseline_num;
        if(q2protoio_write_available(io_arg) < WRITE_GAMESTATE_BASELINE_SIZE)
            return Q2P_ERR_NOT_ENOUGH_PACKET_SPACE;

        WRITE_GAMESTATE_BASELINE(context, io_arg, baseline);
        context->gamestate_pos++;
    }

    // Game state written successfully, reset state
    context->gamestate_pos = 0;

    return Q2P_ERR_SUCCESS;
}
