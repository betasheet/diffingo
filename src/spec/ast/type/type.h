/*
 * type.h
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

#ifndef SRC_SPEC_AST_TYPE_TYPE_H_
#define SRC_SPEC_AST_TYPE_TYPE_H_

#include <memory>
#include <string>

#include "spec/ast/id.h"
#include "spec/ast/location.h"
#include "spec/ast/node.h"
#include "spec/ast/visitor.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace type {

class Type : public Node {
 public:
  explicit Type(const Location& l = Location::None);
  virtual ~Type();

  /// Returns true if two types are equivalent.
  ///
  /// These rules are applied:
  ///
  ///    - Types marked with setWildcard() match any other type of the same
  ///    Type class.
  ///
  ///    - If the two types are of the the same type class, the result of
  ///      _equals() is passed on (which defaults to true).
  ///
  ///    - Otherwise, comparison fails.
  virtual bool equals(node_ptr<Type> other) const;

/// Compares two type instances of the same Type class. The default
/// implementation returns true, but can be overridden.
///
/// other: The instance to compare with. It's guaranteed that this is of
/// the same type as \c *this.
///
/// Note, when overriding, make sure to keep comparison commutative.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
  virtual bool _equals(node_ptr<Type> other) const { return true; }
#pragma GCC diagnostic pop

  /// Returns true if the type has been marked a wildcard type. See
  /// setWildcard().
  bool wildcard(void) const { return wildcard_; }

  /// Marks the type as wildcard type. When comparing types with
  /// operator==(), a wilcard type matches all other instances of the same
  /// Type class.
  void setWildcard(bool wildcard) { wildcard_ = wildcard; }

  /// Returns a scope of sub-identifiers that are relative to the type. If a
  /// type defines IDs in this scope and a global type declaration is added
  /// to a module, these IDs will be accessible via scoping relative to the
  /// declaration. The returned scope should not have its parent set.
  virtual std::shared_ptr<Scope> typeScope();

  node_ptr<ID> id() const { return id_; }

  void set_id(node_ptr<ID> id) {
    if (id_) removeChild(id_);

    id_ = id;
    addChild(id_);
  }

  const std::string& scope() const { return scope_; }

  void set_scope(const std::string& scope) { scope_ = scope; }

  virtual ssize_t static_serialized_length();

  std::string render() override;

  ACCEPT_VISITOR(Node)

 private:
  bool wildcard_ = false;
  node_ptr<ID> id_;
  std::string scope_;
};

}  // namespace type
}  // namespace ast
}  // namespace spec
}  // namespace diffingo

#endif  // SRC_SPEC_AST_TYPE_TYPE_H_
