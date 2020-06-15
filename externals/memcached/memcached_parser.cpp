/*
 * memcached_parser.cpp
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

#include "memcached_parser.h"  // NOLINT

#include <netinet/in.h>
#include <stddef.h>
#include <byteswap.h>
#include <assert.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "protocol_binary.h"  // NOLINT

bool memcached_safe_read(memcached_instance_st *instance, char *buffer,
                         const size_t size) {
  if (instance->read_data_length < size) return false;
  memcpy(buffer, instance->read_ptr, size);
  instance->read_ptr += size;
  instance->read_data_length -= size;
  return true;
}

static inline void libmemcached_free(void *mem) { std::free(mem); }

static inline void *libmemcached_malloc(const size_t size) {
  return std::malloc(size);
}
#define libmemcached_xmalloc(__type) \
  ((__type *)libmemcached_malloc(sizeof(__type)))  // NOLINT

static inline void *libmemcached_realloc(void *mem, size_t nmemb,
                                         const size_t size) {
  return std::realloc(mem, size*nmemb);
}
#define libmemcached_xrealloc(__mem, __nelem, __type)   \
  ((__type *)libmemcached_realloc((__mem), /* NOLINT */ \
                                  (__nelem), sizeof(__type)))
#define libmemcached_xvalloc(__nelem, __type)        \
  ((__type *)libmemcached_realloc(NULL, /* NOLINT */ \
                                  (__nelem), sizeof(__type)))

static inline void *libmemcached_calloc(size_t nelem, size_t size) {
  return std::calloc(nelem, size);
}
#define libmemcached_xcalloc(__nelem, __type)            \
  ((__type *)libmemcached_calloc((__nelem), /* NOLINT */ \
                                 sizeof(__type)))

inline static bool _string_check(memcached_string_st *string, size_t need) {
  if (need &&
      need > (size_t)(string->current_size -
                      (size_t)(string->end - string->string))) {
    size_t current_offset = (size_t)(string->end - string->string);

    /* This is the block multiplier. To keep it larger and surive division
     * errors we must round it up */
    size_t adjust = (need - (size_t)(string->current_size -
                                     (size_t)(string->end - string->string))) /
                    MEMCACHED_BLOCK_SIZE;
    adjust++;

    size_t new_size = sizeof(char) * (size_t)((adjust * MEMCACHED_BLOCK_SIZE) +
                                              string->current_size);
    /* Test for overflow */
    if (new_size < need) {
      char error_message[1024];
      int error_message_length =
          snprintf(error_message, sizeof(error_message), "Needed %ld, got %ld",
                   (long)need, (long)new_size);  // NOLINT
      std::cerr << error_message << std::endl;
      return false;
    }

    char *new_value = libmemcached_xrealloc(string->string, new_size, char);

    if (new_value == NULL) {
      std::cerr << "mem alloc error" << std::endl;
      return false;
    }

    string->string = new_value;
    string->end = string->string + current_offset;

    string->current_size += (MEMCACHED_BLOCK_SIZE * adjust);
  }

  return true;
}

static inline void _init_string(memcached_string_st *self) {
  self->current_size = 0;
  self->end = self->string = NULL;
}

memcached_string_st *memcached_string_create(memcached_string_st *self,
                                             size_t initial_size) {
  /* Saving malloc calls :) */
  if (self) {
    memcached_set_allocated(self, false);
  } else {
    self = libmemcached_xmalloc(memcached_string_st);

    if (self == NULL) {
      return NULL;
    }

    memcached_set_allocated(self, true);
  }

  _init_string(self);

  if (!(_string_check(self, initial_size))) {
    if (memcached_is_allocated(self)) {
      libmemcached_free(self);
    }

    return NULL;
  }

  memcached_set_initialized(self, true);
  return self;
}

static bool memcached_string_append_null(
    memcached_string_st &string) {  // NOLINT
  if (!(_string_check(&string, 1))) {
    return false;
  }

  *string.end = 0;

  return true;
}

static bool memcached_string_append_null(memcached_string_st *string) {
  if (!(_string_check(string, 1))) {
    return false;
  }

  *string->end = 0;

  return true;
}

bool memcached_string_append_character(memcached_string_st *string,
                                       char character) {
  if (!(_string_check(string, 1))) {
    return false;
  }

  *string->end = character;
  string->end++;

  return true;
}

bool memcached_string_append(memcached_string_st *string, const char *value,
                             size_t length) {
  if (!(_string_check(string, length))) {
    return false;
  }

  memcpy(string->end, value, length);
  string->end += length;

  return true;
}

char *memcached_string_c_copy(memcached_string_st *string) {
  if (memcached_string_length(string) == 0) {
    return NULL;
  }

  char *c_ptr = static_cast<char *>(libmemcached_malloc(
      (memcached_string_length(string) + 1) * sizeof(char)));

  if (c_ptr == NULL) {
    return NULL;
  }

  memcpy(c_ptr, memcached_string_value(string),
         memcached_string_length(string));
  c_ptr[memcached_string_length(string)] = 0;

  return c_ptr;
}

bool memcached_string_set(memcached_string_st &string,  // NOLINT
                          const char *value, size_t length) {
  memcached_string_reset(&string);
  if (memcached_string_append(&string, value, length)) {
    memcached_string_append_null(string);
    return true;
  }

  return false;
}

void memcached_string_reset(memcached_string_st *string) {
  string->end = string->string;
}

void memcached_string_free(memcached_string_st &ptr) {  // NOLINT
  memcached_string_free(&ptr);
}

void memcached_string_free(memcached_string_st *ptr) {
  if (ptr == NULL) {
    return;
  }

  if (ptr->string) {
    libmemcached_free(ptr->string);
  }

  if (memcached_is_allocated(ptr)) {
    libmemcached_free(ptr);
  } else {
    ptr->options.is_initialized = false;
  }
}

bool memcached_string_check(memcached_string_st *string, size_t need) {
  return _string_check(string, need);
}

bool memcached_string_resize(memcached_string_st &string,  // NOLINT
                             const size_t need) {
  return _string_check(&string, need);
}

size_t memcached_string_length(const memcached_string_st *self) {
  return size_t(self->end - self->string);
}

size_t memcached_string_length(const memcached_string_st &self) {
  return size_t(self.end - self.string);
}

size_t memcached_string_size(const memcached_string_st *self) {
  return self->current_size;
}

const char *memcached_string_value(const memcached_string_st *self) {
  return self->string;
}

const char *memcached_string_value(const memcached_string_st &self) {
  return self.string;
}

char *memcached_string_take_value(memcached_string_st *self) {
  char *value = NULL;

  if (self) {
    if (memcached_string_length(self)) {
      // If we fail at adding the null, we copy and move on
      if (!(memcached_string_append_null(self))) {
        return NULL;
      }

      value = self->string;
      _init_string(self);
    }
  }

  return value;
}

char *memcached_string_value_mutable(const memcached_string_st *self) {
  return self->string;
}

char *memcached_string_c_str(memcached_string_st &self) {  // NOLINT
  return self.string;
}

void memcached_string_set_length(memcached_string_st *self, size_t length) {
  self->end = self->string + length;
}

void memcached_string_set_length(memcached_string_st &self,  // NOLINT
                                 const size_t length) {
  assert(self.current_size >= length);
  size_t set_length = length;
  if (self.current_size > length) {
    if (!(_string_check(&self, length))) {
      set_length = self.current_size;
    }
  }
  self.end = self.string + set_length;  // NOLINT
}

static inline void _result_init(memcached_result_st *self) {
  self->item_flags = 0;
  self->item_expiration = 0;
  self->key_length = 0;
  self->item_cas = 0;
  self->numeric_value = UINT64_MAX;
  self->count = 0;
  self->item_key[0] = 0;
}

memcached_result_st *memcached_result_create(memcached_result_st *ptr) {
  /* Saving malloc calls :) */
  if (ptr) {
    ptr->options.is_allocated = false;
  } else {
    ptr = libmemcached_xmalloc(memcached_result_st);

    if (!ptr) {
      return NULL;
    }

    ptr->options.is_allocated = true;
  }

  ptr->options.is_initialized = true;

  _result_init(ptr);  // NOLINT

  memcached_string_create(&ptr->value, 0);  // NOLINT

  return ptr;
}

void memcached_result_reset(memcached_result_st *ptr) {
  ptr->key_length = 0;
  memcached_string_reset(&ptr->value);
  ptr->item_flags = 0;
  ptr->item_cas = 0;
  ptr->item_expiration = 0;
  ptr->numeric_value = UINT64_MAX;
}

void memcached_result_free(memcached_result_st *ptr) {
  if (ptr == NULL) {
    return;
  }

  memcached_string_free(&ptr->value);
  ptr->numeric_value = UINT64_MAX;

  if (memcached_is_allocated(ptr)) {
    libmemcached_free(ptr);
  } else {
    ptr->count = 0;
    ptr->options.is_initialized = false;
  }
}

const char *memcached_result_key_value(const memcached_result_st *self) {
  return self->key_length ? self->item_key : NULL;
}

size_t memcached_result_key_length(const memcached_result_st *self) {
  return self->key_length;
}

uint32_t memcached_result_flags(const memcached_result_st *self) {
  return self->item_flags;
}

uint64_t memcached_result_cas(const memcached_result_st *self) {
  return self->item_cas;
}

void memcached_result_set_flags(memcached_result_st *self, uint32_t flags) {
  self->item_flags = flags;
}

void memcached_result_set_expiration(memcached_result_st *self,
                                     time_t expiration) {
  self->item_expiration = expiration;
}

static bool binary_read_one_response(memcached_instance_st *instance,
                                     char *buffer, const size_t buffer_length,
                                     memcached_result_st *result) {
  protocol_binary_response_header header;

  if (!memcached_safe_read(instance, reinterpret_cast<char *>(&header.bytes),
                           sizeof(header.bytes))) {
    return false;
  }

  if (header.response.magic != PROTOCOL_BINARY_RES) {
    std::cerr << "expected binary response magic code" << std::endl;
  }

  /*
   * Convert the header to host local endian!
   */
  header.response.keylen = ntohs(header.response.keylen);
  header.response.status = ntohs(header.response.status);
  header.response.bodylen = ntohl(header.response.bodylen);
  header.response.cas = bswap_64(header.response.cas);
  uint32_t bodylen = header.response.bodylen;

  uint16_t keylen = header.response.keylen;
  uint8_t extlen = header.response.extlen;
  memcached_result_reset(result);
  result->item_cas = header.response.cas;

  result->extras_length = extlen;
  if (!memcached_safe_read(instance, result->item_extras, extlen)) {
    return false;
  }

  bodylen -= extlen;

  result->key_length = keylen;
  if (!memcached_safe_read(instance, result->item_key, keylen)) {
    return false;
  }

  bodylen -= keylen;
  if (!memcached_string_check(&result->value, bodylen)) {
    return false;
  }

  char *vptr = memcached_string_value_mutable(&result->value);
  if (!memcached_safe_read(instance, vptr, bodylen)) {
    return false;
  }

  memcached_string_set_length(&result->value, bodylen);

  return true;
}

bool memcached_read_one_response(memcached_instance_st *instance,
                                 memcached_result_st *result) {
  char buffer[SMALL_STRING_LEN];
  return binary_read_one_response(instance, buffer, sizeof(buffer), result);
}

bool memcached_serialize_binary(int8_t opcode, const char *key,
                                const size_t key_length, const char *extras,
                                const size_t extra_length, const char *value,
                                const size_t value_length, const uint64_t cas,
                                uint16_t opaque, char *out, size_t out_size,
                                size_t *bytes_written) {
  protocol_binary_request_header request = {};
  size_t send_length = sizeof(request.bytes);

  request.request.magic = PROTOCOL_BINARY_REQ;
  request.request.opaque = htons(opaque);

  request.request.opcode = opcode;
  request.request.keylen = htons((uint16_t)(key_length));
  request.request.datatype = PROTOCOL_BINARY_RAW_BYTES;
  request.request.extlen = extra_length;

  request.request.bodylen =
      htonl((uint32_t)(key_length + value_length + request.request.extlen));

  request.request.cas = bswap_64(cas);

  if (out_size < send_length + key_length + value_length + extra_length)
    return false;

  memcpy(out, request.bytes, send_length);
  out += send_length;
  memcpy(out, extras, extra_length);
  out += extra_length;
  memcpy(out, key, key_length);
  out += key_length;
  memcpy(out, value, value_length);

  *bytes_written = send_length + key_length + value_length + extra_length;

  return true;
}
