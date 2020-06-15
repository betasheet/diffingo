/*
 * test_memcached.cpp
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

#include <gtest/gtest.h>
#include <stddef.h>
#include <stdlib.h>

#include "examples/out_test/memcached_inst.h"
#include "runtime/parsing/parse_result.h"
#include "runtime/parsing/parser_state.h"
#include "runtime/unit/unit_area.h"

namespace dr = diffingo::runtime;

TEST(CompactMemcachedTest, ParseRequest) {
  size_t stack_buf_size = 2 * 1024 * 1024;
  size_t out_buf_size = 2 * 1024 * 1024;

  char* stack_buf = reinterpret_cast<char*>(malloc(stack_buf_size));
  char* out_buf = reinterpret_cast<char*>(malloc(out_buf_size));

  // from https://code.google.com/p/memcached/wiki/MemcacheBinaryProtocol
  char in_buf[] = {0x80, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                   0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                   0x00, 0x00, 0x00, 0x00, 0x48, 0x65, 0x6c, 0x6c, 0x6f};
  size_t in_buf_len = 29;

  memcached_compact::CompactMemcachedCommandParser parser;

  auto in_buf_end = in_buf + in_buf_len;
  auto area = new (out_buf) dr::unit::UnitArea(out_buf_size);
  dr::parsing::ParserState state(stack_buf, stack_buf_size);
  size_t bytes_read;

  auto res = parser.parse(in_buf, in_buf_end, area, &state, &bytes_read);

  ASSERT_EQ(dr::parsing::ParseResult::DONE, res);
  ASSERT_EQ(in_buf_len, bytes_read);

  auto command = reinterpret_cast<memcached_compact::CompactMemcachedCommand*>(
      area->contents());
  // ASSERT_EQ(memcached_compact::MemcachedMagicCode::REQUEST,
  //          command->magic_code);
  ASSERT_EQ(memcached_compact::MemcachedOpCode::GET, command->opcode);
  ASSERT_EQ(5, command->key.len_);
  ASSERT_EQ('H', command->key.data_[0]);
  ASSERT_EQ('e', command->key.data_[1]);
  ASSERT_EQ('l', command->key.data_[2]);
  ASSERT_EQ('l', command->key.data_[3]);
  ASSERT_EQ('o', command->key.data_[4]);
}

TEST(CompactMemcachedTest, SerializeRequest) {
  size_t stack_buf_size = 2 * 1024 * 1024;
  size_t out_buf_size = 2 * 1024 * 1024;
  size_t ser_buf_size = 2 * 1024 * 1024;

  char* stack_buf = reinterpret_cast<char*>(malloc(stack_buf_size));
  char* out_buf = reinterpret_cast<char*>(malloc(out_buf_size));
  char* ser_buf = reinterpret_cast<char*>(malloc(ser_buf_size));

  // from https://code.google.com/p/memcached/wiki/MemcacheBinaryProtocol
  char in_buf[] = {0x80, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                   0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                   0x00, 0x00, 0x00, 0x00, 0x48, 0x65, 0x6c, 0x6c, 0x6f};
  size_t in_buf_len = 29;

  memcached_compact::CompactMemcachedCommandParser parser;
  memcached_compact::CompactMemcachedCommandSerializer serializer;

  auto in_buf_end = in_buf + in_buf_len;
  auto area = new (out_buf) dr::unit::UnitArea(out_buf_size);
  dr::parsing::ParserState state(stack_buf, stack_buf_size);
  size_t bytes_read;

  auto res = parser.parse(in_buf, in_buf_end, area, &state, &bytes_read);
  ASSERT_EQ(dr::parsing::ParseResult::DONE, res);

  state.reset();
  size_t bytes_written;
  auto ser_buf_end = ser_buf + ser_buf_size;

  auto res2 = serializer.serialize(area->contents(), ser_buf, ser_buf_end,
                                   &state, &bytes_written);
  ASSERT_EQ(dr::serializing::SerializeResult::DONE, res2);
  ASSERT_EQ(in_buf_len, bytes_written);

  for (size_t i = 0; i < in_buf_len; i++) {
    ASSERT_EQ(in_buf[i], ser_buf[i]);
  }
}
