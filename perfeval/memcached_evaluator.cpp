/*
 * memcached_evaluator.cpp
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

#include "memcached_evaluator.h"  // NOLINT

#include <memcached_parser.h>
#include <gtest/gtest.h>
#include <pantheios/frontends/stock.h>
#include <pantheios/pantheios.hpp>
#include <stddef.h>
#include <stdlib.h>
#include <chrono>  // NOLINT
#include <iostream>
#include <list>
#include <string>

#include "examples/out_test/memcached.h"
#include "examples/out_test/memcached_inst.h"
#include "runtime/parsing/parse_result.h"
#include "runtime/parsing/parser_state.h"
#include "runtime/runtime.h"
#include "runtime/serializing/serialize_result.h"
#include "runtime/unit/data_type.h"
#include "runtime/unit/unit_area.h"
#include "util/util.h"

const PAN_CHAR_T PANTHEIOS_FE_PROCESS_IDENTITY[] = "memcached_eval";

int main(int argc, char** argv) {
  MemcachedEvaluator eval;

  if (argc > 1) {
    eval.set_num_repeats(atoi(argv[1]));
  }
  if (argc > 2) {
    eval.set_num_experiments(atoi(argv[2]));
  }

  eval.run();
}

MemcachedEvaluator::MemcachedEvaluator() {}

MemcachedEvaluator::~MemcachedEvaluator() {}

template <typename Parser, typename Serializer>
inline void MemcachedEvaluator::runExperiments() {
  Parser parser;
  Serializer serializer;
  dr::parsing::ParseResult pres;
  dr::serializing::SerializeResult sres;
  size_t bytes_read;
  size_t bytes_written;
  auto ser_buf_end = ser_buf_ + kSerBufSize;

  for (size_t n = 0; n < num_experiments_; n++) {
    /* ---- RUN PARSING ---- */
    auto begin = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < num_repeats_; i++) {
      state_->reset();
      area_->reset();
      pres = parser.parse(in_buf_, in_buf_end_, area_, state_, &bytes_read);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto parsing_duration_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin)
            .count();

    pantheios::log(pantheios::informational,
                   util::fmt("Parsing done in %d ns.", parsing_duration_ns));

    ASSERT_EQ(dr::parsing::ParseResult::DONE, pres);
    ASSERT_EQ(in_buf_end_ - in_buf_, bytes_read);

    /* ---- RUN PARSING INCREMENTALLY ---- */
    begin = std::chrono::high_resolution_clock::now();
    size_t length;
    size_t in_buf_len = in_buf_end_ - in_buf_;
    for (size_t i = 0; i < num_repeats_; i++) {
      state_->reset();
      area_->reset();
      length = 0;
      pres = dr::parsing::ParseResult::OUT_OF_DATA;
      while (pres == dr::parsing::ParseResult::OUT_OF_DATA) {
        length += 64;
        pres =
            parser.parse(in_buf_, in_buf_ + length, area_, state_, &bytes_read);
      }
    }
    end = std::chrono::high_resolution_clock::now();
    auto parsing_incr_duration_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin)
            .count();

    pantheios::log(pantheios::informational,
                   util::fmt("Parsing (incremental) done in %d ns.",
                             parsing_incr_duration_ns));

    ASSERT_EQ(dr::parsing::ParseResult::DONE, pres);
    ASSERT_EQ(in_buf_end_ - in_buf_, bytes_read);

    size_t msg_size_parsed = area_->allocated();

    /* ---- RUN SERIALIZING ---- */
    begin = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < num_repeats_; i++) {
      state_->reset();
      sres = serializer.serialize(area_->contents(), ser_buf_, ser_buf_end,
                                  state_, &bytes_written);
    }
    end = std::chrono::high_resolution_clock::now();
    auto serializing_duration_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin)
            .count();

    pantheios::log(
        pantheios::informational,
        util::fmt("Serializing done in %d ns.", serializing_duration_ns));

    ASSERT_EQ(dr::serializing::SerializeResult::DONE, sres);
    ASSERT_EQ(in_buf_end_ - in_buf_, bytes_written);

    Result result(util::type_name<Parser>(), key_len_, extras_len_, value_len_,
                  num_repeats_, bytes_read, msg_size_parsed,
                  parsing_duration_ns, parsing_incr_duration_ns,
                  serializing_duration_ns);
    result_records_.push_back(result);
  }
}

void MemcachedEvaluator::runLibmemcachedExperiments() {
  memcached_instance_st memc;
  memcached_result_st* pres = memcached_result_create(0);

  size_t bytes_read;
  size_t bytes_written;
  auto ser_buf_end = ser_buf_ + kSerBufSize;

  for (size_t n = 0; n < num_experiments_; n++) {
    /* ---- RUN PARSING ---- */
    auto begin = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < num_repeats_; i++) {
      memc.read_ptr = in_buf_;
      memc.read_data_length = in_buf_end_ - in_buf_;
      ASSERT_TRUE(memcached_read_one_response(&memc, pres));
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto parsing_duration_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin)
            .count();

    pantheios::log(pantheios::informational,
                   util::fmt("Parsing done in %d ns.", parsing_duration_ns));

    bytes_read = memc.read_ptr - in_buf_;
    ASSERT_EQ(in_buf_end_ - in_buf_, bytes_read);

    /* ---- RUN PARSING INCREMENTALLY ---- */
    begin = std::chrono::high_resolution_clock::now();
    size_t length;
    size_t in_buf_len = in_buf_end_ - in_buf_;
    for (size_t i = 0; i < num_repeats_; i++) {
      state_->reset();
      area_->reset();
      length = 0;
      bool res = false;
      while (!res) {
        length += 64;
        memc.read_ptr = in_buf_;
        memc.read_data_length = length;
        res = memcached_read_one_response(&memc, pres);
      }
    }
    end = std::chrono::high_resolution_clock::now();
    auto parsing_incr_duration_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin)
            .count();

    pantheios::log(pantheios::informational,
                   util::fmt("Parsing (incremental) done in %d ns.",
                             parsing_incr_duration_ns));

    /* ---- RUN SERIALIZING ---- */
    auto command =
        reinterpret_cast<memcached::MemcachedCommand*>(area_->contents());
    *reinterpret_cast<uint64_t*>(command->cas.data_) =
        bswap_64(*reinterpret_cast<uint64_t*>(command->cas.data_));
    *reinterpret_cast<uint16_t*>(command->opaque.data_) =
        bswap_16(*reinterpret_cast<uint16_t*>(command->opaque.data_));

    begin = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < num_repeats_; i++) {
      auto opcode = static_cast<int8_t>(command->opcode);
      auto key = command->key.data_;
      auto key_len = command->key.len_;
      auto extras = command->extras.data_;
      auto extra_len = command->extras.len_;
      auto value = command->value.data_;
      auto value_len = command->value.len_;
      auto cas = *reinterpret_cast<uint64_t*>(command->cas.data_);
      auto opaque = *reinterpret_cast<uint16_t*>(command->cas.data_);
      ASSERT_TRUE(memcached_serialize_binary(
          opcode, key, key_len, extras, extra_len, value, value_len, cas,
          opaque, ser_buf_, kSerBufSize, &bytes_written));
    }
    end = std::chrono::high_resolution_clock::now();
    auto serializing_duration_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin)
            .count();

    pantheios::log(
        pantheios::informational,
        util::fmt("Serializing done in %d ns.", serializing_duration_ns));

    ASSERT_EQ(in_buf_end_ - in_buf_, bytes_written);

    Result result("libmemcached", key_len_, extras_len_, value_len_,
                  num_repeats_, bytes_read, -1, parsing_duration_ns,
                  parsing_incr_duration_ns, serializing_duration_ns);
    result_records_.push_back(result);
  }

  memcached_result_free(pres);
}

void MemcachedEvaluator::run() {
  char* stack_buf = reinterpret_cast<char*>(malloc(kStackBufSize));
  char* out_buf = reinterpret_cast<char*>(malloc(kOutBufSize));
  in_buf_ = reinterpret_cast<char*>(malloc(kInBufSize));
  ser_buf_ = reinterpret_cast<char*>(malloc(kSerBufSize));

  area_ = new (out_buf) dr::unit::UnitArea(kOutBufSize);
  dr::parsing::ParserState state(stack_buf, kStackBufSize);
  state_ = &state;

  key_len_ = 35;
  extras_len_ = 16;

  value_len_ = 0;
  runAndCheck();

  value_len_ = 1 * 1024;
  runAndCheck();

  value_len_ = 3 * 1024;
  runAndCheck();

  value_len_ = 5 * 1024;
  runAndCheck();

  value_len_ = 10 * 1024;
  runAndCheck();

  value_len_ = 15 * 1024;
  runAndCheck();

  value_len_ = 20 * 1024;
  runAndCheck();

  printResults();
}

void MemcachedEvaluator::runAndCheck() {
  fillInputBuffer();

  pantheios::log(
      pantheios::informational,
      "Running experiments with MemcachedCommandParser/Serializer ...");

  runExperiments<memcached::MemcachedCommandParser,
                 memcached::MemcachedCommandSerializer>();

  auto command =
      reinterpret_cast<memcached::MemcachedCommand*>(area_->contents());
  ASSERT_EQ(memcached::MemcachedMagicCode::RESPONSE, command->magic_code);
  ASSERT_EQ(key_len_, command->key.len_);
  for (size_t i = 0; i < key_len_; i++) {
    ASSERT_EQ(static_cast<char>(i & 0xFF), command->key.data_[i]);
  }

  pantheios::log(pantheios::informational,
                 "Running experiments with libmemcached parser ...");

  runLibmemcachedExperiments();

  pantheios::log(pantheios::informational,
                 "Running experiments with CompactMemcachedCommandParser ...");

  runExperiments<memcached_compact::CompactMemcachedCommandParser,
                 memcached_compact::CompactMemcachedCommandSerializer>();

  auto command_c =
      reinterpret_cast<memcached_compact::CompactMemcachedCommand*>(
          area_->contents());
  // ASSERT_EQ(memcached_compact::MemcachedMagicCode::RESPONSE,
  //          command_c->magic_code);
  ASSERT_EQ(memcached_compact::MemcachedOpCode::SET, command_c->opcode);
  ASSERT_EQ(key_len_, command_c->key.len_);
  for (size_t i = 0; i < key_len_; i++) {
    ASSERT_EQ(static_cast<char>(i & 0xFF), command_c->key.data_[i]);
  }

  pantheios::log(pantheios::informational, "Finished.");
}

void MemcachedEvaluator::fillInputBuffer() {
  // see https://code.google.com/p/memcached/wiki/MemcacheBinaryProtocol
  pantheios::log(pantheios::informational, "Filling input stream ...");

  size_t total_len = value_len_ + extras_len_ + key_len_;

  // fixed size fields
  in_buf_[0] = 0x81;
  in_buf_[1] = 0x01;
  in_buf_[2] = reinterpret_cast<char*>(&key_len_)[1];
  in_buf_[3] = reinterpret_cast<char*>(&key_len_)[0];
  in_buf_[4] = reinterpret_cast<char*>(&extras_len_)[0];
  in_buf_[5] = 0x00;
  in_buf_[6] = 0x00;
  in_buf_[7] = 0x00;
  in_buf_[8] = reinterpret_cast<char*>(&total_len)[3];
  in_buf_[9] = reinterpret_cast<char*>(&total_len)[2];
  in_buf_[10] = reinterpret_cast<char*>(&total_len)[1];
  in_buf_[11] = reinterpret_cast<char*>(&total_len)[0];
  for (int i = 12; i <= 23; i++) in_buf_[i] = 0x00;

  // variable size fields
  int pos = 23;
  for (size_t i = 0; i < extras_len_; i++) {
    in_buf_[++pos] = static_cast<char>(i & 0xFF);
  }
  for (size_t i = 0; i < key_len_; i++) {
    in_buf_[++pos] = static_cast<char>(i & 0xFF);
  }
  for (size_t i = 0; i < value_len_; i++) {
    in_buf_[++pos] = static_cast<char>(i & 0xFF);
  }

  auto in_buf_len = ++pos;
  in_buf_end_ = in_buf_ + in_buf_len;
}

void MemcachedEvaluator::printResults() {
  std::cout << util::fmt("%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s", "parser",
                         "key_len", "extras_len", "value_len",
                         "num_experiments", "num_repeats", "msg_size_wire",
                         "msg_size_parsed", "parsing_duration_ns",
                         "parsing_incr_duration_ns", "serializing_duration_ns")
            << std::endl;
  for (const auto& r : result_records_) {
    std::cout << util::fmt("%s;%d;%d;%d;%d;%d;%d;%d;%d;%d", r.parser_,
                           r.key_len_, r.extras_len_, r.value_len_,
                           r.num_repeats_, r.msg_size_wire_, r.msg_size_parsed_,
                           r.parsing_duration_ns_, r.parsing_incr_duration_ns_,
                           r.serializing_duration_ns_) << std::endl;
  }
}
