/*
 * id.cpp
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

#include "spec/ast/expression/id.h"
#include "spec/ast/id.h"

#include <memory>
#include <string>

#include "spec/ast/type/atomic_types.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace expression {

ID::ID(node_ptr<ast::ID> id, const Location& l)
    : Expression(l),
      id_(id),
      type_(node_ptr<type::Type>(std::make_shared<type::Unknown>())) {
  addChild(id_);
  addChild(type_);
}

ID::~ID() {}

node_ptr<Expression> ID::clone() {
  return ast::newNodePtr(std::make_shared<ID>(id(), location()));
}

std::string ID::render() { return id_->render() + " " + Expression::render(); }

}  // namespace expression
}  // namespace ast
}  // namespace spec
}  // namespace diffingo
