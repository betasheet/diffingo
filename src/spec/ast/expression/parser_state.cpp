/*
 * parser_state.cpp
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

#include "spec/ast/expression/parser_state.h"

#include <memory>
#include <string>

#include "spec/ast/id.h"
#include "spec/ast/node.h"
#include "spec/ast/type/atomic_types.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace expression {

ParserState::ParserState(Kind kind, node_ptr<ast::ID> id,
                         node_ptr<type::Type> unit, node_ptr<type::Type> type,
                         const Location& l)
    : Expression(l),
      kind_(kind),
      id_(id),
      unit_(unit),
      type_(type ? type
                 : node_ptr<type::Type>(std::make_shared<type::Unknown>(l))) {
  addChild(id_);
  addChild(unit_);
  addChild(type_);
}

ParserState::~ParserState() {}

void ParserState::set_unit(node_ptr<type::Type> unit) {
  removeChild(unit_);
  unit_ = unit;
  addChild(unit_);
}

void ParserState::set_type(node_ptr<type::Type> type) {
  removeChild(type_);
  type_ = (type ? type : node_ptr<type::Type>(
                             std::make_shared<type::Unknown>(location())));
  addChild(type_);
}

node_ptr<Expression> ParserState::clone() {
  return ast::newNodePtr(
      std::make_shared<ParserState>(kind(), id(), unit(), type(), location()));
}

std::string ParserState::render() {
  std::string s;
  switch (kind()) {
    case Kind::SELF:
      s += "self ";
      break;
    case Kind::DOLLARDOLLAR:
      s += "$$ ";
      break;
    case Kind::PARAMETER:
      s += "param " + id()->render() + " ";
      break;
    default:
      s += "unknown parser state expression ";
  }
  return s + Expression::render();
}

}  // namespace expression
}  // namespace ast
}  // namespace spec
}  // namespace diffingo
