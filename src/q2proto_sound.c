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
#include "q2proto/q2proto_sound.h"

#include "q2proto_internal_defs.h"
#include "q2proto_internal_protocol.h"

void q2proto_sound_decode_message(const q2proto_svc_sound_t *sound_msg, q2proto_sound_t *sound_data)
{
    memset(sound_data, 0, sizeof(*sound_data));

    sound_data->index = sound_msg->index;
    if (sound_msg->flags & SND_ENT)
    {
        sound_data->has_entity_channel = true;
        sound_data->entity = sound_msg->entity;
        sound_data->channel = sound_msg->channel;
    }
    if (sound_msg->flags & SND_POS)
    {
        sound_data->has_position = true;
        q2proto_var_coord_get_float(&sound_msg->pos, sound_data->pos);
    }
    if (sound_msg->flags & SND_VOLUME)
        sound_data->volume = sound_msg->volume / 255.f;
    else
        sound_data->volume = SOUND_DEFAULT_VOLUME / 255.f;

    if (sound_msg->flags & SND_ATTENUATION)
        sound_data->attenuation = sound_msg->attenuation / 64.f;
    else
        sound_data->attenuation = SOUND_DEFAULT_ATTENUATION / 64.f;

    if (sound_msg->flags & SND_OFFSET)
        sound_data->timeofs = sound_msg->timeofs / 1000.f;
}

void q2proto_sound_encode_message(const q2proto_sound_t *sound_data, q2proto_svc_sound_t *sound_msg)
{
    memset(sound_msg, 0, sizeof(*sound_msg));

    sound_msg->index = sound_data->index;
    if (sound_data->has_entity_channel)
    {
        sound_msg->entity = sound_data->entity;
        sound_msg->channel = sound_data->channel;
        sound_msg->flags |= SND_ENT;
    }
    if (sound_data->has_position)
    {
        q2proto_var_coord_set_float(&sound_msg->pos, sound_data->pos);
        sound_msg->flags |= SND_POS;
    }
    int volume = sound_data->volume * 255;
    uint8_t volume_enc = (uint8_t)CLAMP(volume, 0, 255);
    if (volume_enc != SOUND_DEFAULT_VOLUME)
    {
        sound_msg->volume = volume_enc;
        sound_msg->flags |= SND_VOLUME;
    }
    int attn = sound_data->attenuation * 64;
    uint8_t attn_enc = (uint8_t)CLAMP(attn, 0, 255);
    if (attn_enc != SOUND_DEFAULT_ATTENUATION)
    {
        sound_msg->attenuation = attn_enc;
        sound_msg->flags |= SND_ATTENUATION;
    }
    int time = sound_data->timeofs * 1000;
    uint8_t time_enc = (uint8_t)CLAMP(time, 0, 255);
    if (time_enc != 0)
    {
        sound_msg->timeofs = time_enc;
        sound_msg->flags |= SND_OFFSET;
    }

}