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

#include <algorithm>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include "spec/ast/exception.h"
#include "spec/ast/expression/constant.h"
#include "spec/ast/id.h"
#include "spec/ast/node.h"
#include "spec/ast/scope.h"
#include "spec/ast/type/type.h"
#include "util/util.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace type {

Enum::Enum(const label_list& labels, const Location& l) : Type(l) {
  int next = 1;
  for (auto label : labels) {
    if (*label.first == "UNDEF")
      throw ast::RuntimeError("enum label 'Undef' is already predefined", this);

    auto i = label.second;

    if (i < 0) i = next;

    next = std::max(next, i + 1);

    labels_.push_back(std::make_pair(label.first, i));
  }

  labels_.push_back(
      std::make_pair(node_ptr<ID>(std::make_shared<ID>("UNDEF")), -1));
  labels_.sort([](const Label& lhs, const Label& rhs) {
    return lhs.first->name().compare(rhs.first->name()) < 0;
  });
}

Enum::Enum(const Location& l) : Type(l) { setWildcard(true); }

Enum::~Enum() {}

Enum::label_list Enum::labels() const {
  label_list labels;
  for (const auto& l : labels_) labels.push_back(l);
  return labels;
}

int Enum::labelValue(node_ptr<ID> label) {
  for (auto l : labels_) {
    if (*l.first == *label) return l.second;
  }

  throw ast::InternalError(
      util::fmt("unknown enum label %s", label->pathAsString().c_str()), this);
}

std::shared_ptr<Scope> Enum::typeScope() {
  if (scope_) return scope_;

  scope_ = std::make_shared<Scope>();

  auto p = nodePtr<type::Type>();
  for (auto label : labels_) {
    auto val = std::make_shared<constant::Enum>(label.first, p, location());
    auto expr = std::make_shared<expression::Constant>(
        node_ptr<constant::Constant>(val), location());
    scope_->insert(label.first, node_ptr<expression::Expression>(expr));
  }

  return scope_;
}

std::string Enum::render() {
  std::string s = "ENUM (";
  s += std::to_string(labels_.size()) + " labels) ";
  s += Type::render();
  return s;
}

}  // namespace type
}  // namespace ast
}  // namespace spec
}  // namespace diffingo
