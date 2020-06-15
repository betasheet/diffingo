/*
 * location.cpp
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

#include "spec/ast/location.h"

#include <string>

#include "util/util.h"

namespace diffingo {
namespace spec {
namespace ast {

const Location Location::None("<no location>");

Location::Location(const std::string& file, int from_line, int to_line)
    : file_(file), from_(from_line), to_(to_line) {}

Location::~Location() {
  // TODO(Eric Seckler): Auto-generated destructor stub
}

Location::operator std::string() const {
  if (this == &None) return "<no location>";

  std::string s = file_.size() ? file_ : "<no filename>";
  s += ":";

  if (from_ >= 0) {
    if (to_ >= 0)
      s += util::fmt("%d-%d", from_, to_);
    else
      s += util::fmt("%d", from_);
  }

  return s;
}

}  // namespace ast
}  // namespace spec
}  // namespace diffingo
