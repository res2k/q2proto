/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2003-2024 Andrey Nazarov
Copyright (C) 2024-2026 Frank Richter

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

#include "q2proto_deflate_impl_helper.h"

Q2PROTO_DEFLATE_IMPL_HELPER_API void q2proto_deflate_impl_helper_init(q2proto_deflate_impl_helper_args_t *deflate_args,
                                                              const q2proto_deflate_impl_helper_alloc_t *alloc,
                                                              void *z_buffer, unsigned long z_buffer_size)
{
    memset(deflate_args, 0, sizeof(*deflate_args));

    if (alloc) {
        deflate_args->z_raw.zalloc = alloc->zalloc;
        deflate_args->z_raw.zfree = alloc->zfree;
        deflate_args->z_raw.opaque = alloc->alloc_arg;

        deflate_args->z_header.zalloc = alloc->zalloc;
        deflate_args->z_header.zfree = alloc->zfree;
        deflate_args->z_header.opaque = alloc->alloc_arg;
    }

    deflate_args->z_buffer = z_buffer;
    deflate_args->z_buffer_size = z_buffer_size;
}

Q2PROTO_DEFLATE_IMPL_HELPER_API void q2proto_deflate_impl_helper_destroy(q2proto_deflate_impl_helper_args_t *deflate_args)
{
    if (deflate_args->z_header.state)
        deflateEnd(&deflate_args->z_header);
    if (deflate_args->z_raw.state)
        deflateEnd(&deflate_args->z_raw);
}

static inline void _q2proto_deflate_impl_helper_reset_output(q2proto_deflate_impl_helper_args_t* deflate_args)
{
    deflate_args->z_current->next_out = deflate_args->z_buffer;
    deflate_args->z_current->avail_out = deflate_args->z_buffer_size;
    deflate_args->z_current->total_in = 0;
    deflate_args->z_current->total_out = 0;
}

static inline int _q2proto_deflate_impl_helper_setup_stream(q2proto_deflate_impl_helper_args_t* deflate_args, z_streamp stream, int window_bits)
{
    int err;
    if (!stream->state)
        err = deflateInit2(stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, window_bits, 9, Z_DEFAULT_STRATEGY);
    else
        err = deflateReset(stream);
    deflate_args->z_current = stream;
    return err;
}

Q2PROTO_DEFLATE_IMPL_HELPER_API q2proto_error_t q2proto_deflate_impl_helper_begin(q2proto_deflate_impl_helper_args_t* deflate_args, q2proto_inflate_deflate_header_mode_t header_mode, const void* input_start)
{
    int ret;
    if (header_mode == Q2P_INFL_DEFL_RAW)
        ret = _q2proto_deflate_impl_helper_setup_stream(deflate_args, &deflate_args->z_raw, -MAX_WBITS);
    else
        ret = _q2proto_deflate_impl_helper_setup_stream(deflate_args, &deflate_args->z_header, MAX_WBITS);

    if (ret != Z_OK) {
        q2p_inflate_deflate_error("deflate initialization failed", ret);
        return Q2P_ERR_INFLATE_FAILED;
    }
    _q2proto_deflate_impl_helper_reset_output(deflate_args);
    deflate_args->z_current->next_in = (Byte*)input_start;

    return Q2P_ERR_SUCCESS;
}

static inline int _q2proto_deflate_impl_helper_compress_accumulated(q2proto_deflate_impl_helper_args_t *deflate_args, size_t total_in_size, size_t* total_out_size)
{
    // Compress data accumulated in deflate_buf
    deflate_args->z_current->avail_in = total_in_size - deflate_args->z_current->total_in;

    int ret = deflate(deflate_args->z_current, Z_PARTIAL_FLUSH);
    if (ret != Z_OK && ret != Z_STREAM_END) {
        deflateEnd(deflate_args->z_current);
        q2p_inflate_deflate_error("deflate failed", ret);
    }
    *total_out_size = deflate_args->z_current->total_out;
    return ret;
}

#define _Q2PROTO_DEFLATE_IMPL_HELPER_OUTPUT_MARGIN   16

Q2PROTO_DEFLATE_IMPL_HELPER_API size_t q2proto_deflate_impl_helper_remaining(q2proto_deflate_impl_helper_args_t* deflate_args, size_t total_input, size_t max_output)
{
    size_t already_compressed = deflate_args->z_current->total_out;
    size_t yet_uncompressed = total_input - deflate_args->z_current->total_in;
    bool accumulated_input = yet_uncompressed > 0;
    size_t used_size = already_compressed + deflateBound(deflate_args->z_current, yet_uncompressed);
    size_t max_msg_len = max_output > _Q2PROTO_DEFLATE_IMPL_HELPER_OUTPUT_MARGIN ? max_output - _Q2PROTO_DEFLATE_IMPL_HELPER_OUTPUT_MARGIN : 0;
    size_t write_available = max_msg_len - (used_size < max_msg_len ? used_size : max_msg_len);
    if (write_available == 0 && accumulated_input)
    {
        // Actually compress yet-uncompressed data to get an "available" number closer to reality
        size_t already_compressed = 0;
        _q2proto_deflate_impl_helper_compress_accumulated(deflate_args, total_input, &already_compressed);
        write_available = max_msg_len - (already_compressed < max_msg_len ? already_compressed : max_msg_len);
    }
    return write_available;
}

Q2PROTO_DEFLATE_IMPL_HELPER_API q2proto_error_t q2proto_deflate_impl_helper_get_data(q2proto_deflate_impl_helper_args_t* deflate_args, uint32_t total_size, q2proto_deflate_stream_mode_t stream_mode, size_t *in_size, const void **out, size_t *out_size, const void* next_input)
{
    uint32_t input_remain = total_size - deflate_args->z_current->total_in;
    if (input_remain > 0 || stream_mode != Q2P_DEFLATE_DATA_STREAM) {
        deflate_args->z_current->avail_in = input_remain;
        int ret = deflate(deflate_args->z_current, stream_mode == Q2P_DEFLATE_DATA_STREAM ? Z_PARTIAL_FLUSH : Z_FINISH);
        if (ret != Z_OK && ret != Z_STREAM_END) {
            deflateEnd(deflate_args->z_current);
            q2p_inflate_deflate_error("deflate() failed", ret);
            return Q2P_ERR_DEFLATE_FAILED;
        }
    }

    if (in_size)
        *in_size = deflate_args->z_current->total_in;
    *out = deflate_args->z_buffer;
    *out_size = deflate_args->z_current->total_out;

    _q2proto_deflate_impl_helper_reset_output(deflate_args);
    deflate_args->z_current->next_in = (Byte*)next_input;

    return Q2P_ERR_SUCCESS;
}
