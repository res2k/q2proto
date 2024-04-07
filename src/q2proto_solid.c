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
#include "q2proto/q2proto_solid.h"

#include "q2proto_internal_common.h"
#include "q2proto_internal_defs.h"

// Assumes x/y are equal and symmetric. Z does not have to be symmetric, and z maxs can be negative.
uint16_t q2proto_pack_solid_16(const q2proto_vec3_t mins, const q2proto_vec3_t maxs)
{
    int x = maxs[0] / 8;
    int zd = -mins[2] / 8;
    int zu = (maxs[2] + 32) / 8;

    x = CLAMP(x, 1, 31);
    zd = CLAMP(zd, 1, 31);
    zu = CLAMP(zu, 1, 63);

    return (zu << 10) | (zd << 5) | x;
}

void q2proto_unpack_solid_16(uint16_t solid, q2proto_vec3_t mins, q2proto_vec3_t maxs)
{
    int x = 8 * (solid & 31);
    int zd = 8 * ((solid >> 5) & 31);
    int zu = 8 * ((solid >> 10) & 63) - 32;

    mins[0] = -x;
    mins[1] = -x;
    mins[2] = -zd;
    maxs[0] =  x;
    maxs[1] =  x;
    maxs[2] =  zu;
}
