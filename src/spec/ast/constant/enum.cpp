/*
 * enum.cpp
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

#include "spec/ast/constant/enum.h"
#include "spec/ast/type/enum.h"

#include <string>
#include <utility>

#include "spec/ast/id.h"
#include "spec/ast/node.h"
#include "util/util.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace constant {

Enum::Enum(node_ptr<ID> label, node_ptr<type::Type> type, const Location& l)
    : Constant(l), label_(label), type_(checkedCast<type::Enum>(type)) {
  // Check that we know the label.
  for (auto l : type_->labels()) {
    if (*label_ == *l.first) return;
  }

  throw ConstantParseError(this, util::fmt("unknown enum label '%s'",
                                           label_->pathAsString().c_str()));
}

Enum::~Enum() {}

}  // namespace constant
}  // namespace ast
}  // namespace spec
}  // namespace diffingo
