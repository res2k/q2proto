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
 * Common protocol functions
 */
#ifndef Q2PROTO_INTERNAL_COMMON_H_
#define Q2PROTO_INTERNAL_COMMON_H_

#include "q2proto/q2proto.h"

/**\name Common protocol functions
 * @{ */
q2proto_error_t q2proto_common_client_read_entity_bits(uintptr_t io_arg, uint64_t *bits, uint16_t *entnum);
/// Debug helper: Return number of bytes occupied by given entity bits
int q2proto_common_entity_bits_size(uint64_t bits);

q2proto_error_t q2proto_common_client_read_muzzleflash(uintptr_t io_arg, q2proto_svc_muzzleflash_t *muzzleflash, uint16_t silenced_mask);
q2proto_error_t q2proto_common_client_read_temp_entity(uintptr_t io_arg, q2proto_svc_temp_entity_t *temp_entity);
q2proto_error_t q2proto_common_client_read_layout(uintptr_t io_arg, q2proto_svc_layout_t *layout);
q2proto_error_t q2proto_common_client_read_inventory(uintptr_t io_arg, q2proto_svc_inventory_t *inventory);
q2proto_error_t q2proto_common_client_read_sound(uintptr_t io_arg, q2proto_svc_sound_t *sound);
q2proto_error_t q2proto_common_client_read_print(uintptr_t io_arg, q2proto_svc_print_t *print);
q2proto_error_t q2proto_common_client_read_stufftext(uintptr_t io_arg, q2proto_svc_stufftext_t *stufftext);
q2proto_error_t q2proto_common_client_read_configstring(uintptr_t io_arg, q2proto_svc_configstring_t *configstring);
q2proto_error_t q2proto_common_client_read_centerprint(uintptr_t io_arg, q2proto_svc_centerprint_t *centerprint);
q2proto_error_t q2proto_common_client_read_download(uintptr_t io_arg, q2proto_svc_download_t *download);
/** @} */

#endif // Q2PROTO_INTERNAL_COMMON_H_
