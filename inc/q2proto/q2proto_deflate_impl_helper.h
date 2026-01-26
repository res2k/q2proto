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
 * Implementation helpers for q2protoio_deflate_* functions.
 */
#ifndef Q2PROTO_DEFLATE_IMPL_HELPER_H_
#define Q2PROTO_DEFLATE_IMPL_HELPER_H_

/**\name Implementation helpers for q2protoio_deflate_* functions.
 *
 * Although not quite drop-in implemetions, they allow the io_deflate
 * functions to be written using relatively thin wrappers.
 *
 * The implementations for the functions declared in this header
 * are located in <tt>q2proto_deflate_impl_helper.inc</tt>, which
 * has to be included from somewhere in your source code.
 * (Alongside other q2proto glue code is probably a sensible place.)
 *
 * To emit error messages the <tt>q2proto_deflate_impl_helper_*</t> functions 
 * call <tt>void q2p_inflate_deflate_error(const char* message, int z_error)</tt>
 * which you'll also need to provide.
 *
 * These helpers are "bring your own zlib" - the headers for zlib,
 * or a compatible implementation such as miniz (https://github.com/richgel999/miniz),
 * have to be included _before_ this header or <tt>q2proto_deflate_impl_helper.inc</tt>.
 *
 * The helper functions operate on the q2proto_deflate_impl_helper_args_t struct,
 * this should be added as a member of the \c q2protoio_deflate_args_t struct
 * you need to define yourself.
 * - Before the helper struct is used, q2proto_deflate_impl_helper_init() needs to
 *   called. It doesn't have to happen in \c q2protoio_deflate_begin() as a helper
 *   struct can be used for multiple deflate operations (and indeed some overhead
 *   is saved in that case).
 * - \c q2protoio_deflate_begin() calls \c q2proto_deflate_impl_helper_begin().
 * - \c q2protoio_deflate_get_data() calls \c q2proto_deflate_impl_helper_get_data().
 * - \c q2protoio_deflate_end() doesn't need to call any helper.
 * - To obtain a bound of the amount of data that can still be compressed while
 *   fitting into the output buffer, \c q2proto_deflate_impl_helper_remaining()
 *   can be used. (Useful for implementing \c q2protoio_write_available().)
 * - Call \c q2proto_deflate_impl_helper_destroy() for cleanup.
 *
 * @{
 */

/**\def Q2PROTO_DEFLATE_IMPL_HELPER_API
 * Macro used to declare \c q2proto_deflate_impl_helper_* functions.
 * Can be set to eg <tt>static</tt> when including as a single source,
 * multiple times, or tweaking external visibility (\c dllimport,
 * \c dllexport, \c "visibility" attribute and the likes).
 */
#if !defined(Q2PROTO_DEFLATE_IMPL_HELPER_API)
#define Q2PROTO_DEFLATE_IMPL_HELPER_API
#endif

/// Optional memory allocation functions
typedef struct {
    /// Used for zlib internal allocations
    alloc_func zalloc;
    /// Used to free zlib internal allocations
    free_func zfree;
    /// Argument passed to allocation functions
    void *alloc_arg;
} q2proto_deflate_impl_helper_alloc_t;

/// \c q2protoio_deflate_* implementation helper state
typedef struct {
    /// Buffer to store deflated data
    void *z_buffer;
    /// Size of deflated data buffer
    unsigned long z_buffer_size;
    /**
     * Deflate stream (raw). Need separate stream since there's no deflateReset2() which could switch
     * the zlib header mode.
     */
    z_stream z_raw;
    /**
     * Deflate stream (headered). Need separate stream since there's no deflateReset2() which could switch
     * the zlib header mode.
     */
    z_stream z_header;
    /// Currently active stream
    z_streamp z_current;
} q2proto_deflate_impl_helper_args_t;

/**
 * Initialize a \c q2protoio_deflate_* implementation helper.
 * Prepares the internally used zlib streams.
 * \param deflate_args Implementation helper state structure.
 * \param alloc Optional zlib allocation functions used for internally created streams.
 * \param z_buffer Buffer to receive deflated output data. Must be valid for the lifetime of the state structure!
 * \param z_buffer_size Size of the deflated output data buffer. Using \c deflateBound() is a good way to obtain this.
 */
Q2PROTO_DEFLATE_IMPL_HELPER_API void q2proto_deflate_impl_helper_init(q2proto_deflate_impl_helper_args_t *deflate_args,
                                                                      const q2proto_deflate_impl_helper_alloc_t *alloc,
                                                                      void *z_buffer, unsigned long z_buffer_size);
/**
 * Destroy a \c q2protoio_deflate_* implementation helper.
 * Cleans up the internally used zlib streams.
 * \param deflate_args State structure to destroy.
 */
Q2PROTO_DEFLATE_IMPL_HELPER_API void q2proto_deflate_impl_helper_destroy(
    q2proto_deflate_impl_helper_args_t *deflate_args);
/**
 * Begin deflation.
 * \sa q2protoio_deflate_begin
 * \param deflate_args Implementation helper state structure.
 * \param header_mode Header mode, passed through from q2protoio_deflate_begin().
 * \param input_start Pointer to start of input buffer.
 * \returns Error code.
 */
Q2PROTO_DEFLATE_IMPL_HELPER_API q2proto_error_t
q2proto_deflate_impl_helper_begin(q2proto_deflate_impl_helper_args_t *deflate_args,
                                  q2proto_inflate_deflate_header_mode_t header_mode, const void *input_start);
/**
 * Return a (conservative) limit on how many bytes can still be deflated and still fit into the given
 * output size.
 * \note Due to the nature of compression, more data than returned may actually still fit.
 * For example, a call to \c q2proto_deflate_impl_helper_remaining() returned 100 remaining bytes;
 * yet, after writing 100 bytes, the next call reports 50 remaining bytes, due to compressibility of the
 * input.
 * \param deflate_args Implementation helper state structure.
 * \param total_input The \em total amount of input data available in the input buffer.
 * \param max_output The maximum number of bytes to produce.
 * \returns A (conservative) limit on how many bytes can still be deflated; may return 0
 *   if nothing more can be fit.
 */
Q2PROTO_DEFLATE_IMPL_HELPER_API size_t q2proto_deflate_impl_helper_remaining(
    q2proto_deflate_impl_helper_args_t *deflate_args, size_t total_input, size_t max_output);
/**
 * Retrieve deflated data.
 * Ensures all input data is compressed to the previously provided output buffer.
 * \sa q2protoio_deflate_get_data
 * \param deflate_args Implementation helper state structure.
 * \param total_size The \em total amount of input data available in the input buffer.
 * \param stream_mode Stream mode, passed through from q2protoio_deflate_get_data().
 * \param in_size Optional. Receives size of consumed input (uncompressed) data.
 *   Usually passed through from q2protoio_deflate_get_data().
 * \param out Pointer to start of output data. Changed to point after the last written output byte.
 *   Usually passed through from q2protoio_deflate_get_data().
 * \param out_size Amount of output data written. Usually passed through from q2protoio_deflate_get_data().
 * \param next_input Pointer to "next" input buffer, which will be consumed when
 *   q2proto_deflate_impl_helper_get_data() is subsequently called again.
 *   Most likely it's the same pointer you used for \c q2proto_deflate_impl_helper_begin(),
 *   but that's not a requirement.
 * \returns Error code.
 */
Q2PROTO_DEFLATE_IMPL_HELPER_API q2proto_error_t q2proto_deflate_impl_helper_get_data(
    q2proto_deflate_impl_helper_args_t *deflate_args, uint32_t total_size, q2proto_deflate_stream_mode_t stream_mode,
    size_t *in_size, const void **out, size_t *out_size, const void *next_input);

/** @} */

#endif // Q2PROTO_DEFLATE_IMPL_HELPER_H_
