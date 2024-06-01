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
#include "q2proto_internal_common.h"

#include "q2proto_internal_defs.h"
#include "q2proto_internal_io.h"
#include "q2proto_internal_protocol.h"

q2proto_error_t q2proto_common_client_read_entity_bits(uintptr_t io_arg, uint64_t *bits, uint16_t *entnum)
{
    uint64_t total;
    READ_CHECKED(client_read, io_arg, total, u8);

    uint64_t b;
    if (total & U_MOREBITS1)
    {
        READ_CHECKED(client_read, io_arg, b, u8);
        total |= b << 8;
    }
    if (total & U_MOREBITS2)
    {
        READ_CHECKED(client_read, io_arg, b, u8);
        total |= b << 16;
    }
    if (total & U_MOREBITS3)
    {
        READ_CHECKED(client_read, io_arg, b, u8);
        total |= b << 24;
    }
    if (total & U_MOREBITS4)
    {
        READ_CHECKED(client_read, io_arg, b, u8);
        total |= b << 32;
    }

    if (total & U_NUMBER16)
        READ_CHECKED(client_read, io_arg, *entnum, u16);
    else
        READ_CHECKED(client_read, io_arg, *entnum, u8);

    *bits = total;

    return Q2P_ERR_SUCCESS;
}

int q2proto_common_entity_bits_size(uint64_t bits)
{
    int bits_size = 0;
    if(bits & U_MOREBITS4)
        bits_size = 5;
    else if(bits & U_MOREBITS3)
        bits_size = 4;
    else if(bits & U_MOREBITS2)
        bits_size = 3;
    else if(bits & U_MOREBITS1)
        bits_size = 2;
    else
        bits_size = 1;
    bits_size += (bits & U_NUMBER16) ? 2 : 1;
    return bits_size;
}

q2proto_error_t q2proto_common_server_write_entity_bits(uintptr_t io_arg, uint64_t bits, uint16_t entnum)
{
    if (entnum >= 256)
        bits |= U_NUMBER16;

    if (bits & 0xff00000000ull)
        bits |= U_MOREBITS4 | U_MOREBITS3 | U_MOREBITS2 | U_MOREBITS1;
    else if (bits & 0xff000000)
        bits |= U_MOREBITS3 | U_MOREBITS2 | U_MOREBITS1;
    else if (bits & 0x00ff0000)
        bits |= U_MOREBITS2 | U_MOREBITS1;
    else if (bits & 0x0000ff00)
        bits |= U_MOREBITS1;

    WRITE_CHECKED(server_write, io_arg, u8, bits & 0xff);
    if (bits & U_MOREBITS1) WRITE_CHECKED(server_write, io_arg, u8, (bits >>  8) & 0xff);
    if (bits & U_MOREBITS2) WRITE_CHECKED(server_write, io_arg, u8, (bits >> 16) & 0xff);
    if (bits & U_MOREBITS3) WRITE_CHECKED(server_write, io_arg, u8, (bits >> 24) & 0xff);
    if (bits & U_MOREBITS4) WRITE_CHECKED(server_write, io_arg, u8, (bits >> 32) & 0xff);

    if (bits & U_NUMBER16)
        WRITE_CHECKED(server_write, io_arg, u16, entnum);
    else
        WRITE_CHECKED(server_write, io_arg, u8, entnum);

    return Q2P_ERR_SUCCESS;
}

q2proto_error_t q2proto_common_client_read_muzzleflash(uintptr_t io_arg, q2proto_svc_muzzleflash_t *muzzleflash, uint16_t silenced_mask)
{
    READ_CHECKED(client_read, io_arg, muzzleflash->entity, i16);
    READ_CHECKED(client_read, io_arg, muzzleflash->weapon, u8);
    muzzleflash->silenced = muzzleflash->weapon & silenced_mask;
    muzzleflash->weapon &= ~silenced_mask;
    return Q2P_ERR_SUCCESS;
}

static q2proto_error_t read_tent_coord(uintptr_t io_arg, float* coord)
{
    int16_t c;
    READ_CHECKED(client_read, io_arg, c, i16);
    *coord = _q2proto_valenc_int2coord(c);
    return Q2P_ERR_SUCCESS;
}

static q2proto_error_t read_tent_position(uintptr_t io_arg, q2proto_vec3_t pos)
{
    CHECKED(client_read, io_arg, read_tent_coord(io_arg, &pos[0]));
    CHECKED(client_read, io_arg, read_tent_coord(io_arg, &pos[1]));
    CHECKED(client_read, io_arg, read_tent_coord(io_arg, &pos[2]));
    return Q2P_ERR_SUCCESS;
}

#define NUMVERTEXNORMALS    162

static const q2proto_vec3_t bytedirs[NUMVERTEXNORMALS] = {
    #include "anorms.h"
};

static q2proto_error_t read_tent_direction(uintptr_t io_arg, q2proto_vec3_t dir)
{
    uint8_t dir_idx;
    READ_CHECKED(client_read, io_arg, dir_idx, u8);
    if (dir_idx < 0 || dir_idx >= NUMVERTEXNORMALS)
        return Q2P_ERR_BAD_DATA;
    memcpy(dir, bytedirs[dir_idx], sizeof(q2proto_vec3_t));

    return Q2P_ERR_SUCCESS;
}

q2proto_error_t q2proto_common_client_read_temp_entity(uintptr_t io_arg, q2proto_svc_temp_entity_t *temp_entity)
{
    READ_CHECKED(client_read, io_arg, temp_entity->type, u8);

    switch (temp_entity->type) {
    case TE_BLOOD:
    case TE_GUNSHOT:
    case TE_SPARKS:
    case TE_BULLET_SPARKS:
    case TE_SCREEN_SPARKS:
    case TE_SHIELD_SPARKS:
    case TE_SHOTGUN:
    case TE_BLASTER:
    case TE_GREENBLOOD:
    case TE_BLASTER2:
    case TE_FLECHETTE:
    case TE_HEATBEAM_SPARKS:
    case TE_HEATBEAM_STEAM:
    case TE_MOREBLOOD:
    case TE_ELECTRIC_SPARKS:
    case TE_BLUEHYPERBLASTER_2:
    case TE_BERSERK_SLAM:
        CHECKED(client_read, io_arg, read_tent_position(io_arg, temp_entity->position1));
        CHECKED(client_read, io_arg, read_tent_direction(io_arg, temp_entity->direction));
        break;

    case TE_SPLASH:
    case TE_LASER_SPARKS:
    case TE_WELDING_SPARKS:
    case TE_TUNNEL_SPARKS:
        READ_CHECKED(client_read, io_arg, temp_entity->count, u8);
        CHECKED(client_read, io_arg, read_tent_position(io_arg, temp_entity->position1));
        CHECKED(client_read, io_arg, read_tent_direction(io_arg, temp_entity->direction));
        READ_CHECKED(client_read, io_arg, temp_entity->color, u8);
        break;

    case TE_BLUEHYPERBLASTER:
    case TE_RAILTRAIL:
    case TE_RAILTRAIL2:
    case TE_BUBBLETRAIL:
    case TE_DEBUGTRAIL:
    case TE_BUBBLETRAIL2:
    case TE_BFG_LASER:
    case TE_BFG_ZAP:
        CHECKED(client_read, io_arg, read_tent_position(io_arg, temp_entity->position1));
        CHECKED(client_read, io_arg, read_tent_position(io_arg, temp_entity->position2));
        break;

    case TE_GRENADE_EXPLOSION:
    case TE_GRENADE_EXPLOSION_WATER:
    case TE_EXPLOSION2:
    case TE_PLASMA_EXPLOSION:
    case TE_ROCKET_EXPLOSION:
    case TE_ROCKET_EXPLOSION_WATER:
    case TE_EXPLOSION1:
    case TE_EXPLOSION1_NP:
    case TE_EXPLOSION1_BIG:
    case TE_BFG_EXPLOSION:
    case TE_BFG_BIGEXPLOSION:
    case TE_BOSSTPORT:
    case TE_PLAIN_EXPLOSION:
    case TE_CHAINFIST_SMOKE:
    case TE_TRACKER_EXPLOSION:
    case TE_TELEPORT_EFFECT:
    case TE_DBALL_GOAL:
    case TE_WIDOWSPLASH:
    case TE_NUKEBLAST:
    case TE_EXPLOSION1_NL:
    case TE_EXPLOSION2_NL:
        CHECKED(client_read, io_arg, read_tent_position(io_arg, temp_entity->position1));
        break;

    case TE_PARASITE_ATTACK:
    case TE_MEDIC_CABLE_ATTACK:
    case TE_HEATBEAM:
    case TE_MONSTER_HEATBEAM:
    case TE_GRAPPLE_CABLE_2:
    case TE_LIGHTNING_BEAM:
        READ_CHECKED(client_read, io_arg, temp_entity->entity1, i16);
        CHECKED(client_read, io_arg, read_tent_position(io_arg, temp_entity->position1));
        CHECKED(client_read, io_arg, read_tent_position(io_arg, temp_entity->position2));
        break;

    case TE_GRAPPLE_CABLE:
        READ_CHECKED(client_read, io_arg, temp_entity->entity1, i16);
        CHECKED(client_read, io_arg, read_tent_position(io_arg, temp_entity->position1));
        CHECKED(client_read, io_arg, read_tent_position(io_arg, temp_entity->position2));
        CHECKED(client_read, io_arg, read_tent_position(io_arg, temp_entity->offset));
        break;

    case TE_LIGHTNING:
        READ_CHECKED(client_read, io_arg, temp_entity->entity1, i16);
        READ_CHECKED(client_read, io_arg, temp_entity->entity2, i16);
        CHECKED(client_read, io_arg, read_tent_position(io_arg, temp_entity->position1));
        CHECKED(client_read, io_arg, read_tent_position(io_arg, temp_entity->position2));
        break;

    case TE_FLASHLIGHT:
        CHECKED(client_read, io_arg, read_tent_position(io_arg, temp_entity->position1));
        READ_CHECKED(client_read, io_arg, temp_entity->entity1, i16);
        break;

    case TE_FORCEWALL:
        CHECKED(client_read, io_arg, read_tent_position(io_arg, temp_entity->position1));
        CHECKED(client_read, io_arg, read_tent_position(io_arg, temp_entity->position2));
        READ_CHECKED(client_read, io_arg, temp_entity->color, u8);
        break;

    case TE_STEAM:
        READ_CHECKED(client_read, io_arg, temp_entity->entity1, i16);
        READ_CHECKED(client_read, io_arg, temp_entity->count, u8);
        CHECKED(client_read, io_arg, read_tent_position(io_arg, temp_entity->position1));
        CHECKED(client_read, io_arg, read_tent_direction(io_arg, temp_entity->direction));
        READ_CHECKED(client_read, io_arg, temp_entity->color, u8);
        READ_CHECKED(client_read, io_arg, temp_entity->entity2, i16);
        if (temp_entity->entity1 != -1)
        {
            READ_CHECKED(client_read, io_arg, temp_entity->time, i32);
        }
        break;

    case TE_WIDOWBEAMOUT:
        READ_CHECKED(client_read, io_arg, temp_entity->entity1, i16);
        CHECKED(client_read, io_arg, read_tent_position(io_arg, temp_entity->position1));
        break;

    case TE_POWER_SPLASH:
        READ_CHECKED(client_read, io_arg, temp_entity->entity1, i16);
        READ_CHECKED(client_read, io_arg, temp_entity->count, u8);
        break;

    default:
        return HANDLE_ERROR(client_read, io_arg, Q2P_ERR_BAD_DATA, "%s: invalid tent %d", __func__, temp_entity->type);
    }

    return Q2P_ERR_SUCCESS;
}

q2proto_error_t q2proto_common_client_read_layout(uintptr_t io_arg, q2proto_svc_layout_t *layout)
{
    READ_CHECKED(client_read, io_arg, layout->layout_str, string);

    return Q2P_ERR_SUCCESS;
}

q2proto_error_t q2proto_common_client_read_inventory(uintptr_t io_arg, q2proto_svc_inventory_t *inventory)
{
    for (int i = 0; i < Q2PROTO_INVENTORY_ITEMS; i++)
    {
        READ_CHECKED(client_read, io_arg, inventory->inventory[i], i16);
    }

    return Q2P_ERR_SUCCESS;
}

q2proto_error_t q2proto_common_client_read_sound(uintptr_t io_arg, q2proto_svc_sound_t *sound)
{
    READ_CHECKED(client_read, io_arg, sound->flags, u8);
    READ_CHECKED(client_read, io_arg, sound->index, u8);

    if (sound->flags & SND_VOLUME)
        READ_CHECKED(client_read, io_arg, sound->volume, u8);
    else
        sound->volume = 255;

    if (sound->flags & SND_ATTENUATION)
        READ_CHECKED(client_read, io_arg, sound->attenuation, u8);
    else
        sound->attenuation = 64;

    if (sound->flags & SND_OFFSET)
        READ_CHECKED(client_read, io_arg, sound->timeofs, u8);

    if (sound->flags & SND_ENT)
    {
        // entity relative
        uint16_t channel;
        READ_CHECKED(client_read, io_arg, channel, u16);
        sound->entity = channel >> 3;
        sound->channel = channel & 7;
    }

    // positioned in space
    if (sound->flags & SND_POS)
        CHECKED(client_read, io_arg, read_var_coord_short(io_arg, &sound->pos));

    return Q2P_ERR_SUCCESS;
}

q2proto_error_t q2proto_common_client_read_print(uintptr_t io_arg, q2proto_svc_print_t *print)
{
    READ_CHECKED(client_read, io_arg, print->level, u8);
    READ_CHECKED(client_read, io_arg, print->string, string);

    return Q2P_ERR_SUCCESS;
}

q2proto_error_t q2proto_common_client_read_stufftext(uintptr_t io_arg, q2proto_svc_stufftext_t *stufftext)
{
    READ_CHECKED(client_read, io_arg, stufftext->string, string);

    return Q2P_ERR_SUCCESS;
}

q2proto_error_t q2proto_common_client_read_configstring(uintptr_t io_arg, q2proto_svc_configstring_t *configstring)
{
    READ_CHECKED(client_read, io_arg, configstring->index, u16);
    READ_CHECKED(client_read, io_arg, configstring->value, string);

    return Q2P_ERR_SUCCESS;
}

q2proto_error_t q2proto_common_client_read_centerprint(uintptr_t io_arg, q2proto_svc_centerprint_t *centerprint)
{
    READ_CHECKED(client_read, io_arg, centerprint->message, string);

    return Q2P_ERR_SUCCESS;
}

q2proto_error_t q2proto_common_client_read_download(uintptr_t io_arg, q2proto_svc_download_t *download)
{
    READ_CHECKED(client_read, io_arg, download->size, i16);
    READ_CHECKED(client_read, io_arg, download->percent, u8);
    if (download->size > 0)
        READ_CHECKED(client_read, io_arg, download->data, raw, download->size, NULL);
    return Q2P_ERR_SUCCESS;
}

q2proto_error_t q2proto_common_server_write_nop(uintptr_t io_arg)
{
    WRITE_CHECKED(server_write, io_arg, u8, svc_nop);
    return Q2P_ERR_SUCCESS;
}

q2proto_error_t q2proto_common_server_write_disconnect(uintptr_t io_arg)
{
    WRITE_CHECKED(server_write, io_arg, u8, svc_disconnect);
    return Q2P_ERR_SUCCESS;
}

q2proto_error_t q2proto_common_server_write_reconnect(uintptr_t io_arg)
{
    WRITE_CHECKED(server_write, io_arg, u8, svc_reconnect);
    return Q2P_ERR_SUCCESS;
}

q2proto_error_t q2proto_common_server_write_sound(uintptr_t io_arg, const q2proto_svc_sound_t *sound)
{
    WRITE_CHECKED(server_write, io_arg, u8, svc_sound);
    WRITE_CHECKED(server_write, io_arg, u8, sound->flags);
    WRITE_CHECKED(server_write, io_arg, u8, sound->index);

    if (sound->flags & SND_VOLUME)
        WRITE_CHECKED(server_write, io_arg, u8, sound->volume);
    if (sound->flags & SND_ATTENUATION)
        WRITE_CHECKED(server_write, io_arg, u8, sound->attenuation);
    if (sound->flags & SND_OFFSET)
        WRITE_CHECKED(server_write, io_arg, u8, sound->timeofs);

    if (sound->flags & SND_ENT)
        WRITE_CHECKED(server_write, io_arg, u16, (sound->entity << 3) | (sound->channel & 0x7));

    if (sound->flags & SND_POS)
        WRITE_CHECKED(server_write, io_arg, var_coord_short, &sound->pos);

    return Q2P_ERR_SUCCESS;
}

q2proto_error_t q2proto_common_server_write_print(uintptr_t io_arg, const q2proto_svc_print_t *print)
{
    WRITE_CHECKED(server_write, io_arg, u8, svc_print);
    WRITE_CHECKED(server_write, io_arg, u8, print->level);
    WRITE_CHECKED(server_write, io_arg, string, &print->string);
    return Q2P_ERR_SUCCESS;
}

q2proto_error_t q2proto_common_server_write_stufftext(uintptr_t io_arg, const q2proto_svc_stufftext_t *stufftext)
{
    WRITE_CHECKED(server_write, io_arg, u8, svc_stufftext);
    WRITE_CHECKED(server_write, io_arg, string, &stufftext->string);
    return Q2P_ERR_SUCCESS;
}

q2proto_error_t q2proto_common_server_write_configstring(uintptr_t io_arg, const q2proto_svc_configstring_t *configstring)
{
    WRITE_CHECKED(server_write, io_arg, u8, svc_configstring);
    WRITE_CHECKED(server_write, io_arg, u16, configstring->index);
    WRITE_CHECKED(server_write, io_arg, string, &configstring->value);
    return Q2P_ERR_SUCCESS;
}

q2proto_error_t q2proto_common_server_write_centerprint(uintptr_t io_arg, const q2proto_svc_centerprint_t *centerprint)
{
    WRITE_CHECKED(server_write, io_arg, u8, svc_centerprint);
    WRITE_CHECKED(server_write, io_arg, string, &centerprint->message);
    return Q2P_ERR_SUCCESS;
}

q2proto_error_t q2proto_common_server_read_userinfo(uintptr_t io_arg, q2proto_clc_userinfo_t *userinfo)
{
    READ_CHECKED(server_read, io_arg, userinfo->str, string);
    return Q2P_ERR_SUCCESS;
}

q2proto_error_t q2proto_common_server_read_stringcmd(uintptr_t io_arg, q2proto_clc_stringcmd_t *stringcmd)
{
    READ_CHECKED(server_read, io_arg, stringcmd->cmd, string);
    return Q2P_ERR_SUCCESS;
}