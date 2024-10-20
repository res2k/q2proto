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

#define Q2PROTO_BUILD
#include "q2proto_internal_packing.h"

#include "q2proto_internal_defs.h"
#include "q2proto_internal_io.h"

const q2proto_packed_entity_state_t q2proto_null_packed_entity_state;
const q2proto_packed_player_state_t q2proto_null_packed_player_state;

void q2proto_packing_make_entity_state_delta(const q2proto_packed_entity_state_t *from, const q2proto_packed_entity_state_t *to, bool write_old_origin, bool extended_state, q2proto_entity_state_delta_t *delta)
{
    memset(delta, 0, sizeof(*delta));

    if (!from)
        from = &q2proto_null_packed_entity_state;

    q2proto_var_coord_set_int(&delta->origin.write.prev, from->origin);
    q2proto_var_coord_set_int(&delta->origin.write.current, to->origin);

    if (to->angles[0] != from->angles[0])
    {
        delta->angle.delta_bits |= BIT(0);
        q2proto_var_angle_set_short_comp(&delta->angle.values, 0, to->angles[0]);
    }
    if (to->angles[1] != from->angles[1])
    {
        delta->angle.delta_bits |= BIT(1);
        q2proto_var_angle_set_short_comp(&delta->angle.values, 1, to->angles[1]);
    }
    if (to->angles[2] != from->angles[2])
    {
        delta->angle.delta_bits |= BIT(2);
        q2proto_var_angle_set_short_comp(&delta->angle.values, 2, to->angles[2]);
    }

    if (write_old_origin)
    {
        delta->delta_bits |= Q2P_ESD_OLD_ORIGIN;
        q2proto_var_coord_set_int(&delta->old_origin, to->old_origin);
    }

    if (to->skinnum != from->skinnum)
    {
        delta->delta_bits |= Q2P_ESD_SKINNUM;
        delta->skinnum = to->skinnum;
    }

    if (to->frame != from->frame)
    {
        delta->delta_bits |= Q2P_ESD_FRAME;
        delta->frame = to->frame;
    }

    if (to->effects != from->effects)
    {
        if ((uint32_t)to->effects != (uint32_t)from->effects)
            delta->delta_bits |= Q2P_ESD_EFFECTS;
    #if Q2PROTO_ENTITY_STATE_FEATURES >= Q2PROTO_FEATURES_Q2PRO_EXTENDED
        if (extended_state && ((to->effects >> 32) != (from->effects >> 32)))
            delta->delta_bits |= Q2P_ESD_EFFECTS_MORE;
    #endif
        if (delta->delta_bits & (Q2P_ESD_EFFECTS | Q2P_ESD_EFFECTS_MORE))
        {
            delta->effects = to->effects;
        #if Q2PROTO_ENTITY_STATE_FEATURES >= Q2PROTO_FEATURES_Q2PRO_EXTENDED
            delta->effects_more = to->effects >> 32;
        #endif
        }
    }

    if (to->renderfx != from->renderfx)
    {
        delta->delta_bits |= Q2P_ESD_RENDERFX;
        delta->renderfx = to->renderfx;
    }

    if (to->solid != from->solid)
    {
        delta->delta_bits |= Q2P_ESD_SOLID;
        delta->solid = to->solid;
    }

    // event is not delta compressed, just 0 compressed
    if (to->event)
    {
        delta->delta_bits |= Q2P_ESD_EVENT;
        delta->event = to->event;
    }

    if (to->modelindex != from->modelindex)
    {
        delta->delta_bits |= Q2P_ESD_MODELINDEX;
        delta->modelindex = to->modelindex;
    }
    if (to->modelindex2 != from->modelindex2)
    {
        delta->delta_bits |= Q2P_ESD_MODELINDEX2;
        delta->modelindex2 = to->modelindex2;
    }
    if (to->modelindex3 != from->modelindex3)
    {
        delta->delta_bits |= Q2P_ESD_MODELINDEX3;
        delta->modelindex3 = to->modelindex3;
    }
    if (to->modelindex4 != from->modelindex4)
    {
        delta->delta_bits |= Q2P_ESD_MODELINDEX4;
        delta->modelindex4 = to->modelindex4;
    }

    if (to->sound != from->sound)
    {
        delta->delta_bits |= Q2P_ESD_SOUND;
        delta->sound = to->sound;
    }

#if Q2PROTO_ENTITY_STATE_FEATURES >= Q2PROTO_FEATURES_Q2PRO_EXTENDED
    if (extended_state)
    {
        if (to->loop_volume != from->loop_volume)
        {
            delta->delta_bits |= Q2P_ESD_LOOP_VOLUME;
            delta->loop_volume = to->loop_volume;
        }
        if (to->loop_attenuation != from->loop_attenuation)
        {
            delta->delta_bits |= Q2P_ESD_LOOP_ATTENUATION;
            delta->loop_attenuation = to->loop_attenuation;
        }

        if (to->alpha != from->alpha)
        {
            delta->delta_bits |= Q2P_ESD_ALPHA;
            delta->alpha = to->alpha;
        }
        if (to->scale != from->scale)
        {
            delta->delta_bits |= Q2P_ESD_SCALE;
            delta->scale = to->scale;
        }
    }
#endif
}

void q2proto_packing_make_player_state_delta(const q2proto_packed_player_state_t *from, const q2proto_packed_player_state_t *to, q2proto_svc_playerstate_t *delta)
{
    memset(delta, 0, sizeof(*delta));

    if (!from)
        from = &q2proto_null_packed_player_state;

    if (to->pm_type != from->pm_type)
    {
        delta->delta_bits |= Q2P_PSD_PM_TYPE;
        delta->pm_type = to->pm_type;
    }

    q2proto_var_coord_set_int(&delta->pm_origin.write.prev, from->pm_origin);
    q2proto_var_coord_set_int(&delta->pm_origin.write.current, to->pm_origin);
    q2proto_var_coord_set_int(&delta->pm_velocity.write.prev, from->pm_velocity);
    q2proto_var_coord_set_int(&delta->pm_velocity.write.current, to->pm_velocity);

    if (to->pm_time != from->pm_time)
    {
        delta->delta_bits |= Q2P_PSD_PM_TIME;
        delta->pm_time = to->pm_time;
    }

    if (to->pm_flags != from->pm_flags)
    {
        delta->delta_bits |= Q2P_PSD_PM_FLAGS;
        delta->pm_flags = to->pm_flags;
    }

    if (to->pm_gravity != from->pm_gravity)
    {
        delta->delta_bits |= Q2P_PSD_PM_GRAVITY;
        delta->pm_gravity = to->pm_gravity;
    }

    if (memcmp(&to->pm_delta_angles, &from->pm_delta_angles, sizeof(to->pm_delta_angles)) != 0)
    {
        delta->delta_bits |= Q2P_PSD_PM_DELTA_ANGLES;
        q2proto_var_angle_set_short(&delta->pm_delta_angles, to->pm_delta_angles);
    }

#if Q2PROTO_PLAYER_STATE_FEATURES >= Q2PROTO_FEATURES_RERELEASE
    if (to->pm_viewheight != from->pm_viewheight)
    {
        delta->delta_bits |= Q2P_PSD_PM_VIEWHEIGHT;
        delta->pm_viewheight = to->pm_viewheight;
    }
#endif

    if (memcmp(to->viewoffset, from->viewoffset, sizeof(to->viewoffset)) != 0)
    {
        delta->delta_bits |= Q2P_PSD_VIEWOFFSET;
        q2proto_var_small_offset_set_char_comp(&delta->viewoffset, 0, to->viewoffset[0]);
        q2proto_var_small_offset_set_char_comp(&delta->viewoffset, 1, to->viewoffset[1]);
        q2proto_var_small_offset_set_char_comp(&delta->viewoffset, 2, to->viewoffset[2]);
    }

    Q2PROTO_SET_ANGLE_DELTA(delta->viewangles, to->viewangles, from->viewangles, short);

    if (memcmp(to->kick_angles, from->kick_angles, sizeof(to->kick_angles)))
    {
        delta->delta_bits |= Q2P_PSD_KICKANGLES;
        q2proto_var_small_angle_set_char_comp(&delta->kick_angles, 0, to->kick_angles[0]);
        q2proto_var_small_angle_set_char_comp(&delta->kick_angles, 1, to->kick_angles[1]);
        q2proto_var_small_angle_set_char_comp(&delta->kick_angles, 2, to->kick_angles[2]);
    }

    for (int c = 0; c < 4; c++)
    {
        if (to->blend[c] != from->blend[c])
        {
            q2proto_var_blend_set_byte_comp(&delta->blend.values, c, to->blend[c]);
            delta->blend.delta_bits |= BIT(c);
        }
    #if Q2PROTO_PLAYER_STATE_FEATURES >= Q2PROTO_FEATURES_Q2PRO_EXTENDED_V2
        if (to->damage_blend[c] != from->damage_blend[c])
        {
            q2proto_var_blend_set_byte_comp(&delta->damage_blend.values, c, to->damage_blend[c]);
            delta->damage_blend.delta_bits |= BIT(c);
        }
    #endif
    }

    if (to->fov != from->fov)
    {
        delta->delta_bits |= Q2P_PSD_FOV;
        delta->fov = to->fov;
    }

    if (to->rdflags != from->rdflags)
    {
        delta->delta_bits |= Q2P_PSD_RDFLAGS;
        delta->rdflags = to->rdflags;
    }

    if (to->gunframe != from->gunframe)
        delta->delta_bits |= Q2P_PSD_GUNFRAME;
    if (memcmp(to->gunoffset, from->gunoffset, sizeof(to->gunoffset)))
        delta->delta_bits |= Q2P_PSD_GUNOFFSET;
    if (memcmp(to->gunangles, from->gunangles, sizeof(to->gunangles)))
        delta->delta_bits |= Q2P_PSD_GUNANGLES;
    if (delta->delta_bits & (Q2P_PSD_GUNFRAME | Q2P_PSD_GUNOFFSET | Q2P_PSD_GUNANGLES))
    {
        delta->gunframe = to->gunframe;
        q2proto_var_small_offset_set_char_comp(&delta->gunoffset, 0, to->gunoffset[0]);
        q2proto_var_small_offset_set_char_comp(&delta->gunoffset, 1, to->gunoffset[1]);
        q2proto_var_small_offset_set_char_comp(&delta->gunoffset, 2, to->gunoffset[2]);
        q2proto_var_small_angle_set_char_comp(&delta->gunangles, 0, to->gunangles[0]);
        q2proto_var_small_angle_set_char_comp(&delta->gunangles, 1, to->gunangles[1]);
        q2proto_var_small_angle_set_char_comp(&delta->gunangles, 2, to->gunangles[2]);
    }

    if (to->gunindex != from->gunindex)
        delta->delta_bits |= Q2P_PSD_GUNINDEX;
#if Q2PROTO_PLAYER_STATE_FEATURES >= Q2PROTO_FEATURES_Q2PRO_EXTENDED
    if (to->gunskin != from->gunskin)
        delta->delta_bits |= Q2P_PSD_GUNSKIN;
#endif
    if(delta->delta_bits & (Q2P_PSD_GUNINDEX | Q2P_PSD_GUNSKIN))
    {
        delta->gunindex = to->gunindex;
    #if Q2PROTO_PLAYER_STATE_FEATURES >= Q2PROTO_FEATURES_Q2PRO_EXTENDED
        delta->gunskin = to->gunskin;
    #endif
    }

    for (int i = 0; i < Q2PROTO_STATS; i++)
    {
        if (to->stats[i] != from->stats[i])
        {
            delta->statbits |= BIT_ULL(i);
            delta->stats[i] = to->stats[i];
        }
    }
}

void _q2proto_pack_entity_state_dispatch(q2proto_servercontext_t *context, const _q2proto_packing_entity_dispatch_t *dispatch, const void *entity_state_p, q2proto_packed_entity_state_t *entity_packed)
{
    if (context->protocol == Q2P_PROTOCOL_Q2REPRO)
        dispatch->q2repro(entity_state_p, entity_packed);
    else
        dispatch->vanilla(entity_state_p, context->server_info->game_type, entity_packed);
}

void _q2proto_pack_player_state_dispatch(q2proto_servercontext_t *context, const _q2proto_packing_player_dispatch_t *dispatch, const void *player_state_p, q2proto_packed_player_state_t *player_packed)
{
    if (context->protocol == Q2P_PROTOCOL_Q2REPRO)
        dispatch->q2repro(player_state_p, player_packed);
    else
        dispatch->vanilla(player_state_p, player_packed);
}
