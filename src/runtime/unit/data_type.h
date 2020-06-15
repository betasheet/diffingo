/*
 * data_type.h
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

#ifndef SRC_RUNTIME_UNIT_DATA_TYPE_H_
#define SRC_RUNTIME_UNIT_DATA_TYPE_H_

namespace diffingo {
namespace runtime {
namespace unit {

struct stream_range_pointer {
  char* start_;
};

struct stream_range_length {
  size_t len_;
};

struct var_stream_range {
  char* start_;
  size_t len_;
};

struct var_bytes {
  size_t len_;
  char* data_;
};

struct var_string : public var_bytes {};

template <typename ItemT>
struct list {
  typedef ItemT* pointer_array[];

  size_t len_;
  pointer_array* items_;
};

template <typename ItemT, int MaxSize>
struct mutable_list {
  static const size_t kMaxLen = MaxSize;
  typedef ItemT* pointer_array[kMaxLen];

  size_t len_;
  pointer_array items_;

  void add(ItemT* item) { items_[static_cast<int>(len_++)] = item; }
};

template <typename ItemT>
struct list_of_atomics {
  size_t len_;
  ItemT items_[];
};

template <typename ItemT, int MaxSize>
struct mutable_list_of_atomics {
  size_t len_;
  ItemT items_[MaxSize];

  void add(ItemT item) { items_[len_++] = item; }
};

}  // namespace unit
}  // namespace runtime
}  // namespace diffingo

#endif  // SRC_RUNTIME_UNIT_DATA_TYPE_H_
