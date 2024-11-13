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
 * Provide implementation of an entity state packing function.
 * To generate a packing function, the following macros \em must be define beforehand:
 * - #Q2P_PACK_ENTITY_FUNCTION_NAME
 * - #Q2P_PACK_ENTITY_TYPE
 * The following macro can be defined to customize retrieval of entity state fields:
 * - #Q2P_PACK_GET_ENTITY_VALUE
 */
#include "q2proto.h"

#include "q2proto_packing_helpers.h"

/**\def Q2P_PACK_ENTITY_FUNCTION_NAME
 * Name of generated entity state packing function.
 */
#if !defined(Q2P_PACK_ENTITY_FUNCTION_NAME)
#error Please define Q2P_PACK_ENTITY_FUNCTION_NAME.
#endif

/**\def Q2P_PACK_ENTITY_TYPE
 * Entity state type to pack.
 */
#if !defined(Q2P_PACK_ENTITY_TYPE)
#error Please define Q2P_PACK_ENTITY_TYPE.
#endif

// Prototype to avoid "no previous prototype" warnings
void Q2P_PACK_ENTITY_FUNCTION_NAME(q2proto_servercontext_t *context, const Q2P_PACK_ENTITY_TYPE entity_state, q2proto_packed_entity_state_t *entity_packed);

/**\def Q2P_PACK_GET_ENTITY_VALUE
 * Extract a member from entity packing input data.
 */
#if !defined(Q2P_PACK_GET_ENTITY_VALUE)
#define Q2P_PACK_GET_ENTITY_VALUE(ENTITY, MEMBER)               ((ENTITY)->MEMBER)
#define _Q2P_PACK_GET_ENTITY_VALUE_DEFAULTED
#endif // !defined(Q2P_PACK_GET_ENTITY_VALUE)

#define _Q2P_PACK_ENTITY_VANILLA_FUNCTION_NAME   _Q2PROTO_PACKING_NAME(Q2P_PACK_ENTITY_FUNCTION_NAME, _int)
#define _Q2P_PACK_ENTITY_Q2REPRO_FUNCTION_NAME   _Q2PROTO_PACKING_NAME(Q2P_PACK_ENTITY_FUNCTION_NAME, _float)

#define _Q2PROTO_PACKING_CAST_ENTITYSTATE       (*((const Q2P_PACK_ENTITY_TYPE*)entity_state_p))

static void _Q2P_PACK_ENTITY_VANILLA_FUNCTION_NAME(const void * restrict entity_state_p, q2proto_game_type_t game_type, q2proto_packed_entity_state_t * restrict entity_packed)
{
    memset(entity_packed, 0, sizeof(*entity_packed));

    entity_packed->modelindex = (uint16_t)Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, modelindex);
    entity_packed->modelindex2 = (uint16_t)Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, modelindex2);
    entity_packed->modelindex3 = (uint16_t)Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, modelindex3);
    entity_packed->modelindex4 = (uint16_t)Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, modelindex4);
    entity_packed->frame = (uint16_t)Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, frame);
    entity_packed->skinnum = (uint32_t)Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, skinnum);
    entity_packed->effects = Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, effects);
    entity_packed->renderfx = (uint32_t)Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, renderfx);
    _Q2P_PACKING_PACK_COORD_VEC_TO_INT(entity_packed->origin, Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, origin));
    _Q2P_PACKING_PACK_ANGLE_VEC_TO_INT(entity_packed->angles, Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, angles));
    _Q2P_PACKING_PACK_COORD_VEC_TO_INT(entity_packed->old_origin, Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, old_origin));
    entity_packed->sound = (uint16_t)Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, sound);
    entity_packed->event = (uint8_t)Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, event);
    entity_packed->solid = (uint32_t)Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, solid);
#if Q2PROTO_ENTITY_STATE_FEATURES >= Q2PROTO_FEATURES_Q2PRO_EXTENDED
    if (game_type >= Q2PROTO_GAME_Q2PRO_EXTENDED)
    {
        entity_packed->loop_volume = _q2proto_valenc_clamped_mul(Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, loop_volume), 255, 0, UINT8_MAX);
        entity_packed->loop_attenuation = q2proto_sound_encode_loop_attenuation(Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, loop_attenuation));
        entity_packed->alpha = _q2proto_valenc_entityalpha2byte(Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, alpha));
        entity_packed->scale = _q2proto_valenc_entityscale2byte(Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, scale));
    }
#endif
}

static void _Q2P_PACK_ENTITY_Q2REPRO_FUNCTION_NAME(const void * restrict entity_state_p, q2proto_packed_entity_state_t * restrict entity_packed)
{
    memset(entity_packed, 0, sizeof(*entity_packed));

    entity_packed->modelindex = (uint16_t)Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, modelindex);
    entity_packed->modelindex2 = (uint16_t)Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, modelindex2);
    entity_packed->modelindex3 = (uint16_t)Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, modelindex3);
    entity_packed->modelindex4 = (uint16_t)Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, modelindex4);
    entity_packed->frame = (uint16_t)Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, frame);
    entity_packed->skinnum = (uint32_t)Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, skinnum);
    entity_packed->effects = Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, effects);
    entity_packed->renderfx = (uint32_t)Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, renderfx);
    _Q2P_PACKING_PACK_COORD_VEC_TO_FLOATBITS(entity_packed->origin, Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, origin));
    _Q2P_PACKING_PACK_ANGLE_VEC_TO_INT(entity_packed->angles, Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, angles));
    _Q2P_PACKING_PACK_COORD_VEC_TO_FLOATBITS(entity_packed->old_origin, Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, old_origin));
    entity_packed->sound = (uint16_t)Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, sound);
    entity_packed->event = (uint8_t)Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, event);
    entity_packed->solid = (uint32_t)Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, solid);
#if Q2PROTO_ENTITY_STATE_FEATURES >= Q2PROTO_FEATURES_Q2PRO_EXTENDED
    entity_packed->loop_volume = _q2proto_valenc_clamped_mul(Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, loop_volume), 255, 0, UINT8_MAX);
    entity_packed->loop_attenuation = q2proto_sound_encode_loop_attenuation(Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, loop_attenuation));
    entity_packed->alpha = _q2proto_valenc_entityalpha2byte(Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, alpha));
    entity_packed->scale = _q2proto_valenc_entityscale2byte(Q2P_PACK_GET_ENTITY_VALUE(_Q2PROTO_PACKING_CAST_ENTITYSTATE, scale));
#endif
}

#undef _Q2PROTO_PACKING_CAST_ENTITYSTATE

void Q2P_PACK_ENTITY_FUNCTION_NAME(q2proto_servercontext_t *context, const Q2P_PACK_ENTITY_TYPE entity_state, q2proto_packed_entity_state_t *entity_packed)
{
    q2proto_game_type_t game_type;
    switch(_q2proto_get_packing_flavor(context, &game_type))
    {
    case _Q2P_PACKING_VANILLA:
        // Fall through to default case
        break;
    case _Q2P_PACKING_REPRO:
        _Q2P_PACK_ENTITY_Q2REPRO_FUNCTION_NAME(entity_state, entity_packed);
        return;
    }
    _Q2P_PACK_ENTITY_VANILLA_FUNCTION_NAME(entity_state, game_type, entity_packed);
}

#undef _Q2P_PACK_ENTITY_VANILLA_FUNCTION_NAME
#undef _Q2P_PACK_ENTITY_Q2REPRO_FUNCTION_NAME

#if defined(_Q2P_PACK_GET_ENTITY_VALUE_DEFAULTED)
#undef _Q2P_PACK_GET_ENTITY_VALUE_DEFAULTED
#undef Q2P_PACK_GET_ENTITY_VALUE
#endif // defined(_Q2P_PACK_GET_ENTITY_VALUE_DEFAULTED)
