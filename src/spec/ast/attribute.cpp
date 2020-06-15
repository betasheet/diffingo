/*
 * attribute.cpp
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

#include "spec/ast/attribute.h"

#include <stddef.h>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include "spec/ast/expression/expression.h"
#include "spec/ast/node.h"

namespace diffingo {
namespace spec {
namespace ast {

Attribute::Attribute(const std::string& key,
                     node_ptr<expression::Expression> value, bool internal,
                     const Location& l)
    : Node(l), key_(key), value_(value), internal_(internal) {
  while (key_.size() && (strchr("&%.", key_[0]) != 0))
    key_ = key_.substr(1, std::string::npos);
  addChild(value_);
}

Attribute::~Attribute() {}

bool Attribute::operator==(const Attribute& other) const {
  return key_ == other.key();
}

node_ptr<Attribute> Attribute::clone() const {
  return ast::newNodePtr(std::make_shared<Attribute>(key(), value()->clone(),
                                                     internal(), location()));
}

std::string Attribute::render() {
  std::string s = key_;
  if (internal_) s += " (internal)";
  if (value_) s += " = " + value_->render();
  return s;
}

AttributeMap::AttributeMap(const Location& l) : Node(l) {}

AttributeMap::AttributeMap(const attribute_list& attrs, const Location& l)
    : Node(l) {
  for (auto a : attrs) add(a);
}

AttributeMap::~AttributeMap() {}

node_ptr<Attribute> AttributeMap::add(node_ptr<Attribute> attr) {
  node_ptr<Attribute> old = nullptr;

  auto i = attrs_.find(attr->key());

  if (i != attrs_.end()) {
    old = i->second;
    attrs_.erase(i);
    removeChild(old);
  }

  auto p = attrs_.insert(std::make_pair(attr->key(), attr));
  addChild(p.first->second);

  return old;
}

node_ptr<Attribute> AttributeMap::remove(const std::string& key) {
  node_ptr<Attribute> old = nullptr;

  auto i = attrs_.find(key);

  if (i != attrs_.end()) {
    old = i->second;
    attrs_.erase(i);
    removeChild(old);
  }

  return old;
}

bool AttributeMap::has(const std::string& key) const {
  return lookup(key) != nullptr;
}

node_ptr<Attribute> AttributeMap::lookup(const std::string& key) const {
  auto i = attrs_.find(key);
  return i != attrs_.end() ? i->second : nullptr;
}

std::list<node_ptr<Attribute>> AttributeMap::attributes() const {
  std::list<node_ptr<Attribute>> attrs;

  for (auto i : attrs_) attrs.push_back(i.second);

  return attrs;
}

size_t AttributeMap::size() const {
  return attrs_.size();
}

AttributeMap& AttributeMap::operator=(const attribute_list& attrs) {
  attrs_.clear();

  for (auto a : attrs) add(a);

  return *this;
}

std::list<node_ptr<Attribute>> AttributeMap::clonedAttributes() const {
  std::list<node_ptr<Attribute>> attrs;

  for (auto i : attrs_) attrs.push_back(i.second->clone());

  return attrs;
}

std::string AttributeMap::render() {
  std::string s = "attrs {";
  bool first = true;
  for (auto i : attrs_) {
    if (first)
      first = false;
    else
      s += ", ";
    s += i.second->render();
  }
  s += "}";
  return s;
}

}  // namespace ast
}  // namespace spec
}  // namespace diffingo
