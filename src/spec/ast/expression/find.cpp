/*
 * find.cpp
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

#include "spec/ast/expression/find.h"

#include <string>

#include "spec/ast/expression/lambda.h"
#include "spec/ast/node.h"
#include "spec/ast/type/type.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace expression {

Find::Find(node_ptr<Expression> list_expr, node_ptr<Lambda> cond_expr,
           node_ptr<Lambda> found_expr, node_ptr<Expression> not_found_expr,
           const Location& l)
    : Expression(l),
      list_expr_(list_expr),
      cond_expr_(cond_expr),
      found_expr_(found_expr),
      not_found_expr_(not_found_expr) {
  addChild(list_expr_);
  addChild(cond_expr_);
  addChild(found_expr_);
  addChild(not_found_expr_);
}

Find::~Find() {}

node_ptr<type::Type> Find::type() const { return found_expr_->type(); }

node_ptr<Expression> Find::clone() {
  return ast::newNodePtr(std::make_shared<Find>(
      list_expr()->clone(), checkedCast<Lambda>(cond_expr()->clone()),
      checkedCast<Lambda>(found_expr()->clone()), not_found_expr()->clone(),
      location()));
}

std::string Find::render() {
  return "(find(" + list_expr_->render() + ", " + cond_expr_->render() + ", " +
         found_expr_->render() + ", " + not_found_expr_->render() + ") " +
         Expression::render() + ")";
}

}  // namespace expression
}  // namespace ast
}  // namespace spec
}  // namespace diffingo
