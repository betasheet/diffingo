/*
 * parser_state.h
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

#ifndef SRC_RUNTIME_PARSING_PARSER_STATE_H_
#define SRC_RUNTIME_PARSING_PARSER_STATE_H_

#include <stddef.h>
#include <cassert>

namespace diffingo {
namespace runtime {
namespace parsing {

#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wsign-conversion"

class ParserState {
 public:
  ParserState(char* stack, size_t stack_size)
      : stack_(stack), stack_size_(stack_size), stack_top_(stack_) {}

  size_t space() { return stack_size_ - (stack_top_ - stack_); }

  template <typename T>
  void push(const T& t) {
    assert(space() > sizeof(t));
    *reinterpret_cast<T*>(stack_top_) = t;
    stack_top_ += sizeof(t);
  }

  template <typename T>
  T* push() {
    assert(space() > sizeof(T));
    T* pos = reinterpret_cast<T*>(stack_top_);
    stack_top_ += sizeof(T);
    return pos;
  }

  template <typename T>
  T* peek() {
    assert(space() <= stack_size_ - sizeof(T));
    return reinterpret_cast<T*>(stack_top_ - sizeof(T));
  }

  template <typename T>
  T peekCopy() {
    assert(space() <= stack_size_ - sizeof(T));
    return *reinterpret_cast<T*>(stack_top_ - sizeof(T));
  }

  template <typename T>
  void pop() {
    assert(space() <= stack_size_ - sizeof(T));
    stack_top_ -= sizeof(T);
  }

  template <typename T>
  T popAndGet() {
    T t;
    assert(space() <= stack_size_ - sizeof(t));
    stack_top_ -= sizeof(t);
    t = *reinterpret_cast<T*>(stack_top_);
    return t;
  }

  void returnToInstruction() { instruction_ = popAndGet<void*>(); }

  // TODO(ES): calling with arguments(?)
  void callInstruction(void* call, void* return_to) {
    push<void*>(return_to);
    instruction_ = call;
  }

  void advanceToInstruction(void* new_instr) { instruction_ = new_instr; }

  void* instruction() { return instruction_; }

  char** stream_pos() { return &stream_pos_; }

  void reset() {
    stack_top_ = stack_;
    instruction_ = nullptr;
    stream_pos_ = nullptr;
  }

 private:
  char* stack_;
  size_t stack_size_;

  char* stack_top_;
  void* instruction_ = nullptr;

  char* stream_pos_ = nullptr;
};

}  // namespace parsing
}  // namespace runtime
}  // namespace diffingo

#endif  // SRC_RUNTIME_PARSING_PARSER_STATE_H_
