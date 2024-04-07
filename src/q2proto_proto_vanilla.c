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
#include "q2proto_internal.h"

#include <string.h>

//
// CLIENT: PARSE MESSAGES FROM SERVER
//

static q2proto_error_t vanilla_client_read(q2proto_clientcontext_t *context, uintptr_t io_arg, q2proto_svc_message_t *svc_message);
static q2proto_error_t vanilla_client_next_frame_entity_delta(q2proto_clientcontext_t *context, uintptr_t io_arg, q2proto_svc_frame_entity_delta_t *frame_entity_delta);
static uint32_t vanilla_pack_solid(q2proto_clientcontext_t *context, const q2proto_vec3_t mins, const q2proto_vec3_t maxs);
static void vanilla_unpack_solid(q2proto_clientcontext_t *context, uint32_t solid, q2proto_vec3_t mins, q2proto_vec3_t maxs);

q2proto_error_t q2proto_vanilla_continue_serverdata(q2proto_clientcontext_t *context, uintptr_t io_arg, q2proto_svc_serverdata_t *serverdata)
{
    context->pack_solid = vanilla_pack_solid;
    context->unpack_solid = vanilla_unpack_solid;

    READ_CHECKED(client_read, io_arg, serverdata->servercount, i32);
    READ_CHECKED(client_read, io_arg, serverdata->attractloop, bool);
    READ_CHECKED(client_read, io_arg, serverdata->gamedir, string);
    READ_CHECKED(client_read, io_arg, serverdata->clientnum, i16);
    READ_CHECKED(client_read, io_arg, serverdata->levelname, string);

    context->client_read = vanilla_client_read;
    context->server_protocol = serverdata->protocol == PROTOCOL_OLD_DEMO ? Q2P_PROTOCOL_OLD_DEMO : Q2P_PROTOCOL_VANILLA;

    return Q2P_ERR_SUCCESS;
}

static q2proto_error_t vanilla_client_read_serverdata(q2proto_clientcontext_t *context, uintptr_t io_arg, q2proto_svc_serverdata_t *serverdata);
static q2proto_error_t vanilla_client_read_entity_delta(uintptr_t io_arg, uint64_t bits, q2proto_entity_state_delta_t *entity_state);
static q2proto_error_t vanilla_client_read_baseline(uintptr_t io_arg, q2proto_svc_spawnbaseline_t *spawnbaseline);
static q2proto_error_t vanilla_client_read_frame(q2proto_clientcontext_t *context, uintptr_t io_arg, q2proto_svc_frame_t *frame);

static MAYBE_UNUSED const char* server_cmd_string(int command)
{
    const char *str = q2proto_debug_common_svc_string(command);
    return str ? str : q2proto_va("%d", command);
}

static q2proto_error_t vanilla_client_read(q2proto_clientcontext_t *context, uintptr_t io_arg, q2proto_svc_message_t *svc_message)
{
    memset(svc_message, 0, sizeof(*svc_message));

    size_t command_read = 0;
    const void *command_ptr = NULL;
    READ_CHECKED(client_read, io_arg, command_ptr, raw, 1, &command_read);
    if (command_read == 0)
        return Q2P_ERR_NO_MORE_INPUT;

    uint8_t command = *(const uint8_t*)command_ptr;
    SHOWNET(io_arg, 1, -1, "%s", server_cmd_string(command));

    switch (command)
    {
    case svc_nop:
        svc_message->type = Q2P_SVC_NOP;
        return Q2P_ERR_SUCCESS;

    case svc_disconnect:
        svc_message->type = Q2P_SVC_DISCONNECT;
        return Q2P_ERR_SUCCESS;

    case svc_reconnect:
        svc_message->type = Q2P_SVC_RECONNECT;
        return Q2P_ERR_SUCCESS;

    case svc_print:
        svc_message->type = Q2P_SVC_PRINT;
        return q2proto_common_client_read_print(io_arg, &svc_message->print);

    case svc_centerprint:
        svc_message->type = Q2P_SVC_CENTERPRINT;
        return q2proto_common_client_read_centerprint(io_arg, &svc_message->centerprint);

    case svc_stufftext:
        svc_message->type = Q2P_SVC_STUFFTEXT;
        return q2proto_common_client_read_stufftext(io_arg, &svc_message->stufftext);

    case svc_serverdata:
        svc_message->type = Q2P_SVC_SERVERDATA;
        return vanilla_client_read_serverdata(context, io_arg, &svc_message->serverdata);

    case svc_configstring:
        svc_message->type = Q2P_SVC_CONFIGSTRING;
        return q2proto_common_client_read_configstring(io_arg, &svc_message->configstring);

    case svc_sound:
        svc_message->type = Q2P_SVC_SOUND;
        return q2proto_common_client_read_sound(io_arg, &svc_message->sound);

    case svc_spawnbaseline:
        svc_message->type = Q2P_SVC_SPAWNBASELINE;
        return vanilla_client_read_baseline(io_arg, &svc_message->spawnbaseline);

    case svc_temp_entity:
        svc_message->type = Q2P_SVC_TEMP_ENTITY;
        return q2proto_common_client_read_temp_entity(io_arg, &svc_message->temp_entity);

    case svc_muzzleflash:
        svc_message->type = Q2P_SVC_MUZZLEFLASH;
        return q2proto_common_client_read_muzzleflash(io_arg, &svc_message->muzzleflash, MZ_SILENCED);

    case svc_muzzleflash2:
        svc_message->type = Q2P_SVC_MUZZLEFLASH2;
        return q2proto_common_client_read_muzzleflash(io_arg, &svc_message->muzzleflash, 0);

    case svc_download:
        svc_message->type = Q2P_SVC_DOWNLOAD;
        return q2proto_common_client_read_download(io_arg, &svc_message->download);

    case svc_frame:
        svc_message->type = Q2P_SVC_FRAME;
        return vanilla_client_read_frame(context, io_arg, &svc_message->frame);

    case svc_inventory:
        svc_message->type = Q2P_SVC_INVENTORY;
        return q2proto_common_client_read_inventory(io_arg, &svc_message->inventory);

    case svc_layout:
        svc_message->type = Q2P_SVC_LAYOUT;
        return q2proto_common_client_read_layout(io_arg, &svc_message->layout);
    }

    return HANDLE_ERROR(client_read, io_arg, Q2P_ERR_BAD_COMMAND, "%s: bad server command %d", __func__, command);
}

static q2proto_error_t vanilla_client_read_delta_entities(q2proto_clientcontext_t *context, uintptr_t io_arg, q2proto_svc_message_t *svc_message)
{
    memset(svc_message, 0, sizeof(*svc_message));

    svc_message->type = Q2P_SVC_FRAME_ENTITY_DELTA;
    q2proto_error_t err = vanilla_client_next_frame_entity_delta(context, io_arg, &svc_message->frame_entity_delta);
    if (err != Q2P_ERR_SUCCESS)
    {
        // FIXME: May be insufficient, might need some explicit way to reset parsing...
        context->client_read = vanilla_client_read;
        return err;
    }

    if (svc_message->frame_entity_delta.newnum == 0)
    {
        context->client_read = vanilla_client_read;
    }
    return Q2P_ERR_SUCCESS;
}

static q2proto_error_t vanilla_client_next_frame_entity_delta(q2proto_clientcontext_t *context, uintptr_t io_arg, q2proto_svc_frame_entity_delta_t *frame_entity_delta)
{
    memset(frame_entity_delta, 0, sizeof(*frame_entity_delta));

    uint64_t bits;
    CHECKED(client_read, io_arg, q2proto_common_client_read_entity_bits(io_arg, &bits, &frame_entity_delta->newnum));
    if (bits & U_MOREBITS4)
        return Q2P_ERR_BAD_DATA;

#if Q2PROTO_SHOWNET
    if (q2protodbg_shownet_check(io_arg, 2) && bits) {
        char buf[1024];
        q2proto_debug_common_entity_delta_bits_to_str(buf, sizeof(buf), bits);
        SHOWNET(io_arg, 2, -q2proto_common_entity_bits_size(bits), "%s", buf);
    }
#endif

    if (frame_entity_delta->newnum == 0)
    {
        return Q2P_ERR_SUCCESS;
    }

    if (bits & U_REMOVE)
    {
        frame_entity_delta->remove = true;
        return Q2P_ERR_SUCCESS;
    }

    return vanilla_client_read_entity_delta(io_arg, bits, &frame_entity_delta->entity_delta);
}

static q2proto_error_t vanilla_client_read_serverdata(q2proto_clientcontext_t *context, uintptr_t io_arg, q2proto_svc_serverdata_t *serverdata)
{
    int32_t protocol;
    READ_CHECKED(client_read, io_arg, protocol, i32);

    if (protocol != PROTOCOL_OLD_DEMO && protocol != PROTOCOL_VANILLA)
        return HANDLE_ERROR(client_read, io_arg, Q2P_ERR_BAD_DATA, "unexpected protocol %d", protocol);

    serverdata->protocol = protocol;

    return q2proto_vanilla_continue_serverdata(context, io_arg, serverdata);
}

static q2proto_error_t vanilla_client_read_entity_delta(uintptr_t io_arg, uint64_t bits, q2proto_entity_state_delta_t *entity_state)
{
    if (delta_bits_check(bits, U_MODEL, &entity_state->delta_bits, Q2P_ESD_MODELINDEX))
        READ_CHECKED(client_read, io_arg, entity_state->modelindex, u8);
    if (delta_bits_check(bits, U_MODEL2, &entity_state->delta_bits, Q2P_ESD_MODELINDEX2))
        READ_CHECKED(client_read, io_arg, entity_state->modelindex2, u8);
    if (delta_bits_check(bits, U_MODEL3, &entity_state->delta_bits, Q2P_ESD_MODELINDEX3))
        READ_CHECKED(client_read, io_arg, entity_state->modelindex3, u8);
    if (delta_bits_check(bits, U_MODEL4, &entity_state->delta_bits, Q2P_ESD_MODELINDEX4))
        READ_CHECKED(client_read, io_arg, entity_state->modelindex4, u8);

    if (delta_bits_check(bits, U_FRAME8, &entity_state->delta_bits, Q2P_ESD_FRAME))
        READ_CHECKED(client_read, io_arg, entity_state->frame, u8);
    else if (delta_bits_check(bits, U_FRAME16, &entity_state->delta_bits, Q2P_ESD_FRAME))
        READ_CHECKED(client_read, io_arg, entity_state->frame, u16);

    if (delta_bits_check(bits, U_SKIN32, &entity_state->delta_bits, Q2P_ESD_SKINNUM))
    {
        if ((bits & U_SKIN32) == U_SKIN32) // used for laser colors
            READ_CHECKED(client_read, io_arg, entity_state->skinnum, u32);
        else if (bits & U_SKIN16)
            READ_CHECKED(client_read, io_arg, entity_state->skinnum, u16);
        else if (bits & U_SKIN8)
            READ_CHECKED(client_read, io_arg, entity_state->skinnum, u8);
    }

    if (delta_bits_check(bits, U_EFFECTS32, &entity_state->delta_bits, Q2P_ESD_EFFECTS))
    {
        if ((bits & U_EFFECTS32) == U_EFFECTS32)
            READ_CHECKED(client_read, io_arg, entity_state->effects, u32);
        else if (bits & U_EFFECTS16)
            READ_CHECKED(client_read, io_arg, entity_state->effects, u16);
        else if (bits & U_EFFECTS8)
            READ_CHECKED(client_read, io_arg, entity_state->effects, u8);
    }

    if (delta_bits_check(bits, U_RENDERFX32, &entity_state->delta_bits, Q2P_ESD_RENDERFX))
    {
        if ((bits & U_RENDERFX32) == U_RENDERFX32)
            READ_CHECKED(client_read, io_arg, entity_state->renderfx, u32);
        else if (bits & U_RENDERFX16)
            READ_CHECKED(client_read, io_arg, entity_state->renderfx, u16);
        else if (bits & U_RENDERFX8)
            READ_CHECKED(client_read, io_arg, entity_state->renderfx, u8);
    }

    entity_state->origin.read.value.delta_bits = 0;
    if (bits & U_ORIGIN1)
    {
        READ_CHECKED_VAR_COORD_COMP_16(client_read, io_arg, &entity_state->origin.read.value.values, 0);
        entity_state->origin.read.value.delta_bits |= BIT(0);
    }
    if (bits & U_ORIGIN2)
    {
        READ_CHECKED_VAR_COORD_COMP_16(client_read, io_arg, &entity_state->origin.read.value.values, 1);
        entity_state->origin.read.value.delta_bits |= BIT(1);
    }
    if (bits & U_ORIGIN3)
    {
        READ_CHECKED_VAR_COORD_COMP_16(client_read, io_arg, &entity_state->origin.read.value.values, 2);
        entity_state->origin.read.value.delta_bits |= BIT(2);
    }

    entity_state->angle.delta_bits = 0;
    if (bits & U_ANGLE1)
    {
        READ_CHECKED_VAR_ANGLE_COMP_8(client_read, io_arg, &entity_state->angle.values, 0);
        entity_state->angle.delta_bits |= BIT(0);
    }
    if (bits & U_ANGLE2)
    {
        READ_CHECKED_VAR_ANGLE_COMP_8(client_read, io_arg, &entity_state->angle.values, 1);
        entity_state->angle.delta_bits |= BIT(1);
    }
    if (bits & U_ANGLE3)
    {
        READ_CHECKED_VAR_ANGLE_COMP_8(client_read, io_arg, &entity_state->angle.values, 2);
        entity_state->angle.delta_bits |= BIT(2);
    }

    if (delta_bits_check(bits, U_OLDORIGIN, &entity_state->delta_bits, Q2P_ESD_OLD_ORIGIN))
        CHECKED(client_read, io_arg, read_var_coord_short(io_arg, &entity_state->old_origin));

    if (delta_bits_check(bits, U_SOUND, &entity_state->delta_bits, Q2P_ESD_SOUND))
        READ_CHECKED(client_read, io_arg, entity_state->sound, u8);

    if (delta_bits_check(bits, U_EVENT, &entity_state->delta_bits, Q2P_ESD_EVENT))
        READ_CHECKED(client_read, io_arg, entity_state->event, u8);

    if (delta_bits_check(bits, U_SOLID, &entity_state->delta_bits, Q2P_ESD_SOLID))
        READ_CHECKED(client_read, io_arg, entity_state->solid, u16);

    return Q2P_ERR_SUCCESS;
}

static q2proto_error_t vanilla_client_read_baseline(uintptr_t io_arg, q2proto_svc_spawnbaseline_t *spawnbaseline)
{
    uint64_t bits;
    CHECKED(client_read, io_arg, q2proto_common_client_read_entity_bits(io_arg, &bits, &spawnbaseline->entnum));

#if Q2PROTO_SHOWNET
    if (q2protodbg_shownet_check(io_arg, 2) && bits) {
        char buf[1024];
        q2proto_debug_common_entity_delta_bits_to_str(buf, sizeof(buf), bits);
        SHOWNET(io_arg, 2, -q2proto_common_entity_bits_size(bits), "   baseline: %i %s", spawnbaseline->entnum, buf);
    }
#endif

    return vanilla_client_read_entity_delta(io_arg, bits, &spawnbaseline->delta_state);
}

static q2proto_error_t vanilla_client_read_playerstate(uintptr_t io_arg, q2proto_svc_playerstate_t *playerstate)
{
    uint16_t flags;
    READ_CHECKED(client_read, io_arg, flags, u16);

#if Q2PROTO_SHOWNET
    if (q2protodbg_shownet_check(io_arg, 2) && flags) {
        char buf[1024];
        q2proto_debug_common_player_delta_bits_to_str(buf, sizeof(buf), flags);
        SHOWNET(io_arg, 2, -2, "   %s", buf);
    }
#endif

    //
    // parse the pmove_state_t
    //
    if (delta_bits_check(flags, PS_M_TYPE, &playerstate->delta_bits, Q2P_PSD_PM_TYPE))
        READ_CHECKED(client_read, io_arg, playerstate->pm_type, u8);

    if(flags & PS_M_ORIGIN)
    {
        CHECKED(client_read, io_arg, read_var_coord_short(io_arg, &playerstate->pm_origin.read.value.values));
        playerstate->pm_origin.read.value.delta_bits = 0x7;
    }

    if(flags & PS_M_VELOCITY)
    {
        CHECKED(client_read, io_arg, read_var_coord_short(io_arg, &playerstate->pm_velocity.read.value.values));
        playerstate->pm_velocity.read.value.delta_bits = 0x7;
    }

    if (delta_bits_check(flags, PS_M_TIME, &playerstate->delta_bits, Q2P_PSD_PM_TIME))
        READ_CHECKED(client_read, io_arg, playerstate->pm_time, u8);

    if (delta_bits_check(flags, PS_M_FLAGS, &playerstate->delta_bits, Q2P_PSD_PM_FLAGS))
        READ_CHECKED(client_read, io_arg, playerstate->pm_flags, u8);

    if (delta_bits_check(flags, PS_M_GRAVITY, &playerstate->delta_bits, Q2P_PSD_PM_GRAVITY))
        READ_CHECKED(client_read, io_arg, playerstate->pm_gravity, i16);

    if (delta_bits_check(flags, PS_M_DELTA_ANGLES, &playerstate->delta_bits, Q2P_PSD_PM_DELTA_ANGLES))
        CHECKED(client_read, io_arg, read_var_angle16(io_arg, &playerstate->pm_delta_angles));

    //
    // parse the rest of the player_state_t
    //
    if (delta_bits_check(flags, PS_VIEWOFFSET, &playerstate->delta_bits, Q2P_PSD_VIEWOFFSET))
        CHECKED(client_read, io_arg, read_var_small_offset(io_arg, &playerstate->viewoffset));

    if(flags & PS_VIEWANGLES)
    {
        CHECKED(client_read, io_arg, read_var_angle16(io_arg, &playerstate->viewangles.values));
        playerstate->viewangles.delta_bits = 0x7;
    }

    if (delta_bits_check(flags, PS_KICKANGLES, &playerstate->delta_bits, Q2P_PSD_KICKANGLES))
        CHECKED(client_read, io_arg, read_var_small_angles(io_arg, &playerstate->kick_angles));

    if (delta_bits_check(flags, PS_WEAPONINDEX, &playerstate->delta_bits, Q2P_PSD_GUNINDEX))
        READ_CHECKED(client_read, io_arg, playerstate->gunindex, u8);

    if (delta_bits_check(flags, PS_WEAPONFRAME, &playerstate->delta_bits, Q2P_PSD_GUNFRAME | Q2P_PSD_GUNOFFSET | Q2P_PSD_GUNANGLES))
    {
        READ_CHECKED(client_read, io_arg, playerstate->gunframe, u8);
        CHECKED(client_read, io_arg, read_var_small_offset(io_arg, &playerstate->gunoffset));
        CHECKED(client_read, io_arg, read_var_small_angles(io_arg, &playerstate->gunangles));
    }

    if (flags & PS_BLEND)
    {
        CHECKED(client_read, io_arg, read_var_blend(io_arg, &playerstate->blend.values));
        playerstate->blend.delta_bits = 0xf;
    }

    if (delta_bits_check(flags, PS_FOV, &playerstate->delta_bits, Q2P_PSD_FOV))
        READ_CHECKED(client_read, io_arg, playerstate->fov, u8);

    if (delta_bits_check(flags, PS_RDFLAGS, &playerstate->delta_bits, Q2P_PSD_RDFLAGS))
        READ_CHECKED(client_read, io_arg, playerstate->rdflags, u8);

    // parse stats
    READ_CHECKED(client_read, io_arg, playerstate->statbits, u32);
    for (int i = 0; i < 32; i++)
        if (playerstate->statbits & (1 << i))
            READ_CHECKED(client_read, io_arg, playerstate->stats[i], i16);

    return Q2P_ERR_SUCCESS;
}

static q2proto_error_t vanilla_client_read_frame(q2proto_clientcontext_t *context, uintptr_t io_arg, q2proto_svc_frame_t* frame)
{
    READ_CHECKED(client_read, io_arg, frame->serverframe, i32);
    READ_CHECKED(client_read, io_arg, frame->deltaframe, i32);

    // BIG HACK to let old demos continue to work
    if (context->server_protocol != Q2P_PROTOCOL_OLD_DEMO)
        READ_CHECKED(client_read, io_arg, frame->suppress_count, u8);

    // read areabits
    READ_CHECKED(client_read, io_arg, frame->areabits_len, u8);
    READ_CHECKED(client_read, io_arg, frame->areabits, raw, frame->areabits_len, NULL);

    uint8_t cmd;
    // read playerinfo
    READ_CHECKED(client_read, io_arg, cmd, u8);
    if (cmd != svc_playerinfo)
        return HANDLE_ERROR(client_read, io_arg, Q2P_ERR_BAD_DATA, "%s: expected playerinfo, got %d", __func__, cmd);
    SHOWNET(io_arg, 2, -1, "playerinfo");
    CHECKED(client_read, io_arg, vanilla_client_read_playerstate(io_arg, &frame->playerstate));

    // read packet entities
    READ_CHECKED(client_read, io_arg, cmd, u8);
    if (cmd != svc_packetentities)
        return HANDLE_ERROR(client_read, io_arg, Q2P_ERR_BAD_DATA, "%s: expected packetentities, got %d", __func__, cmd);
    context->client_read = vanilla_client_read_delta_entities;
    SHOWNET(io_arg, 2, -1, "packetentities");

    return Q2P_ERR_SUCCESS;
}

static uint32_t vanilla_pack_solid(q2proto_clientcontext_t *context, const q2proto_vec3_t mins, const q2proto_vec3_t maxs)
{
    return q2proto_pack_solid_16(mins, maxs);
}

static void vanilla_unpack_solid(q2proto_clientcontext_t *context, uint32_t solid, q2proto_vec3_t mins, q2proto_vec3_t maxs)
{
    q2proto_unpack_solid_16(solid, mins, maxs);
}
