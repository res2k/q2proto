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

/**\file
 * Implementation helpers for q2protoio_inflate_* functions.
 */
#ifndef Q2PROTO_INFLATE_IMPL_HELPER_H_
#define Q2PROTO_INFLATE_IMPL_HELPER_H_

/**\name Implementation helpers for q2protoio_inflate_* functions.
 *
 * Although not quite drop-in implemetions, they allow the io_inflate
 * functions to be written using relatively thin wrappers.
 *
 * The implementations for the functions declared in this header
 * are located in <tt>q2proto_inflate_impl_helper.inc</tt>, which
 * has to be included from somewhere in your source code.
 * (Alongside other q2proto glue code is probably a sensible place.)
 *
 * To emit error messages the <tt>q2proto_inflate_impl_helper_*</t> functions 
 * call <tt>void q2p_inflate_inflate_error(const char* message, int z_error)</tt>
 * which you'll also need to provide.
 *
 * These helpers are "bring your own zlib" - the headers for zlib,
 * or a compatible implementation such as miniz (https://github.com/richgel999/miniz),
 * have to be included _before_ this header or <tt>q2proto_inflate_impl_helper.inc</tt>.
 *
 * The helper functions operate on a zstream configured for inflation.
 * - \c q2protoio_inflate_begin() calls \c q2proto_inflate_impl_helper_begin()
 *   to configure the zstream.
 * - \c q2protoio_inflate_get_data() calls \c q2proto_inflate_impl_helper_data()
 *   to obtain the uncompressed data and some extra values.
 * - \c q2protoio_inflate_end() calls \c q2proto_inflate_impl_helper_end() to
 *   clean up the zstream.
 *
 * @{
 */

/**\def Q2PROTO_INFLATE_IMPL_HELPER_API
 * Macro used to declare \c q2proto_inflate_impl_helper_* functions.
 * Can be set to eg <tt>static</tt> when including as a single source,
 * multiple times, or tweaking external visibility (\c dllimport,
 * \c dllexport, \c "visibility" attribute and the likes).
 */
#if !defined(Q2PROTO_INFLATE_IMPL_HELPER_API)
#define Q2PROTO_INFLATE_IMPL_HELPER_API
#endif

/**
 * Initialize inflate decompression.
 * \param header_mode Whether to expect a header in compressed data. Passed through from q2protoio_inflate_begin().
 * \param z Stream to use for decompression.
 * \returns Error code
 */
Q2PROTO_INFLATE_IMPL_HELPER_API q2proto_error_t
q2proto_inflate_impl_helper_begin(q2proto_inflate_deflate_header_mode_t header_mode, z_streamp z);
/**
 * Inflate some data.
 * \param z Stream to use for decompression.
 * \param compressed_data Pointer to buffer containing compressed (input) data.
 * \param compressed_size Size of compressed (input) data.
 * \param out_buffer Pointer to buffer to receive uncompressed (output) data.
 * \param out_buffer_size Size of output buffer.
 * \param uncompressed_size Will receive the amount of uncompressed data produced.
 * \param stream_end Will receive flag whether the end of the compressed data has been reached.
 * \returns Error code
 */
Q2PROTO_INFLATE_IMPL_HELPER_API q2proto_error_t
q2proto_inflate_impl_helper_data(z_streamp z, const void *compressed_data, uint32_t compressed_size, void *out_buffer,
                                 uint32_t out_buffer_size, unsigned long* uncompressed_size, bool *stream_end);
/**
 * End inflation.
 * \param z Stream to use for decompression.
 * \returns Error code
 */
Q2PROTO_INFLATE_IMPL_HELPER_API q2proto_error_t q2proto_inflate_impl_helper_end(z_streamp z);

/** @} */

#endif // Q2PROTO_INFLATE_IMPL_HELPER_H_
