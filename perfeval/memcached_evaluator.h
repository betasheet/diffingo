/*
 * memcached_evaluator.h
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

#ifndef PERFEVAL_MEMCACHED_EVALUATOR_H_
#define PERFEVAL_MEMCACHED_EVALUATOR_H_

#include <stddef.h>
#include <chrono>  // NOLINT
#include <cstdint>
#include <list>
#include <string>

#include "runtime/runtime.h"

namespace dr = diffingo::runtime;

int main(int argc, char** argv);

class MemcachedEvaluator {
 public:
  struct Result {
    std::string parser_;
    size_t key_len_;
    size_t extras_len_;
    size_t value_len_;
    size_t num_repeats_;
    size_t msg_size_wire_;
    size_t msg_size_parsed_;

    int64_t parsing_duration_ns_;
    int64_t parsing_incr_duration_ns_;
    int64_t serializing_duration_ns_;

    Result(std::string parser, size_t key_len, size_t extras_len,
           size_t value_len, size_t num_repeats, size_t msg_size_wire,
           size_t msg_size_parsed, int64_t parsing_duration_ns,
           int64_t parsing_incr_duration_ns, int64_t serializing_duration_ns)
        : parser_(parser),
          key_len_(key_len),
          extras_len_(extras_len),
          value_len_(value_len),
          num_repeats_(num_repeats),
          msg_size_wire_(msg_size_wire),
          msg_size_parsed_(msg_size_parsed),
          parsing_duration_ns_(parsing_duration_ns),
          parsing_incr_duration_ns_(parsing_incr_duration_ns),
          serializing_duration_ns_(serializing_duration_ns) {}
  };

  MemcachedEvaluator();
  virtual ~MemcachedEvaluator();

  template <typename Parser, typename Serializer>
  void runExperiments();

  void runLibmemcachedExperiments();

  void run();
  void runAndCheck();

  void fillInputBuffer();

  void printResults();

  void set_num_experiments(size_t num_experiments) {
    num_experiments_ = num_experiments;
  }

  void set_num_repeats(size_t num_repeats) { num_repeats_ = num_repeats; }

 private:
  static const size_t kStackBufSize = 2 * 1024 * 1024;
  static const size_t kOutBufSize = 2 * 1024 * 1024;
  static const size_t kInBufSize = 2 * 1024 * 1024;
  static const size_t kSerBufSize = 2 * 1024 * 1024;

  size_t key_len_ = 0;
  size_t extras_len_ = 0;
  size_t value_len_ = 0;

  size_t num_experiments_ = 5;
  size_t num_repeats_ = 10 * 1000 * 1000;

  char* in_buf_ = nullptr;
  char* in_buf_end_ = nullptr;
  char* ser_buf_ = nullptr;
  dr::unit::UnitArea* area_ = nullptr;
  dr::parsing::ParserState* state_ = nullptr;

  std::list<Result> result_records_;
};

#endif  // PERFEVAL_MEMCACHED_EVALUATOR_H_
