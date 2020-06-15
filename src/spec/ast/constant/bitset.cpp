/*
 * bitset.cpp
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

#include "spec/ast/constant/bitset.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace constant {

Bitset::Bitset(const bit_list& bits, node_ptr<type::Type> type,
               const Location& l)
    : Constant(l), bits_(bits), type_(checkedCast<type::Bitset>(type)) {
  // Check that we know all the bit labels.
  for (auto b : bits) {
    bool found = false;

    for (auto l : type_->labels()) {
      if (b == l.first) {
        found = 1;
        break;
      }
    }

    if (!found)
      throw ConstantParseError(this, util::fmt("unknown bitset label '%s'",
                                               b->pathAsString().c_str()));
  }
}

Bitset::~Bitset() {}

}  // namespace constant
}  // namespace ast
}  // namespace spec
}  // namespace diffingo
