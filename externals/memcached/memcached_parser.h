/*
 * memcached_parser.h
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

#ifndef EXTERNALS_MEMCACHED_MEMCACHED_PARSER_H_
#define EXTERNALS_MEMCACHED_MEMCACHED_PARSER_H_

#include <netinet/in.h>
#include <stddef.h>
#include <cstdint>
#include <ctime>

struct addrinfo;
struct memcached_allocator_t;
struct memcached_array_st;
struct memcached_continuum_item_st;
struct memcached_error_t;
struct memcached_sasl_st;
struct memcached_virtual_bucket_t;

#define MEMCACHED_MAX_BUFFER 8196
#define MEMCACHED_MAX_KEY 251 /* We add one to have it null terminated */

#define MEMCACHED_BLOCK_SIZE 1024
#define MEMCACHED_DEFAULT_COMMAND_SIZE 350
#define SMALL_STRING_LEN 1024
#define HUGE_STRING_LEN 8196
// @todo Complete class transformation
struct memcached_instance_st {
  in_port_t port() const { return port_; }

  void port(in_port_t arg) { port_ = arg; }

  void mark_server_as_clean() {
    server_failure_counter = 0;
    server_timeout_counter = 0;
    next_retry = 0;
  }

  void disable() {}

  void enable() {}

  bool valid() const;

  bool is_shutting_down() const;

  void start_close_socket();
  void close_socket();
  void reset_socket();

  uint32_t response_count() const { return cursor_active_; }

  struct {
    bool is_allocated;
    bool is_initialized;
    bool is_shutting_down;
    bool is_dead;
    bool ready;
  } options;

  int16_t _events;
  int16_t _revents;

  int16_t events(void) { return _events; }

  int16_t revents(void) { return _revents; }

  void events(int16_t);
  void revents(int16_t);

  uint32_t cursor_active_;
  in_port_t port_;
  uint32_t io_bytes_sent; /* # bytes sent since last read */
  uint32_t request_id;
  uint32_t server_failure_counter;
  uint64_t server_failure_counter_query_id;
  uint32_t server_timeout_counter;
  uint32_t server_timeout_counter_query_id;
  uint32_t weight;
  uint32_t version;
  struct {
    uint32_t read;
    uint32_t write;
    uint32_t timeouts;
    size_t _bytes_read;
  } io_wait_count;
  uint8_t major_version;  // Default definition of UINT8_MAX means that it has
                          // not been set.
  uint8_t micro_version;  // ditto, and note that this is the third, not second
                          // version bit
  uint8_t minor_version;  // ditto
  char *read_ptr;
  size_t read_buffer_length;
  size_t read_data_length;
  size_t write_buffer_offset;
  struct addrinfo *address_info;
  struct addrinfo *address_info_next;
  time_t next_retry;
  uint64_t limit_maxbytes;
  struct memcached_error_t *error_messages;
  char read_buffer[MEMCACHED_MAX_BUFFER];
  char write_buffer[MEMCACHED_MAX_BUFFER];
};

struct memcached_string_st {
  char *end;
  char *string;  // NOLINT
  size_t current_size;
  struct {
    bool is_allocated : 1;
    bool is_initialized : 1;
  } options;
};

struct memcached_result_st {
  uint32_t item_flags;
  time_t item_expiration;
  size_t key_length;
  size_t extras_length;
  uint64_t item_cas;
  memcached_string_st value;
  uint64_t numeric_value;
  uint64_t count;
  char item_key[MEMCACHED_MAX_KEY];
  char item_extras[MEMCACHED_MAX_KEY];
  struct {
    bool is_allocated : 1;
    bool is_initialized : 1;
  } options;
  /* Add result callback function */
};

#define memcached_is_initialized(__object) ((__object)->options.is_initialized)
#define memcached_is_allocated(__object) ((__object)->options.is_allocated)
#define memcached_set_initialized(__object, __value) \
  ((__object)->options.is_initialized = (__value))
#define memcached_set_allocated(__object, __value) \
  ((__object)->options.is_allocated = (__value))

memcached_string_st *memcached_string_create(memcached_string_st *string,
                                             size_t initial_size);

bool memcached_string_check(memcached_string_st *string, size_t need);

char *memcached_string_c_copy(memcached_string_st *string);

bool memcached_string_append_character(memcached_string_st *string,
                                       char character);

bool memcached_string_append(memcached_string_st *string, const char *value,
                             size_t length);

void memcached_string_reset(memcached_string_st *string);

void memcached_string_free(memcached_string_st *string);  // NOLINT
void memcached_string_free(memcached_string_st &);

size_t memcached_string_length(const memcached_string_st *self);
size_t memcached_string_length(const memcached_string_st &);

size_t memcached_string_size(const memcached_string_st *self);

const char *memcached_string_value(const memcached_string_st *self);
const char *memcached_string_value(const memcached_string_st &);

char *memcached_string_take_value(memcached_string_st *self);

char *memcached_string_value_mutable(const memcached_string_st *self);

bool memcached_string_set(memcached_string_st &, const char *, size_t);

void memcached_string_set_length(memcached_string_st *self, size_t length);
void memcached_string_set_length(memcached_string_st &, const size_t length);

bool memcached_string_resize(memcached_string_st &, const size_t);
char *memcached_string_c_str(memcached_string_st &);

memcached_result_st *memcached_result_create(memcached_result_st *ptr);
void memcached_result_free(memcached_result_st *ptr);

bool memcached_read_one_response(memcached_instance_st *instance,
                                 memcached_result_st *result);

bool memcached_serialize_binary(int8_t opcode, const char *key,
                                const size_t key_length, const char *extras,
                                const size_t extra_length, const char *value,
                                const size_t value_length, const uint64_t cas,
                                uint16_t opaque, char *out, size_t out_size,
                                size_t *bytes_written);

#endif  // EXTERNALS_MEMCACHED_MEMCACHED_PARSER_H_
