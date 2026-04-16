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

#include "q2proto/q2proto.h"

#include <cstdarg>
#include <cstdio>
#include <fmt/format.h>

extern "C" bool q2protodbg_shownet_check(uintptr_t io_arg, int level) { return true; }

extern "C" void q2protodbg_shownet(uintptr_t io_arg, int level, int offset, const char *msg, ...)
{
    auto *io_ctx = reinterpret_cast<const io_context *>(io_arg);

    char buf[256];
    va_list argptr;

    const char *offset_suffix = io_ctx->is_inflate() ? "[z]" : "";

    va_start(argptr, msg);
    vsnprintf(buf, sizeof(buf), msg, argptr);
    va_end(argptr);

    fmt::println("{:5}{}:{}", io_ctx->pos + offset, offset_suffix, buf);
}
