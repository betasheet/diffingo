/*
 * scope.h
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

#ifndef SRC_SPEC_AST_SCOPE_H_
#define SRC_SPEC_AST_SCOPE_H_

#include <cassert>
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

namespace diffingo {
namespace spec {
namespace ast {

class Scope {
 private:
  typedef node_ptr<expression::Expression> Value;
  typedef std::map<std::string, std::shared_ptr<Scope>> scope_map;
  typedef std::map<std::string, std::shared_ptr<std::list<Value>>> value_map;

  struct Data {
    node_ptr<ID> id = nullptr;
    scope_map children;
    value_map values;
  };

 public:
  /// Constructor.
  ///
  /// parent: A parent scope. If an ID is not found in the current scope,
  /// lookup() will forward recursively to parent scopes.
  explicit Scope(std::shared_ptr<Scope> parent = nullptr);

  virtual ~Scope();

  /// Associates an ID with the scope.
  void set_id(node_ptr<ID> id) { data_->id = id; }

  /// Returns an associated ID, or null if none.
  node_ptr<ID> id() const { return data_->id; }

  /// Returns the scope's parent, or null if none.
  std::shared_ptr<Scope> parent() const { return parent_; }

  /// Sets the scope's parent.
  ///
  /// parent: The parent.
  void set_parent(std::shared_ptr<Scope> parent) { parent_ = parent; }

  /// Adds a child scope.
  ///
  /// id: The name of child's scope. If that's, e.g., \c Foo, lookup() now
  /// searches there when \c Foo::bar is queried.
  ///
  /// child: The child scope.
  void addChild(node_ptr<ID> id, std::shared_ptr<Scope> child) {
    data_->children.insert(
        typename scope_map::value_type(id->pathAsString(), child));
    // TODO(ES): Should this update old_child->parent? We might need to use new
    // the alias support more if changed.
  }

  /// Returns the complete mapping of identifier to values. Note that
  /// because one can't insert scoped identifiers, all the strings represent
  /// identifier local to this scope.
  const value_map& map() const { return data_->values; }

  /// Inserts an identifier into the scope. If the ID already exists, the
  /// value is added to its list.
  ///
  /// id: The ID. It must not be scoped.
  ///
  /// value: The value to associate with the ID.
  ///
  /// use_scope: If true, the name will be inserted with it's
  /// fully-qualified ID right into this scope. Look-ups will only find
  /// that if they are likewise fully-qualified. If false, any scope
  /// qualifiers are ignored (i.e., we assume the ID is local to this
  /// scope).
  bool insert(node_ptr<ID> id, Value value, bool use_scope = false);

  /// Lookups an ID.
  ///
  /// id: The ID, which may be scoped or unscoped.
  ///
  /// traverse: If true, the lookup traverses parent and child scopes as
  /// appropiate.
  ///
  /// Returns: The list of values associated with the ID, or an empty list
  /// if it was not found.
  std::list<Value> lookup(node_ptr<ID> id, bool traverse = true) const {
    auto val = find(id, traverse);
    if (val.size()) return val;

    return parent_ && traverse ? parent_->lookup(id, traverse) : val;
  }

  /// Lookups an ID under the assumption that is must only have at max 1
  /// value associated with it. If it has more, that's an internal error
  /// and aborts execution.
  ///
  /// id: The ID, which may be scoped or unscoped.
  ///
  /// traverse: If true, the lookup traverses parent and child scopes as
  /// appropiate.
  ///
  /// Returns: The value associated with the ID, or null if it was not
  /// found.
  const Value lookupUnique(node_ptr<ID> id, bool traverse = true) const {
    auto vals = lookup(id, traverse);
    assert(vals.size() <= 1);
    return vals.size() ? vals.front() : nullptr;
  }

  /// Returns true if an ID exists.
  ///
  /// id: The ID, which may be scoped or unscoped.
  ///
  /// traverse: If true, the lookup traverses parent and child scopes as
  /// appropiate.
  ///
  bool has(node_ptr<ID> id, bool traverse = true) const {
    auto vals = find(id, traverse);
    if (vals.size()) return true;

    return parent_ && traverse ? parent_->has(id, traverse) : false;
  }

  /// Removes a local ID from the scope.
  ///
  /// id: The ID to remove. It must not be scoped.
  bool remove(node_ptr<ID> id);

  /// Removes all entries from the scope.
  void clear();

  /// Returns an alias to the current scope. An alias scope reflects any
  /// changed made later to the original scope, however it maintains its
  /// own parent scope pointer and can thus be linked into a different
  /// module.
  ///
  /// parent: The parent for the alias scope, or null if none.
  ///
  /// Returns: A new scope that's an alias to the current one.
  std::shared_ptr<Scope> createAlias(
      std::shared_ptr<Scope> parent = nullptr) const {
    auto alias = std::make_shared<Scope>(parent);
    alias->data_ = data_;
    return alias;
  }

  /// XXX
  void mergeFrom(std::shared_ptr<Scope> other) {
    for (auto i : other->data_->values) {
      if (data_->values.find(i.first) == data_->values.end())
        data_->values.insert(i);
    }
  }

  /// Dumps out a debugging representation of the scope's binding.
  ///
  /// out: The stream to use.
  void dump(std::ostream* out);

 private:
  void dump(std::ostream* out, int level, std::set<const Scope*>* seen);

  std::list<Value> find(node_ptr<ID> id, bool traverse) const;
  std::list<Value> find(ID::component_list::const_iterator begin,
                        ID::component_list::const_iterator end,
                        bool traverse) const;

  std::shared_ptr<Data> data_;
  std::shared_ptr<Scope> parent_;
};

}  // namespace ast
}  // namespace spec
}  // namespace diffingo

#endif  // SRC_SPEC_AST_SCOPE_H_
