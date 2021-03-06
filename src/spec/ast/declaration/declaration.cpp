/*
 * declaration.cpp
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

#include "spec/ast/declaration/declaration.h"

#include <memory>
#include <string>

#include "spec/ast/id.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace declaration {

Declaration::Declaration(node_ptr<ID> id, Linkage linkage,
                         const Location& l)
    : Node(l), id_(id), linkage_(linkage) {
  addChild(id_);
}

Declaration::~Declaration() {}

std::string Declaration::linkageAsString() const {
  switch (linkage_) {
    case EXPORTED:
      return "export";
    case IMPORTED:
      return "import";
    default:
      return "";
  }
}

}  // namespace declaration
}  // namespace ast
}  // namespace spec
}  // namespace diffingo
