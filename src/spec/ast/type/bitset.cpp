/*
 * bitset.cpp
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

#include "spec/ast/constant/bitset.h"
#include "spec/ast/type/bitset.h"

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

Bitset::Bitset(const label_list& labels, const Location& l) : Type(l) {
  int next = 0;
  for (auto label : labels) {
    auto bit = label.second;

    if (bit < 0) bit = next;

    next = std::max(next, bit + 1);

    labels_.push_back(std::make_pair(label.first, bit));
  }

  labels_.sort([](const Label& lhs, const Label& rhs) {
    return lhs.first->name().compare(rhs.first->name()) < 0;
  });
}

Bitset::Bitset(const Location& l) : Type(l) { setWildcard(true); }

Bitset::~Bitset() {}

Bitset::label_list Bitset::labels() const {
  label_list labels;
  for (auto l : labels_) labels.push_back(l);
  return labels;
}

int Bitset::labelBit(node_ptr<ID> label) {
  for (auto l : labels_) {
    if (*l.first == *label) return l.second;
  }

  throw ast::InternalError(
      util::fmt("unknown bitset label %s", label->pathAsString().c_str()),
      this);
}

std::shared_ptr<Scope> Bitset::typeScope() {
  if (scope_) return scope_;

  scope_ = std::make_shared<Scope>();

  auto p = nodePtr<type::Type>();
  for (auto label : labels_) {
    constant::Bitset::bit_list bl;
    bl.push_back(label.first);

    auto val = std::make_shared<constant::Bitset>(bl, p, location());
    auto expr = std::make_shared<expression::Constant>(
        node_ptr<constant::Constant>(val), location());
    scope_->insert(label.first, node_ptr<expression::Expression>(expr));
  }

  return scope_;
}

ssize_t Bitset::static_serialized_length() {
  // TODO(ES): support serialized bitsets
  return -1;
}

std::string Bitset::render() {
  std::string s = "BITSET (";
  s += std::to_string(labels_.size()) + " labels) ";
  s += Type::render();
  return s;
}

}  // namespace type
}  // namespace ast
}  // namespace spec
}  // namespace diffingo
