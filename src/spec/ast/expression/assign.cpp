/*
 * assign.cpp
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

#include "spec/ast/expression/assign.h"

#include <memory>
#include <string>

#include "spec/ast/type/type.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace expression {

Assign::Assign(node_ptr<Expression> dst, node_ptr<Expression> src,
               const Location& l)
    : Expression(l), src_(src), dst_(dst) {
  addChild(dst_);
  addChild(src_);
}

Assign::~Assign() {}

node_ptr<type::Type> Assign::type() const { return dst_->type(); }

node_ptr<Expression> Assign::clone() {
  return ast::newNodePtr(
      std::make_shared<Assign>(dst()->clone(), src()->clone(), location()));
}

std::string Assign::render() {
  return "(" + dst_->render() + " = " + src_->render() + " " +
         Expression::render() + ")";
}

}  // namespace expression
}  // namespace ast
}  // namespace spec
}  // namespace diffingo
