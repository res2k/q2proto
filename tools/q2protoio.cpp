/*
Copyright (C) 2026 Frank Richter

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

#include "q2protoio.hpp"

#include <bit>
#include <cstdio>
#include <cstring>
#include <fmt/format.h>
#include <utility>

#include "q2proto/q2proto.h"

#include <zlib.h>

extern "C" q2proto_error_t q2protoio_get_error(uintptr_t io_arg)
{
    auto *io_ctx = reinterpret_cast<io_context *>(io_arg);
    return io_ctx->err;
}

static bool context_read(io_context* io_ctx, void* buf, size_t size)
{
    auto max_read = std::min(size, size_t(io_ctx->size - io_ctx->pos));
    memcpy(buf, io_ctx->data + io_ctx->pos, max_read);
    io_ctx->pos += max_read;
    if (max_read < size) {
        io_ctx->err = Q2P_ERR_IO_READ;
        return false;
    }
    return true;
}

extern "C" uint8_t q2protoio_read_u8(uintptr_t io_arg)
{
    auto *io_ctx = reinterpret_cast<io_context *>(io_arg);
    auto u = (uint8_t)-1;
    context_read(io_ctx, &u, sizeof(u));
    return u;
}

extern "C" uint16_t q2protoio_read_u16(uintptr_t io_arg)
{
    auto *io_ctx = reinterpret_cast<io_context *>(io_arg);
    auto u = (uint16_t)-1;
    context_read(io_ctx, &u, sizeof(u));
    if constexpr (std::endian::native != std::endian::little)
        u = std::byteswap(u);
    return u;
}

extern "C" uint32_t q2protoio_read_u32(uintptr_t io_arg)
{
    auto *io_ctx = reinterpret_cast<io_context *>(io_arg);
    auto u = (uint32_t)-1;
    context_read(io_ctx, &u, sizeof(u));
    if constexpr (std::endian::native != std::endian::little)
        u = std::byteswap(u);
    return u;
}

extern "C" uint64_t q2protoio_read_u64(uintptr_t io_arg)
{
    auto *io_ctx = reinterpret_cast<io_context *>(io_arg);
    auto u = (uint64_t)-1;
    context_read(io_ctx, &u, sizeof(u));
    if constexpr (std::endian::native != std::endian::little)
        u = std::byteswap(u);
    return u;
}

extern "C" q2proto_string_t q2protoio_read_string(uintptr_t io_arg)
{
    auto *io_ctx = reinterpret_cast<io_context *>(io_arg);
    auto str = q2proto_string_t{.str = nullptr, .len = 0};
    if (io_ctx->pos >= io_ctx->size) {
        io_ctx->err = Q2P_ERR_IO_READ;
        return str;
    }
    str.str = reinterpret_cast<const char *>(io_ctx->data + io_ctx->pos);
    while(io_ctx->pos < io_ctx->size) {
        auto c = str.str[str.len];
        ++io_ctx->pos;
        if (c == 0)
            return str;
        ++str.len;
    }
    io_ctx->err = Q2P_ERR_IO_READ;
    return str;
}

extern "C" const void *q2protoio_read_raw(uintptr_t io_arg, size_t size, size_t *readcount)
{
    auto *io_ctx = reinterpret_cast<io_context *>(io_arg);
    size_t max_read = std::min(size, size_t(io_ctx->size - io_ctx->pos));
    const void *ptr = io_ctx->data + io_ctx->pos;
    io_ctx->pos += max_read;
    if (readcount)
        *readcount = max_read;
    else if (max_read < size)
        io_ctx->err = Q2P_ERR_IO_READ;
    return ptr;
}

extern "C" size_t q2protoio_read_available(uintptr_t io_arg)
{
    auto *io_ctx = reinterpret_cast<io_context *>(io_arg);
    return io_ctx->size - io_ctx->pos;
}

static inline void q2p_inflate_deflate_error(const char* message, int z_error)
{
    fmt::println(stderr, "{}: {}", message, zError(z_error));
}

struct inflate_io_context : public io_context
{
    z_stream z = {};
    std::byte buffer[0x10000];
    bool stream_end = false;

    inflate_io_context() : io_context(buffer, 0) {}
    bool is_inflate() const override { return true; }
};

extern "C" {
#define Q2PROTO_INFLATE_IMPL_HELPER_API static inline

// #include "q2proto/q2proto_deflate_impl_helper.inc"
#include "q2proto/q2proto_inflate_impl_helper.inc"
} // extern "C"

extern "C" q2proto_error_t q2protoio_inflate_begin(uintptr_t io_arg, q2proto_inflate_deflate_header_mode_t header_mode, uintptr_t* inflate_io_arg)
{
    auto *io_ctx = reinterpret_cast<io_context *>(io_arg);
    if (io_ctx->is_inflate())
        return Q2P_ERR_INVALID_ARGUMENT;

    auto *new_ctx = new inflate_io_context;
    q2proto_error_t err = q2proto_inflate_impl_helper_begin(header_mode, &new_ctx->z);

    *inflate_io_arg = reinterpret_cast<uintptr_t>(new_ctx);
    return err;
}

extern "C" q2proto_error_t q2protoio_inflate_data(uintptr_t io_arg, uintptr_t inflate_io_arg, size_t compressed_size)
{
    auto *io_ctx = reinterpret_cast<io_context *>(io_arg);
    auto *inflate_io_ctx = reinterpret_cast<inflate_io_context *>(inflate_io_arg);
    if (io_ctx->is_inflate() || !inflate_io_ctx->is_inflate())
        return Q2P_ERR_INVALID_ARGUMENT;

    const std::byte *in_data = nullptr;
    if (compressed_size == (size_t)-1) {
        in_data = io_ctx->data + io_ctx->pos;
        compressed_size = io_ctx->size - io_ctx->pos;
        io_ctx->pos += io_ctx->size;
    } else {
        if (io_ctx->size - io_ctx->pos < compressed_size)
            return Q2P_ERR_IO_READ;
        in_data = io_ctx->data + io_ctx->pos;
        io_ctx->pos += compressed_size;
    }

    unsigned long uncompressed_size = 0;
    q2proto_error_t err = q2proto_inflate_impl_helper_data(&inflate_io_ctx->z, in_data, (uint32_t)compressed_size,
                                                           &inflate_io_ctx->buffer, sizeof(inflate_io_ctx->buffer),
                                                           &uncompressed_size, &inflate_io_ctx->stream_end);

    inflate_io_ctx->size = uncompressed_size;
    inflate_io_ctx->pos = 0;

    return err;
}

extern "C" q2proto_error_t q2protoio_inflate_stream_ended(uintptr_t inflate_io_arg, bool *stream_end)
{
    auto *inflate_io_ctx = reinterpret_cast<inflate_io_context *>(inflate_io_arg);
    if (!inflate_io_ctx->is_inflate())
        return Q2P_ERR_INVALID_ARGUMENT;
    *stream_end = inflate_io_ctx->stream_end;
    return Q2P_ERR_SUCCESS;
}

extern "C" q2proto_error_t q2protoio_inflate_end(uintptr_t inflate_io_arg)
{
    auto *inflate_io_ctx = reinterpret_cast<inflate_io_context *>(inflate_io_arg);
    if (!inflate_io_ctx->is_inflate())
        return Q2P_ERR_INVALID_ARGUMENT;
    q2proto_error_t err = q2proto_inflate_impl_helper_end(&inflate_io_ctx->z);
    if (err == Q2P_ERR_SUCCESS)
        err = inflate_io_ctx->pos < inflate_io_ctx->size ? Q2P_ERR_MORE_DATA_DEFLATED : Q2P_ERR_SUCCESS;
    delete inflate_io_ctx;
    return err;
}

#if 0
void Q2PROTO_deflate_args_init(q2protoio_deflate_args_t *deflate_args, byte *buffer, unsigned buffer_size, memtag_t z_stream_tag)
{
    q2proto_deflate_impl_helper_init(&deflate_args->defl, NULL, buffer, buffer_size);
}

void Q2PROTO_deflate_args_destroy(q2protoio_deflate_args_t *deflate_args)
{
    q2proto_deflate_impl_helper_destroy(&deflate_args->defl);
}

q2proto_error_t q2protoio_deflate_begin(q2protoio_deflate_args_t* deflate_args, size_t max_deflated, q2proto_inflate_deflate_header_mode_t header_mode, uintptr_t *deflate_io_arg)
{
    Q_assert(!deflate_q2protoio_ioarg.deflate);

    q2proto_error_t err = q2proto_deflate_impl_helper_begin(&deflate_args->defl, header_mode, deflate_buf);
    if (err != Q2P_ERR_SUCCESS)
        return err;

    SZ_InitWrite(&msg_deflate, deflate_buf, MAX_MSGLEN);

    deflate_q2protoio_ioarg.max_msg_len = max_deflated;
    deflate_q2protoio_ioarg.deflate = deflate_args;
    *deflate_io_arg = IOARG_DEFLATE;

    return Q2P_ERR_SUCCESS;
}

q2proto_error_t q2protoio_deflate_get_data(uintptr_t deflate_io_arg, q2proto_deflate_stream_mode_t stream_mode, size_t *in_size, const void **out, size_t *out_size)
{
    q2protoio_ioarg_t *io_data = (q2protoio_ioarg_t *)deflate_io_arg;
    q2protoio_deflate_args_t *deflate_args = io_data->deflate;

    q2proto_error_t err = q2proto_deflate_impl_helper_get_data(&deflate_args->defl, msg_deflate.cursize, stream_mode, in_size, out, out_size, deflate_buf);
    if (err != Q2P_ERR_SUCCESS)
        return err;

    SZ_Clear(&msg_deflate);

    return Q2P_ERR_SUCCESS;
}

q2proto_error_t q2protoio_deflate_end(uintptr_t deflate_io_arg)
{
    q2protoio_ioarg_t *io_data = (q2protoio_ioarg_t *)deflate_io_arg;

    io_data->deflate = NULL;

    return Q2P_ERR_SUCCESS;
}
#endif
