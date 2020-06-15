/*
 * scope.cpp
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

#include "spec/ast/scope.h"

#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>

#include "spec/ast/expression/expression.h"
#include "spec/ast/id.h"
#include "spec/ast/node.h"
#include "util/util.h"

namespace diffingo {
namespace spec {
namespace ast {

Scope::Scope(std::shared_ptr<Scope> parent)
    : data_(std::make_shared<Data>()), parent_(parent) {}

Scope::~Scope() {}

bool Scope::insert(node_ptr<ID> id, Value value, bool use_scope) {
  auto name = use_scope ? id->pathAsString() : id->name();
  auto i = data_->values.find(name);

  if (i != data_->values.end()) {
    i->second->push_back(value);
  } else {
    auto list = std::make_shared<std::list<Value>>();
    list->push_back(value);
    data_->values.insert(typename value_map::value_type(name, list));
  }

  return true;
}

bool Scope::remove(node_ptr<ID> id) {
  auto i = data_->values.find(id->pathAsString());
  if (i == data_->values.end()) return false;

  data_->values.erase(i);
  return true;
}

void Scope::clear() {
  data_->children.clear();
  data_->values.clear();
}

void Scope::dump(std::ostream* out) {
  std::set<const Scope*> seen;
  return dump(out, 0, &seen);
}

void Scope::dump(std::ostream* out, int level, std::set<const Scope*>* seen) {
  std::string indent = "";

  for (int i = 0; i < level; ++i) indent += "    ";

  if (seen->find(this) != seen->end()) {
    *out << indent << "  (suppressed, already dumped)" << std::endl;
    return;
  }

  seen->insert(this);

  *out << indent << "* Scope" << this;

  if (parent_)
    *out << " / "
        << "has parent scope " << parent_;
  else
    *out << " / "
        << "no parent scope" << parent_;

  *out << std::endl;

  for (auto v : data_->values) {
    *out << indent << "  " << std::string(v.first) << " -> ";

    bool first = true;

    for (auto i : *v.second) {
      if (!first) *out << indent << "    ";

      *out << std::string(*i) << " " << std::endl;
      first = false;
    }
  }

  for (auto c : data_->children) {
    *out << indent << "  " << c.first << std::endl;
    c.second->dump(out, level + 1, seen);
  }
}

std::list<Scope::Value> Scope::find(node_ptr<ID> id, bool traverse) const {
  // Try a direct lookup on the full path first.
  // TODO(ES): I believe we can skip this now that the other find() tries the
  // full path.
  auto i = data_->values.find(id->pathAsString());
  if (i != data_->values.end()) return *i->second;

  // Now try the path-based lookup.
  const typename ID::component_list& path = id->path();
  return find(path.begin(), path.end(), traverse);
}

std::list<Scope::Value> Scope::find(ID::component_list::const_iterator begin,
                                    ID::component_list::const_iterator end,
                                    bool traverse) const {
  if (begin == end) return std::list<Value>();

  // Ignore the initial component if it's our own scope.
  if (id() && ::util::strtolower(*begin) == ::util::strtolower(id()->name()))
    ++begin;

  if (begin == end) return std::list<Value>();

  // See if it directly references a value.
  auto second = begin;

  if (++second == end) {
    auto val = data_->values.find(*begin);

    if (val != data_->values.end()) return *val->second;
  }

  if (!traverse)
    // Not found.
    return std::list<Value>();

  // Try lookups in child scopes of successive "left-anchored" subpaths,
  // starting with most specific.
  for (auto j = end; j != begin; --j) {
    auto id = ::util::strjoin(begin, j, "::");
    auto child = data_->children.find(id);

    if (child != data_->children.end())
      // Found a child that matches our subpath.
      return child->second->find(j, end, traverse);
  }

  // Not found.
  return std::list<Value>();
}

}  // namespace ast
}  // namespace spec
}  // namespace diffingo
