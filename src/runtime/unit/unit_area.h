/*
 * unit_area.h
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

#ifndef SRC_RUNTIME_UNIT_UNIT_AREA_H_
#define SRC_RUNTIME_UNIT_UNIT_AREA_H_

#include <stddef.h>
#include <cassert>

namespace diffingo {
namespace runtime {
namespace unit {

#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wsign-conversion"

class UnitArea {
 public:
  explicit UnitArea(size_t bufferSize)
      : nextAllocPos_(contents()), size_(bufferSize - sizeof(*this)) {}

  char *contents() { return reinterpret_cast<char *>(this) + sizeof(*this); }

  size_t size() const { return size_; }

  size_t allocated() { return nextAllocPos_ - contents(); }

  size_t space() { return size() - allocated(); }

  char *nextPos() const { return nextAllocPos_; }

  bool allocate(size_t size, char **pos) {
    if (space() < size) return false;
    *pos = nextAllocPos_;
    nextAllocPos_ += size;
    return true;
  }

  template <typename ItemT>
  bool allocate(ItemT **item) {
    return allocate(sizeof(**item), reinterpret_cast<char **>(item));
  }

  template <typename ItemT>
  bool allocate_new(ItemT **item) {
    if (!allocate(sizeof(**item), item)) return false;
    *item = new (*item) ItemT;  // call constructor
    return true;
  }

  void moveNextPos(size_t _size) {
    nextAllocPos_ += _size;
    assert(space() <= size());
  }

  void reset() {
    nextAllocPos_ = contents();
  }

 private:
  char *nextAllocPos_;
  size_t size_;
};

}  // namespace unit
}  // namespace runtime
}  // namespace diffingo

#endif  // SRC_RUNTIME_UNIT_UNIT_AREA_H_
