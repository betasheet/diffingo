/*
 * tuple.cpp
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

#include "spec/ast/constant/tuple.h"

#include <sys/types.h>
#include <list>
#include <memory>
#include <string>

#include "autogen/spec/parser/location.hh"
#include "spec/ast/expression/constant.h"
#include "spec/ast/expression/expression.h"
#include "spec/ast/node.h"
#include "spec/ast/type/atomic_types.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace constant {

Tuple::Tuple(const expression_list& elems, const Location& l) : Constant(l) {
  for (auto e : elems) elems_.push_back(e);
  for (auto e : elems_) addChild(e);
}

Tuple::~Tuple() {}

node_ptr<type::Type> Tuple::type() const {
  type_list types;

  for (auto e : elems_) {
    types.push_back(e->type());
  }

  return node_ptr<type::Tuple>(
      std::make_shared<type::Tuple>(types, location()));
}

expression_list Tuple::value() const { return elems_; }

ssize_t Tuple::static_serialized_length() {
  ssize_t len = 0;
  for (auto e : elems_) {
    if (auto ec = ast::tryCast<expression::Constant>(e)) {
      auto c = ec->constant();
      auto c_len = c->static_serialized_length();
      if (c_len < 0) return -1;
      len += c_len;
    } else {
      return -1;
    }
  }
  return len;
}

std::string Tuple::render() {
  std::string s = "(";

  bool first = true;
  for (auto e : elems_) {
    if (first)
      first = false;
    else
      s += ", ";

    s += e->render();
  }

  s += ") " + Constant::render();
  return s;
}

}  // namespace constant
}  // namespace ast
}  // namespace spec
}  // namespace diffingo
