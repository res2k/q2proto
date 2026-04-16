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

#include "q2proto_inflate_impl_helper.h"

Q2PROTO_INFLATE_IMPL_HELPER_API q2proto_error_t q2proto_inflate_impl_helper_begin(q2proto_inflate_deflate_header_mode_t header_mode, z_streamp z)
{
    int window_bits = header_mode == Q2P_INFL_DEFL_RAW ? -MAX_WBITS : MAX_WBITS;
    int ret;
    if (z->state)
        ret = inflateReset2(z, window_bits);
    else
        ret = inflateInit2(z, window_bits);
    if (ret != Z_OK) {
        q2p_inflate_deflate_error("inflate initialization failed", ret);
        return Q2P_ERR_INFLATE_FAILED;
    }
    return Q2P_ERR_SUCCESS;
}

Q2PROTO_INFLATE_IMPL_HELPER_API q2proto_error_t
q2proto_inflate_impl_helper_data(z_streamp z, const void *compressed_data, uint32_t compressed_size, void *out_buffer,
                                 uint32_t out_buffer_size, unsigned long* uncompressed_size, bool *stream_end)
{
    z->next_in = (Byte*)compressed_data;
    z->avail_in = compressed_size;
    z->next_out = out_buffer;
    z->avail_out = out_buffer_size;
    int ret = inflate(z, Z_SYNC_FLUSH);
    if (ret != Z_OK && ret != Z_STREAM_END && ret != Z_BUF_ERROR) {
        inflateEnd(z);
        q2p_inflate_deflate_error("inflate() failed", ret);
        return Q2P_ERR_INFLATE_FAILED;
    }

    *uncompressed_size = out_buffer_size - z->avail_out;
    *stream_end = ret == Z_STREAM_END;

    return Q2P_ERR_SUCCESS;
}

Q2PROTO_INFLATE_IMPL_HELPER_API q2proto_error_t q2proto_inflate_impl_helper_end(z_streamp z)
{
    int ret = inflateEnd(z);
    if (ret != Z_OK && ret != Z_STREAM_END) {
        q2p_inflate_deflate_error("inflateEnd() failed", ret);
        return Q2P_ERR_INFLATE_FAILED;
    }
    return Q2P_ERR_SUCCESS;
}
