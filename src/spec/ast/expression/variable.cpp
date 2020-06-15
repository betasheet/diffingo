/*
 * variable.cpp
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

#include "spec/ast/expression/variable.h"
#include "spec/ast/variable/variable.h"

#include <string>

#include "spec/ast/node.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace expression {

Variable::Variable(node_ptr<variable::Variable> var, const Location& l)
    : Expression(l), var_(var) {
  addChild(var_);
}

Variable::~Variable() {}

node_ptr<type::Type> Variable::type() const { return var_->type(); }

node_ptr<Expression> Variable::clone() {
  return ast::newNodePtr(std::make_shared<Variable>(variable(), location()));
}

std::string Variable::render() {
  return var_->id()->render() + " " + Expression::render();
}

}  // namespace expression
}  // namespace ast
}  // namespace spec
}  // namespace diffingo
