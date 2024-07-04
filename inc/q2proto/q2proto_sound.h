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
 * Sound convenience functions
 */
#ifndef Q2PROTO_SOUND_H_
#define Q2PROTO_SOUND_H_

#include "q2proto_coords.h"
#include "q2proto_struct_svc.h"

#include <stdbool.h>

/**\name Sound messages
 * @{ */
/// Sound parameters, as typically stored internally by engines
typedef struct q2proto_sound_s
{
    bool has_entity_channel;
    bool has_position;
    int index;
    int entity;
    int channel;
    q2proto_vec3_t pos;
    float volume;
    float attenuation;
    float timeofs;
} q2proto_sound_t;

/// "Decode" a sound message into easier to use sound info.
void q2proto_sound_decode_message(const q2proto_svc_sound_t *sound_msg, q2proto_sound_t *sound_data);
/**
 * "Encode" sound info into a sound message.
 * \note Does some clamping internally, but does not range-check input values!
 * Especially \c index must be in range (<= 255) on "non-extended" servers,
 * as otherwise a broken message would be written.
 */
void q2proto_sound_encode_message(const q2proto_sound_t *sound_data, q2proto_svc_sound_t *sound_msg);
/** @} */

#endif // Q2PROTO_SOUND_H_
