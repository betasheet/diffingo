/*
 * list_comprehension.cpp
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

#include "spec/ast/expression/list_comprehension.h"

#include <memory>
#include <string>

#include "spec/ast/expression/expression.h"
#include "spec/ast/expression/id.h"
#include "spec/ast/id.h"
#include "spec/ast/type/container.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace expression {

ListComprehension::ListComprehension(node_ptr<Expression> output,
                                     node_ptr<ast::ID> variable,
                                     node_ptr<Expression> input,
                                     node_ptr<Expression> predicate,
                                     const Location& l)
    : Expression(l),
      output_(output),
      variable_(variable),
      input_(input),
      predicate_(predicate) {
  addChild(output_);
  addChild(variable_);
  addChild(input_);
  addChild(predicate_);
}

ListComprehension::~ListComprehension() {}

node_ptr<type::Type> ListComprehension::type() const {
  return node_ptr<type::Type>(std::make_shared<type::List>(output_->type()));
}

node_ptr<Expression> ListComprehension::clone() {
  return ast::newNodePtr(std::make_shared<ListComprehension>(
      output()->clone(), variable(), input()->clone(), predicate()->clone(),
      location()));
}

std::string ListComprehension::render() {
  auto s = "(" + output_->render() + " for " + variable_->render() + " in " +
           input_->render() + " ";
  if (predicate_) s += "if " + predicate_->render() + " ";
  s += Expression::render() + ")";
  return s;
}

}  // namespace expression
}  // namespace ast
}  // namespace spec
}  // namespace diffingo
