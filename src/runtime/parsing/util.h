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

#ifndef SRC_RUNTIME_PARSING_UTIL_H_
#define SRC_RUNTIME_PARSING_UTIL_H_

#include <endian.h>
#include <byteswap.h>
#include <stddef.h>
#include <sys/types.h>
#include <cstdint>
#include <cstring>

#include "runtime/parsing/parse_result.h"
#include "runtime/unit/unit_area.h"

namespace diffingo {
namespace runtime {
namespace parsing {
namespace util {

template <typename T>
inline ParseResult copyAtomicType(char** pos_ptr, char* in_buf_end,
                                  char* parse_dest) {
  if (in_buf_end - *pos_ptr < sizeof(T)) return ParseResult::OUT_OF_DATA;
  *reinterpret_cast<T*>(parse_dest) = *reinterpret_cast<T*>(*pos_ptr);
  *pos_ptr += sizeof(T);
  return ParseResult::DONE;
}

/*
inline ParseResult copyBytes(char** pos_ptr, char* in_buf_end, char* parse_dest,
                             size_t len) {
  size_t free = in_buf_end - *pos_ptr;
  size_t parse_len = min(free, len);
  if (parse_len <= 0) return ParseResult::OUT_OF_DATA;
  memcpy(parse_dest, *pos_ptr, parse_len);
  pos_ptr += x;
  len -= x;
  if (parse_len < len) return ParseResult::OUT_OF_DATA;
  return ParseResult::DONE;
}
*/

inline ParseResult copyBytes(char** pos_ptr, char* in_buf_end, char* parse_dest,
                             size_t len) {
  if (in_buf_end - *pos_ptr < static_cast<ssize_t>(len))
    return ParseResult::OUT_OF_DATA;
  memcpy(parse_dest, *pos_ptr, len);
  *pos_ptr += len;
  return ParseResult::DONE;
}

inline ParseResult advance(char** pos_ptr, char* in_buf_end, size_t len) {
  if (in_buf_end - *pos_ptr < static_cast<ssize_t>(len))
    return ParseResult::OUT_OF_DATA;
  *pos_ptr += len;
  return ParseResult::DONE;
}

inline ParseResult allocateCopyBytes(char** pos_ptr, char* in_buf_end,
                                     char** parse_dest, size_t len,
                                     unit::UnitArea* area) {
  if (in_buf_end - *pos_ptr < static_cast<ssize_t>(len))
    return ParseResult::OUT_OF_DATA;
  if (!area->allocate(len, parse_dest)) return ParseResult::AREA_FULL;
  memcpy(*parse_dest, *pos_ptr, len);
  *pos_ptr += len;
  return ParseResult::DONE;
}

// unsigned integers - big endian
inline ParseResult parseInt8_unsigned_big(char** pos_ptr, char* in_buf_end,
                                          char* parse_dest);
inline ParseResult parseInt16_unsigned_big(char** pos_ptr, char* in_buf_end,
                                           char* parse_dest);
inline ParseResult parseInt32_unsigned_big(char** pos_ptr, char* in_buf_end,
                                           char* parse_dest);
inline ParseResult parseInt64_unsigned_big(char** pos_ptr, char* in_buf_end,
                                           char* parse_dest);

// unsigned integers - little endian
inline ParseResult parseInt8_unsigned_little(char** pos_ptr, char* in_buf_end,
                                             char* parse_dest);
inline ParseResult parseInt16_unsigned_little(char** pos_ptr, char* in_buf_end,
                                              char* parse_dest);
inline ParseResult parseInt32_unsigned_little(char** pos_ptr, char* in_buf_end,
                                              char* parse_dest);
inline ParseResult parseInt64_unsigned_little(char** pos_ptr, char* in_buf_end,
                                              char* parse_dest);

// signed integers - big endian
inline ParseResult parseInt8_signed_big(char** pos_ptr, char* in_buf_end,
                                        char* parse_dest);
inline ParseResult parseInt16_signed_big(char** pos_ptr, char* in_buf_end,
                                         char* parse_dest);
inline ParseResult parseInt32_signed_big(char** pos_ptr, char* in_buf_end,
                                         char* parse_dest);
inline ParseResult parseInt64_signed_big(char** pos_ptr, char* in_buf_end,
                                         char* parse_dest);

// signed integers - little endian
inline ParseResult parseInt8_signed_little(char** pos_ptr, char* in_buf_end,
                                           char* parse_dest);
inline ParseResult parseInt16_signed_little(char** pos_ptr, char* in_buf_end,
                                            char* parse_dest);
inline ParseResult parseInt32_signed_little(char** pos_ptr, char* in_buf_end,
                                            char* parse_dest);
inline ParseResult parseInt64_signed_little(char** pos_ptr, char* in_buf_end,
                                            char* parse_dest);

/* --- IMPLEMENTATION --- */

#pragma GCC diagnostic ignored "-Wsign-compare"

#if __BYTE_ORDER == __LITTLE_ENDIAN

inline ParseResult parseInt8_unsigned_big(char** pos_ptr, char* in_buf_end,
                                          char* parse_dest) {
  // only a single byte -> nothing changes
  return copyAtomicType<uint8_t>(pos_ptr, in_buf_end, parse_dest);
}

inline ParseResult parseInt16_unsigned_big(char** pos_ptr, char* in_buf_end,
                                           char* parse_dest) {
  if (in_buf_end - *pos_ptr < sizeof(uint16_t)) return ParseResult::OUT_OF_DATA;
  *reinterpret_cast<uint16_t*>(parse_dest) =
      bswap_16(*reinterpret_cast<uint16_t*>(*pos_ptr));
  *pos_ptr += sizeof(uint16_t);
  return ParseResult::DONE;
}

inline ParseResult parseInt32_unsigned_big(char** pos_ptr, char* in_buf_end,
                                           char* parse_dest) {
  if (in_buf_end - *pos_ptr < sizeof(uint32_t)) return ParseResult::OUT_OF_DATA;
  *reinterpret_cast<uint32_t*>(parse_dest) =
      bswap_32(*reinterpret_cast<uint32_t*>(*pos_ptr));
  *pos_ptr += sizeof(uint32_t);
  return ParseResult::DONE;
}

inline ParseResult parseInt64_unsigned_big(char** pos_ptr, char* in_buf_end,
                                           char* parse_dest) {
  if (in_buf_end - *pos_ptr < sizeof(uint64_t)) return ParseResult::OUT_OF_DATA;
  *reinterpret_cast<uint64_t*>(parse_dest) =
      bswap_64(*reinterpret_cast<uint64_t*>(*pos_ptr));
  *pos_ptr += sizeof(uint64_t);
  return ParseResult::DONE;
}

inline ParseResult parseInt8_unsigned_little(char** pos_ptr, char* in_buf_end,
                                             char* parse_dest) {
  return copyAtomicType<uint8_t>(pos_ptr, in_buf_end, parse_dest);
}

inline ParseResult parseInt16_unsigned_little(char** pos_ptr, char* in_buf_end,
                                              char* parse_dest) {
  return copyAtomicType<uint16_t>(pos_ptr, in_buf_end, parse_dest);
}

inline ParseResult parseInt32_unsigned_little(char** pos_ptr, char* in_buf_end,
                                              char* parse_dest) {
  return copyAtomicType<uint32_t>(pos_ptr, in_buf_end, parse_dest);
}

inline ParseResult parseInt64_unsigned_little(char** pos_ptr, char* in_buf_end,
                                              char* parse_dest) {
  return copyAtomicType<uint64_t>(pos_ptr, in_buf_end, parse_dest);
}

inline ParseResult parseInt8_signed_big(char** pos_ptr, char* in_buf_end,
                                        char* parse_dest) {
  // only a single byte -> nothing changes
  return copyAtomicType<int8_t>(pos_ptr, in_buf_end, parse_dest);
}

inline ParseResult parseInt16_signed_big(char** pos_ptr, char* in_buf_end,
                                         char* parse_dest) {
  if (in_buf_end - *pos_ptr < sizeof(int16_t)) return ParseResult::OUT_OF_DATA;
  *reinterpret_cast<int16_t*>(parse_dest) =
      bswap_16(*reinterpret_cast<int16_t*>(*pos_ptr));
  *pos_ptr += sizeof(int16_t);
  return ParseResult::DONE;
}

inline ParseResult parseInt32_signed_big(char** pos_ptr, char* in_buf_end,
                                         char* parse_dest) {
  if (in_buf_end - *pos_ptr < sizeof(int32_t)) return ParseResult::OUT_OF_DATA;
  *reinterpret_cast<int32_t*>(parse_dest) =
      bswap_32(*reinterpret_cast<int32_t*>(*pos_ptr));
  *pos_ptr += sizeof(int32_t);
  return ParseResult::DONE;
}

inline ParseResult parseInt64_signed_big(char** pos_ptr, char* in_buf_end,
                                         char* parse_dest) {
  if (in_buf_end - *pos_ptr < sizeof(int64_t)) return ParseResult::OUT_OF_DATA;
  *reinterpret_cast<int64_t*>(parse_dest) =
      bswap_64(*reinterpret_cast<int64_t*>(*pos_ptr));
  *pos_ptr += sizeof(int64_t);
  return ParseResult::DONE;
}

inline ParseResult parseInt8_signed_little(char** pos_ptr, char* in_buf_end,
                                           char* parse_dest) {
  return copyAtomicType<int8_t>(pos_ptr, in_buf_end, parse_dest);
}

inline ParseResult parseInt16_signed_little(char** pos_ptr, char* in_buf_end,
                                            char* parse_dest) {
  return copyAtomicType<int16_t>(pos_ptr, in_buf_end, parse_dest);
}

inline ParseResult parseInt32_signed_little(char** pos_ptr, char* in_buf_end,
                                            char* parse_dest) {
  return copyAtomicType<int32_t>(pos_ptr, in_buf_end, parse_dest);
}

inline ParseResult parseInt64_signed_little(char** pos_ptr, char* in_buf_end,
                                            char* parse_dest) {
  return copyAtomicType<int64_t>(pos_ptr, in_buf_end, parse_dest);
}

#elif __BYTE_ORDER == __BIG_ENDIAN
// TODO(ES): support big endian systems
#error big endian system not supported yet
#endif

}  // namespace util
}  // namespace parsing
}  // namespace runtime
}  // namespace diffingo

#endif  // SRC_RUNTIME_PARSING_UTIL_H_
