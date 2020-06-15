/*
 * conditional.cpp
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

#include "spec/ast/expression/conditional.h"

#include <memory>
#include <string>

#include "spec/ast/type/type.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace expression {

Conditional::Conditional(node_ptr<Expression> cond, node_ptr<Expression> true__,
                         node_ptr<Expression> false__, const Location& l)
    : Expression(l), cond_(cond), true_(true__), false_(false__) {
  addChild(cond_);
  addChild(true_);
  if (false_) addChild(false_);
}

Conditional::~Conditional() {}

node_ptr<type::Type> Conditional::type() const { return true_->type(); }

node_ptr<Expression> Conditional::clone() {
  auto false_ = _false() ? _false()->clone() : nullptr;
  return ast::newNodePtr(std::make_shared<Conditional>(
      cond()->clone(), _true()->clone(), false_, location()));
}

std::string Conditional::render() {
  std::string s = "(" + cond_->render() + " ? " + true_->render();
  if (false_) s += " : " + false_->render() + " " + Expression::render();
  s += ")";
  return s;
}

}  // namespace expression
}  // namespace ast
}  // namespace spec
}  // namespace diffingo
