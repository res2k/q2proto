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
 * Stateful download helpers
 */
#ifndef Q2PROTO_INTERNAL_DOWNLOAD_H_
#define Q2PROTO_INTERNAL_DOWNLOAD_H_

#include "q2proto/q2proto.h"

/**\name Stateful download helpers
 * @{ */
/// Stateful download function table
struct q2proto_download_funcs_s {
    /// Begin download. For protocol-specific adjustments. Can be \c NULL, in which case q2proto_download_common_begin() is used.
    q2proto_error_t (*begin)(q2proto_servercontext_t *context, q2proto_server_download_state_t* state);
    /// Fill a download message with a chunk of data. \sa q2proto_server_download_data
    q2proto_error_t (*data)(q2proto_server_download_state_t *state, const uint8_t **data, size_t *remaining, size_t packet_remaining, q2proto_svc_download_t *svc_download);
    /// Fill a download message for a finished download. \sa q2proto_server_download_finished
    q2proto_error_t (*finish)(q2proto_server_download_state_t *state, q2proto_svc_download_t *svc_download);
    /// Fill a download message for an aborted download. \sa q2proto_server_download_aborted
    q2proto_error_t (*abort)(q2proto_server_download_state_t *state, q2proto_svc_download_t *svc_download);
};

/// Common "begin download" logic
void q2proto_download_common_begin(q2proto_servercontext_t *context, size_t total_size, q2proto_server_download_state_t* state);
/// Compute transferred size, completion percentage after preparing a chunk of data
q2proto_error_t q2proto_download_common_complete_struct(q2proto_server_download_state_t *state, size_t download_remaining, q2proto_svc_download_t *svc_download);
/// Default "data chunk" implementation (uncompressed only)
q2proto_error_t q2proto_download_common_data(q2proto_server_download_state_t *state, const uint8_t **data, size_t *remaining, size_t max_message_size, q2proto_svc_download_t *svc_download);
/// Default "finish download" implementation
q2proto_error_t q2proto_download_common_finish(q2proto_server_download_state_t *state, q2proto_svc_download_t *svc_download);
/// Default "abort download" implementation
q2proto_error_t q2proto_download_common_abort(q2proto_server_download_state_t *state, q2proto_svc_download_t *svc_download);
/** @} */

#endif // Q2PROTO_INTERNAL_DOWNLOAD_H_
