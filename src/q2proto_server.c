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

static q2proto_error_t next_connect_int(q2proto_string_t *connect_str, long *result)
{
    q2proto_string_t value_token;
    if (!next_token(&value_token, connect_str, ' '))
        return Q2P_ERR_BAD_DATA;

    errno = 0;
    *result = q2pstol(&value_token, 10);
    if (errno != 0)
        return Q2P_ERR_BAD_DATA;

    return Q2P_ERR_SUCCESS;
}

static q2proto_error_t parse_connect_r1q2(q2proto_string_t* connect_str, q2proto_connect_t *parsed_connect)
{
    // set minor protocol version
    q2proto_string_t protocol_ver_token = {0};
    next_token(&protocol_ver_token, connect_str, ' ');
    if (protocol_ver_token.len > 0) {
        parsed_connect->version = q2pstol(&protocol_ver_token, 10);
        if (parsed_connect->version < PROTOCOL_VERSION_R1Q2_MINIMUM)
            parsed_connect->version = PROTOCOL_VERSION_R1Q2_MINIMUM;
        else if (parsed_connect->version > PROTOCOL_VERSION_R1Q2_CURRENT)
            parsed_connect->version = PROTOCOL_VERSION_R1Q2_CURRENT;
    } else {
        parsed_connect->version = PROTOCOL_VERSION_R1Q2_MINIMUM;
    }
    parsed_connect->has_zlib = true;

    return Q2P_ERR_SUCCESS;
}

static q2proto_error_t parse_connect_q2pro(q2proto_string_t* connect_str, q2proto_connect_t *parsed_connect)
{
    q2proto_string_t netchan_token = {0};
    next_token(&netchan_token, connect_str, ' ');
    if (netchan_token.len > 0)
        parsed_connect->q2pro_nctype = q2pstol(&netchan_token, 10);
    else
        parsed_connect->q2pro_nctype = 1; // NETCHAN_NEW

    q2proto_string_t zlib_token = {0};
    next_token(&zlib_token, connect_str, ' ');
    parsed_connect->has_zlib = q2pstol(&zlib_token, 10) != 0;

    // set minor protocol version
    q2proto_string_t protocol_ver_token = {0};
    next_token(&protocol_ver_token, connect_str, ' ');
    if (protocol_ver_token.len > 0) {
        parsed_connect->version = q2pstol(&protocol_ver_token, 10);
        if (parsed_connect->version < PROTOCOL_VERSION_Q2PRO_MINIMUM)
            parsed_connect->version = PROTOCOL_VERSION_Q2PRO_MINIMUM;
        else if (parsed_connect->version > PROTOCOL_VERSION_Q2PRO_CURRENT)
            parsed_connect->version = PROTOCOL_VERSION_Q2PRO_CURRENT;
        if (parsed_connect->version == PROTOCOL_VERSION_Q2PRO_RESERVED)
            parsed_connect->version--; // never use this version
    } else {
        parsed_connect->version = PROTOCOL_VERSION_Q2PRO_MINIMUM;
    }

    return Q2P_ERR_SUCCESS;
}

// Filter list of accepted protocols by restrictions from server info (mainly game type atm)
static size_t filter_accepted_protocols(q2proto_protocol_t *new_accepted_protocols, const q2proto_protocol_t *accepted_protocols, size_t num_accepted_protocols, const q2proto_server_info_t *server_info)
{
    size_t out_num = 0;
    for (size_t i = 0; i < num_accepted_protocols; i++)
    {
        q2proto_protocol_t protocol = accepted_protocols[i];
        switch (server_info->game_type)
        {
        case Q2PROTO_GAME_VANILLA:
            new_accepted_protocols[out_num++] = protocol;
            break;
        case Q2PROTO_GAME_Q2PRO_EXTENDED:
        case Q2PROTO_GAME_Q2PRO_EXTENDED_V2:
            if (protocol == Q2P_PROTOCOL_Q2PRO)
                new_accepted_protocols[out_num++] = protocol;
            break;
        }
    }
    return out_num;
}

q2proto_error_t q2proto_parse_connect(const char *connect_args, const q2proto_protocol_t *accepted_protocols, size_t num_accepted_protocols, const q2proto_server_info_t *server_info, q2proto_connect_t *parsed_connect)
{
    q2proto_protocol_t *new_accepted_protocols = alloca(sizeof(q2proto_protocol_t) * num_accepted_protocols);
    size_t num_new_accepted_protocols = filter_accepted_protocols(new_accepted_protocols, accepted_protocols, num_accepted_protocols, server_info);

    memset(parsed_connect, 0, sizeof(*parsed_connect));

    q2proto_string_t connect_str = q2proto_make_string(connect_args);

    q2proto_error_t parse_err;
    // Parse challenge value
    long protocol_value;
    parse_err = next_connect_int(&connect_str, &protocol_value);
    if (parse_err != Q2P_ERR_SUCCESS)
        return parse_err;

    parsed_connect->protocol = q2proto_protocol_from_netver(protocol_value);
    bool proto_found = false;
    for (size_t i = 0; i < num_new_accepted_protocols; i++)
    {
        if (new_accepted_protocols[i] == parsed_connect->protocol)
        {
            proto_found = true;
            break;
        }
    }
    if (!proto_found)
        return Q2P_ERR_PROTOCOL_NOT_SUPPORTED;

    long qport_value;
    parse_err = next_connect_int(&connect_str, &qport_value);
    if (parse_err != Q2P_ERR_SUCCESS)
        return parse_err;
    parsed_connect->qport = qport_value;

    long challenge_value;
    parse_err = next_connect_int(&connect_str, &challenge_value);
    if (parse_err != Q2P_ERR_SUCCESS)
        return parse_err;
    parsed_connect->challenge = challenge_value;

    if (!next_token(&parsed_connect->userinfo, &connect_str, ' '))
        return Q2P_ERR_BAD_DATA;

    long packet_length_value = server_info->default_packet_length;
    if (parsed_connect->protocol >= Q2P_PROTOCOL_R1Q2)
    {
        q2proto_string_t packet_length_token = {0};
        if (!next_token(&packet_length_token, &connect_str, ' '))
            return Q2P_ERR_BAD_DATA;
        if (packet_length_token.len > 0)
            packet_length_value = q2pstol(&packet_length_token, 10);
    }
    parsed_connect->packet_length = packet_length_value;

    switch(parsed_connect->protocol)
    {
    case Q2P_PROTOCOL_INVALID:
        return Q2P_ERR_PROTOCOL_NOT_SUPPORTED;
    case Q2P_PROTOCOL_OLD_DEMO:
    case Q2P_PROTOCOL_VANILLA:
        break;
    case Q2P_PROTOCOL_R1Q2:
        return parse_connect_r1q2(&connect_str, parsed_connect);
    case Q2P_PROTOCOL_Q2PRO:
        return parse_connect_q2pro(&connect_str, parsed_connect);
    }

    return Q2P_ERR_SUCCESS;
}

q2proto_error_t q2proto_init_servercontext(q2proto_servercontext_t* context, const q2proto_server_info_t *server_info, const q2proto_connect_t* connect_info)
{
    memset(context, 0, sizeof(*context));
    context->server_info = server_info;

    switch(connect_info->protocol)
    {
    case Q2P_PROTOCOL_INVALID:
    case Q2P_PROTOCOL_OLD_DEMO:
        return Q2P_ERR_PROTOCOL_NOT_SUPPORTED;
    case Q2P_PROTOCOL_VANILLA:
        return q2proto_vanilla_init_servercontext(context, connect_info);
    case Q2P_PROTOCOL_R1Q2:
        return q2proto_r1q2_init_servercontext(context, connect_info);
    case Q2P_PROTOCOL_Q2PRO:
        return q2proto_q2pro_init_servercontext(context, connect_info);
    }

    return Q2P_ERR_PROTOCOL_NOT_SUPPORTED;
}

q2proto_error_t q2proto_server_fill_serverdata(q2proto_servercontext_t *context, q2proto_svc_serverdata_t *serverdata)
{
    return context->fill_serverdata(context, serverdata);
}

void q2proto_server_make_entity_state_delta(q2proto_servercontext_t *context, const q2proto_packed_entity_state_t *from, const q2proto_packed_entity_state_t *to, bool write_old_origin, q2proto_entity_state_delta_t *delta)
{
    context->make_entity_state_delta(context, from, to, write_old_origin, delta);
}

void q2proto_server_make_player_state_delta(q2proto_servercontext_t *context, const q2proto_packed_player_state_t *from, const q2proto_packed_player_state_t *to, q2proto_svc_playerstate_t *delta)
{
    context->make_player_state_delta(context, from, to, delta);
}

q2proto_error_t q2proto_server_write_pos(const q2proto_server_info_t *server_info, uintptr_t io_arg, const q2proto_vec3_t pos)
{
    if (server_info->game_type == Q2PROTO_GAME_Q2PRO_EXTENDED_V2)
    {
        WRITE_CHECKED(server_write, io_arg, q2pro_i23, _q2proto_valenc_coord2int(pos[0]), 0);
        WRITE_CHECKED(server_write, io_arg, q2pro_i23, _q2proto_valenc_coord2int(pos[1]), 0);
        WRITE_CHECKED(server_write, io_arg, q2pro_i23, _q2proto_valenc_coord2int(pos[2]), 0);
    }
    else
    {
        WRITE_CHECKED(server_write, io_arg, u16, _q2proto_valenc_coord2int(pos[0]));
        WRITE_CHECKED(server_write, io_arg, u16, _q2proto_valenc_coord2int(pos[1]));
        WRITE_CHECKED(server_write, io_arg, u16, _q2proto_valenc_coord2int(pos[2]));
    }
    return Q2P_ERR_SUCCESS;
}

q2proto_error_t q2proto_server_write(q2proto_servercontext_t *context, uintptr_t io_arg, const q2proto_svc_message_t *svc_message)
{
    return context->server_write(context, io_arg, svc_message);
}

q2proto_error_t q2proto_server_write_gamestate(q2proto_servercontext_t *context, q2protoio_deflate_args_t* deflate_args, uintptr_t io_arg, const q2proto_gamestate_t *gamestate)
{
    return context->server_write_gamestate(context, deflate_args, io_arg, gamestate);
}

q2proto_error_t q2proto_server_write_zpacket(q2proto_servercontext_t *context, q2protoio_deflate_args_t *deflate_args, uintptr_t io_arg, const void *packet_data, size_t packet_len)
{
#if Q2PROTO_COMPRESSION_DEFLATE
    if (!context->features.enable_deflate)
        return Q2P_ERR_DEFLATE_NOT_SUPPORTED;

    // Don't double-compress messages...
    uint8_t message_type = *(const uint8_t *)packet_data;
    if (message_type == svc_r1q2_zpacket)
        return Q2P_ERR_ALREADY_COMPRESSED;

    size_t deflate_io_arg;
    size_t max_deflated = q2protoio_write_available(io_arg);
    CHECKED(server_write, io_arg, q2protoio_deflate_begin(deflate_args, max_deflated, &deflate_io_arg));
    WRITE_CHECKED(server_write, deflate_io_arg, raw, packet_data, packet_len, NULL);
    const void *compressed_data;
    size_t uncompressed_len, compressed_len;
    CHECKED(server_write, io_arg, q2protoio_deflate_get_data(deflate_io_arg, &uncompressed_len, &compressed_data, &compressed_len));

    // Data didn't compress very well. No point to wrap it.
    if (compressed_len > uncompressed_len + 5)
    {
        q2protoio_deflate_end(deflate_io_arg);
        return Q2P_ERR_ALREADY_COMPRESSED;
    }

    WRITE_CHECKED(server_write, io_arg, u8, svc_r1q2_zpacket);
    WRITE_CHECKED(server_write, io_arg, u16, compressed_len);
    WRITE_CHECKED(server_write, io_arg, u16, packet_len);
    WRITE_CHECKED(server_write, io_arg, raw, compressed_data, compressed_len, NULL);

    CHECKED(server_write, io_arg, q2protoio_deflate_end(deflate_io_arg));

    return Q2P_ERR_SUCCESS;

#else
    return Q2P_ERR_DEFLATE_NOT_SUPPORTED;
#endif
}

q2proto_error_t q2proto_server_download_begin(q2proto_servercontext_t *context, size_t total_size, q2proto_download_compress_t compress, q2protoio_deflate_args_t* deflate_args, q2proto_server_download_state_t* state)
{
    q2proto_download_common_begin(context, total_size, state);
    if (context->download_funcs->begin)
        return context->download_funcs->begin(context, state, compress, deflate_args);
    else
        return Q2P_ERR_SUCCESS;
}

void q2proto_server_download_end(q2proto_server_download_state_t* state)
{
    if (!state)
        return;

#if Q2PROTO_COMPRESSION_DEFLATE
    if (state->deflate_io_valid)
    {
        q2protoio_deflate_end(state->deflate_io);
        state->deflate_io_valid = false;
    }
#endif
}

q2proto_error_t q2proto_server_download_data(q2proto_server_download_state_t *state, const uint8_t **data, size_t *remaining, size_t packet_remaining, q2proto_svc_download_t *svc_download)
{
    return state->context->download_funcs->data(state, data, remaining, packet_remaining, svc_download);
}

q2proto_error_t q2proto_server_download_finish(q2proto_server_download_state_t *state, q2proto_svc_download_t *svc_download)
{
    return state->context->download_funcs->finish(state, svc_download);
}

q2proto_error_t q2proto_server_download_abort(q2proto_server_download_state_t *state, q2proto_svc_download_t *svc_download)
{
    // Allow generating a "download abort" message even w/o state or context
    if (state && state->context)
        return state->context->download_funcs->abort(state, svc_download);
    else
        return q2proto_download_common_abort(state, svc_download);
}

void q2proto_server_download_get_progress(const q2proto_server_download_state_t *state, size_t *completed, size_t *total)
{
    *completed = state->transferred;
    *total = state->total_size;
}

q2proto_error_t q2proto_server_read(q2proto_servercontext_t *context, uintptr_t io_arg, q2proto_clc_message_t *clc_message)
{
    return context->server_read(context, io_arg, clc_message);
}
