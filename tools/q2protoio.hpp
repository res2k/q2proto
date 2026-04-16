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

#ifndef Q2PROTOIO_HPP_
#define Q2PROTOIO_HPP_

#include <cstddef>
#include <cstdint>

#include "q2proto/q2proto_error.h"

struct io_context
{
    const std::byte *data;
    uint32_t size;
    uint32_t pos = 0;
    q2proto_error_t err = Q2P_ERR_SUCCESS;

    io_context(const std::byte *data, uint32_t size) : data(data), size(size) {}
    io_context(io_context &&) = delete;
    io_context(const io_context &) = delete;
    virtual ~io_context() {}
    virtual bool is_inflate() const { return false; }

    io_context& operator=(io_context &&) = delete;
    io_context& operator=(const io_context &) = delete;
};

#endif // Q2PROTOIO_HPP_
