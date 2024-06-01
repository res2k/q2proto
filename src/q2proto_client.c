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
#include "q2proto/q2proto_client.h"

#include "q2proto/q2proto_io.h"
#include "q2proto_internal.h"

#include <stdlib.h>

// Pick "best" accepted protocol from a comma-separated list of protocol versions. Ignore unsupported/invalid versions.
static void parse_challenge_protocol(q2proto_string_t protos_str, const q2proto_protocol_t *accepted_protocols, size_t num_accepted_protocols, q2proto_challenge_t *parsed_challenge)
{
    size_t best_protocol_idx = SIZE_MAX;
    q2proto_protocol_t best_protocol = Q2P_PROTOCOL_INVALID;

    q2proto_string_t proto_num_token;
    while(next_token(&proto_num_token, &protos_str, ','))
    {
        errno = 0;
        long proto_value = q2pstol(&proto_num_token, 10);
        if (errno != 0)
            continue;

        for (size_t i = 0; i < num_accepted_protocols; i++)
        {
            q2proto_protocol_t p = accepted_protocols[i];
            if(q2proto_get_protocol_netver(p) == proto_value)
            {
                if (i < best_protocol_idx)
                {
                    best_protocol_idx = i;
                    best_protocol = p;
                }
                break;
            }
        }
    }
    if (best_protocol != Q2P_PROTOCOL_INVALID)
        parsed_challenge->server_protocol = best_protocol;
}

q2proto_error_t q2proto_parse_challenge(const char *challenge_args, const q2proto_protocol_t *accepted_protocols, size_t num_accepted_protocols, q2proto_challenge_t *parsed_challenge)
{
    q2proto_string_t challenge_str = q2proto_make_string(challenge_args);

    // Parse challenge value
    q2proto_string_t challenge_value_token;
    if (!next_token(&challenge_value_token, &challenge_str, ' '))
        return Q2P_ERR_BAD_DATA;

    errno = 0;
    long challenge_value = q2pstol(&challenge_value_token, 10);
    if (errno != 0)
        return Q2P_ERR_BAD_DATA;

    parsed_challenge->challenge = (int32_t)challenge_value;
    parsed_challenge->server_protocol = Q2P_PROTOCOL_INVALID;

    // No "p=" args means vanilla protocol. See if it's in the list of accepted protocols.
    for (size_t i = 0; i < num_accepted_protocols; i++)
    {
        if (accepted_protocols[i] == Q2P_PROTOCOL_VANILLA)
        {
            parsed_challenge->server_protocol = Q2P_PROTOCOL_INVALID;
            break;
        }
    }

    // Parse challenge args
    q2proto_string_t challenge_arg;
    while (next_token(&challenge_arg, &challenge_str, ' '))
    {
        if(strncmp(challenge_arg.str, "p=", 2) == 0)
        {
            parse_challenge_protocol(q2ps_substr(&challenge_arg, 2), accepted_protocols, num_accepted_protocols, parsed_challenge);
        }
    }

    return parsed_challenge->server_protocol != Q2P_PROTOCOL_INVALID ? Q2P_ERR_SUCCESS : Q2P_ERR_NO_ACCEPTABLE_PROTOCOL;
}

static q2proto_error_t default_client_packet_parse(q2proto_clientcontext_t *context, uintptr_t io_arg, q2proto_svc_message_t *svc_message);

q2proto_error_t q2proto_init_clientcontext(q2proto_clientcontext_t* context)
{
    memset(context, 0, sizeof(*context));

    context->client_read = default_client_packet_parse;

    return Q2P_ERR_SUCCESS;
}

q2proto_error_t q2proto_client_read(q2proto_clientcontext_t *context, uintptr_t io_arg, q2proto_svc_message_t *svc_message)
{
    return context->client_read(context, io_arg, svc_message);
}

static MAYBE_UNUSED const char* server_cmd_string(int command)
{
    const char *str = q2proto_debug_common_svc_string(command);
    return str ? str : q2proto_va("%d", command);
}

static q2proto_error_t default_client_packet_parse(q2proto_clientcontext_t *context, uintptr_t io_arg, q2proto_svc_message_t *svc_message)
{
    memset(svc_message, 0, sizeof(*svc_message));

    size_t command_read = 0;
    const void *command_ptr = NULL;
    READ_CHECKED(client_read, io_arg, command_ptr, raw, 1, &command_read);
    if (command_read == 0)
        return Q2P_ERR_NO_MORE_INPUT;

    uint8_t command = *(const uint8_t*)command_ptr;
    SHOWNET(io_arg, 1, -1, "%s", server_cmd_string(command));
    if (command == svc_stufftext)
    {
        svc_message->type = Q2P_SVC_STUFFTEXT;
        q2proto_common_client_read_stufftext(io_arg, &svc_message->stufftext);
        return Q2P_ERR_SUCCESS;
    }
    else if (command != svc_serverdata)
        return HANDLE_ERROR(client_read, io_arg, Q2P_ERR_EXPECTED_SERVERDATA, "expected svc_serverdata, got %d", command);

    svc_message->type = Q2P_SVC_SERVERDATA;

    int32_t protocol;
    READ_CHECKED(client_read, io_arg, protocol, i32);

    svc_message->serverdata.protocol = protocol;
    switch (protocol)
    {
    case PROTOCOL_OLD_DEMO:
    case PROTOCOL_VANILLA:
        return q2proto_vanilla_continue_serverdata(context, io_arg, &svc_message->serverdata);
    }

    return HANDLE_ERROR(client_read, io_arg, Q2P_ERR_PROTOCOL_NOT_SUPPORTED, "protocol unsupported: %d", protocol);
}

uint32_t q2proto_client_pack_solid(q2proto_clientcontext_t *context, const q2proto_vec3_t mins, const q2proto_vec3_t maxs)
{
    return context->pack_solid(context, mins, maxs);
}

void q2proto_client_unpack_solid(q2proto_clientcontext_t *context, uint32_t solid, q2proto_vec3_t mins, q2proto_vec3_t maxs)
{
    context->unpack_solid(context, solid, mins, maxs);
}

q2proto_error_t q2proto_client_download_reset(q2proto_clientcontext_t *context)
{
    return Q2P_ERR_SUCCESS;
}