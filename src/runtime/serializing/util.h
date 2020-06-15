/*
 * util.h
 *
 * Distributed under the MIT License (MIT).
 *
 * Copyright (c) 2015 Eric Seckler
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef SRC_RUNTIME_SERIALIZING_UTIL_H_
#define SRC_RUNTIME_SERIALIZING_UTIL_H_

#include <endian.h>
#include <byteswap.h>
#include <stddef.h>
#include <sys/types.h>
#include <cstdint>
#include <cstring>

#include "runtime/serializing/serialize_result.h"

namespace diffingo {
namespace runtime {
namespace serializing {
namespace util {

template <typename T>
inline SerializeResult copyAtomicType(char* serialize_src, char** pos_ptr,
                                      char* out_buf_end) {
  if (out_buf_end - *pos_ptr < sizeof(T)) return SerializeResult::OUT_BUF_FULL;
  *reinterpret_cast<T*>(*pos_ptr) = *reinterpret_cast<T*>(serialize_src);
  *pos_ptr += sizeof(T);
  return SerializeResult::DONE;
}

/*
inline SerializeResult copyBytes(char** pos_ptr, char* in_buf_end, char*
parse_dest,
                             size_t len) {
  size_t free = in_buf_end - *pos_ptr;
  size_t parse_len = min(free, len);
  if (parse_len <= 0) return SerializeResult::OUT_OF_DATA;
  memcpy(parse_dest, *pos_ptr, parse_len);
  pos_ptr += x;
  len -= x;
  if (parse_len < len) return SerializeResult::OUT_OF_DATA;
  return SerializeResult::DONE;
}
*/

inline SerializeResult copyBytes(char* serialize_src, size_t len,
                                 char** pos_ptr, char* out_buf_end) {
  if (out_buf_end - *pos_ptr < static_cast<ssize_t>(len))
    return SerializeResult::OUT_BUF_FULL;
  memcpy(*pos_ptr, serialize_src, len);
  *pos_ptr += len;
  return SerializeResult::DONE;
}

// unsigned integers - big endian
inline SerializeResult serializeInt8_unsigned_big(char* serialize_src,
                                                  char** pos_ptr,
                                                  char* out_buf_end);
inline SerializeResult serializeInt16_unsigned_big(char* serialize_src,
                                                   char** pos_ptr,
                                                   char* out_buf_end);
inline SerializeResult serializeInt32_unsigned_big(char* serialize_src,
                                                   char** pos_ptr,
                                                   char* out_buf_end);
inline SerializeResult serializeInt64_unsigned_big(char* serialize_src,
                                                   char** pos_ptr,
                                                   char* out_buf_end);

// unsigned integers - little endian
inline SerializeResult serializeInt8_unsigned_little(char* serialize_src,
                                                     char** pos_ptr,
                                                     char* out_buf_end);
inline SerializeResult serializeInt16_unsigned_little(char* serialize_src,
                                                      char** pos_ptr,
                                                      char* out_buf_end);
inline SerializeResult serializeInt32_unsigned_little(char* serialize_src,
                                                      char** pos_ptr,
                                                      char* out_buf_end);
inline SerializeResult serializeInt64_unsigned_little(char* serialize_src,
                                                      char** pos_ptr,
                                                      char* out_buf_end);

// signed integers - big endian
inline SerializeResult serializeInt8_signed_big(char* serialize_src,
                                                char** pos_ptr,
                                                char* out_buf_end);
inline SerializeResult serializeInt16_signed_big(char* serialize_src,
                                                 char** pos_ptr,
                                                 char* out_buf_end);
inline SerializeResult serializeInt32_signed_big(char* serialize_src,
                                                 char** pos_ptr,
                                                 char* out_buf_end);
inline SerializeResult serializeInt64_signed_big(char* serialize_src,
                                                 char** pos_ptr,
                                                 char* out_buf_end);

// signed integers - little endian
inline SerializeResult serializeInt8_signed_little(char* serialize_src,
                                                   char** pos_ptr,
                                                   char* out_buf_end);
inline SerializeResult serializeInt16_signed_little(char* serialize_src,
                                                    char** pos_ptr,
                                                    char* out_buf_end);
inline SerializeResult serializeInt32_signed_little(char* serialize_src,
                                                    char** pos_ptr,
                                                    char* out_buf_end);
inline SerializeResult serializeInt64_signed_little(char* serialize_src,
                                                    char** pos_ptr,
                                                    char* out_buf_end);

/* --- IMPLEMENTATION --- */

#pragma GCC diagnostic ignored "-Wsign-compare"

#if __BYTE_ORDER == __LITTLE_ENDIAN

inline SerializeResult serializeInt8_unsigned_big(char* serialize_src,
                                                  char** pos_ptr,
                                                  char* out_buf_end) {
  // only a single byte -> nothing changes
  return copyAtomicType<uint8_t>(serialize_src, pos_ptr, out_buf_end);
}

inline SerializeResult serializeInt16_unsigned_big(char* serialize_src,
                                                   char** pos_ptr,
                                                   char* out_buf_end) {
  if (out_buf_end - *pos_ptr < sizeof(uint16_t))
    return SerializeResult::OUT_BUF_FULL;
  *reinterpret_cast<uint16_t*>(*pos_ptr) =
      bswap_16(*reinterpret_cast<uint16_t*>(serialize_src));
  *pos_ptr += sizeof(uint16_t);
  return SerializeResult::DONE;
}

inline SerializeResult serializeInt32_unsigned_big(char* serialize_src,
                                                   char** pos_ptr,
                                                   char* out_buf_end) {
  if (out_buf_end - *pos_ptr < sizeof(uint32_t))
    return SerializeResult::OUT_BUF_FULL;
  *reinterpret_cast<uint32_t*>(*pos_ptr) =
      bswap_32(*reinterpret_cast<uint32_t*>(serialize_src));
  *pos_ptr += sizeof(uint32_t);
  return SerializeResult::DONE;
}

inline SerializeResult serializeInt64_unsigned_big(char* serialize_src,
                                                   char** pos_ptr,
                                                   char* out_buf_end) {
  if (out_buf_end - *pos_ptr < sizeof(uint64_t))
    return SerializeResult::OUT_BUF_FULL;
  *reinterpret_cast<uint64_t*>(*pos_ptr) =
      bswap_64(*reinterpret_cast<uint64_t*>(serialize_src));
  *pos_ptr += sizeof(uint64_t);
  return SerializeResult::DONE;
}

inline SerializeResult serializeInt8_unsigned_little(char* serialize_src,
                                                     char** pos_ptr,
                                                     char* out_buf_end) {
  return copyAtomicType<uint8_t>(serialize_src, pos_ptr, out_buf_end);
}

inline SerializeResult serializeInt16_unsigned_little(char* serialize_src,
                                                      char** pos_ptr,
                                                      char* out_buf_end) {
  return copyAtomicType<uint16_t>(serialize_src, pos_ptr, out_buf_end);
}

inline SerializeResult serializeInt32_unsigned_little(char* serialize_src,
                                                      char** pos_ptr,
                                                      char* out_buf_end) {
  return copyAtomicType<uint32_t>(serialize_src, pos_ptr, out_buf_end);
}

inline SerializeResult serializeInt64_unsigned_little(char* serialize_src,
                                                      char** pos_ptr,
                                                      char* out_buf_end) {
  return copyAtomicType<uint64_t>(serialize_src, pos_ptr, out_buf_end);
}

inline SerializeResult serializeInt8_signed_big(char* serialize_src,
                                                char** pos_ptr,
                                                char* out_buf_end) {
  // only a single byte -> nothing changes
  return copyAtomicType<int8_t>(serialize_src, pos_ptr, out_buf_end);
}

inline SerializeResult serializeInt16_signed_big(char* serialize_src,
                                                 char** pos_ptr,
                                                 char* out_buf_end) {
  if (out_buf_end - *pos_ptr < sizeof(int16_t))
    return SerializeResult::OUT_BUF_FULL;
  *reinterpret_cast<int16_t*>(*pos_ptr) =
      bswap_16(*reinterpret_cast<int16_t*>(serialize_src));
  *pos_ptr += sizeof(int16_t);
  return SerializeResult::DONE;
}

inline SerializeResult serializeInt32_signed_big(char* serialize_src,
                                                 char** pos_ptr,
                                                 char* out_buf_end) {
  if (out_buf_end - *pos_ptr < sizeof(int32_t))
    return SerializeResult::OUT_BUF_FULL;
  *reinterpret_cast<int32_t*>(*pos_ptr) =
      bswap_32(*reinterpret_cast<int32_t*>(serialize_src));
  *pos_ptr += sizeof(int32_t);
  return SerializeResult::DONE;
}

inline SerializeResult serializeInt64_signed_big(char* serialize_src,
                                                 char** pos_ptr,
                                                 char* out_buf_end) {
  if (out_buf_end - *pos_ptr < sizeof(int64_t))
    return SerializeResult::OUT_BUF_FULL;
  *reinterpret_cast<int64_t*>(*pos_ptr) =
      bswap_64(*reinterpret_cast<int64_t*>(serialize_src));
  *pos_ptr += sizeof(int64_t);
  return SerializeResult::DONE;
}

inline SerializeResult serializeInt8_signed_little(char* serialize_src,
                                                   char** pos_ptr,
                                                   char* out_buf_end) {
  return copyAtomicType<int8_t>(serialize_src, pos_ptr, out_buf_end);
}

inline SerializeResult serializeInt16_signed_little(char* serialize_src,
                                                    char** pos_ptr,
                                                    char* out_buf_end) {
  return copyAtomicType<int16_t>(serialize_src, pos_ptr, out_buf_end);
}

inline SerializeResult serializeInt32_signed_little(char* serialize_src,
                                                    char** pos_ptr,
                                                    char* out_buf_end) {
  return copyAtomicType<int32_t>(serialize_src, pos_ptr, out_buf_end);
}

inline SerializeResult serializeInt64_signed_little(char* serialize_src,
                                                    char** pos_ptr,
                                                    char* out_buf_end) {
  return copyAtomicType<int64_t>(serialize_src, pos_ptr, out_buf_end);
}

#elif __BYTE_ORDER == __BIG_ENDIAN
// TODO(ES): support big endian systems
#error big endian system not supported yet
#endif

}  // namespace util
}  // namespace serializing
}  // namespace runtime
}  // namespace diffingo

#endif  // SRC_RUNTIME_SERIALIZING_UTIL_H_
