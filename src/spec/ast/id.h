/*
 * id.h
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

#ifndef SRC_SPEC_AST_ID_H_
#define SRC_SPEC_AST_ID_H_

#include <list>
#include <string>

#include "spec/ast/location.h"
#include "spec/ast/node.h"
#include "spec/ast/visitor.h"

namespace diffingo {
namespace spec {
namespace ast {

class ID : public Node {
 public:
  typedef std::list<std::string> component_list;

  explicit ID(std::string path, const Location& l = Location::None);
  explicit ID(component_list path, const Location& l = Location::None);

  virtual ~ID();

  /// Returns the identifier as single string. For scoped identifiers, the
  /// path components will be joined by \c ::.
  ///
  /// relative_to: If given and the full names starts with a initial
  /// component like it, then that one is removed. Typically, one would
  /// pass in a module name here and the method then returns the ID as to
  /// be used from inside that module.
  std::string pathAsString(node_ptr<ID> relative_to = nullptr) const;

  component_list path() const { return path_; }

  void addPath(component_list path) {
    for (auto c : path) {
      addComponent(c);
    }
  }

  void addComponent(std::string component) { path_.push_back(component); }

  /// Returns the local component of the identifier. For \c Foo::bar this
  /// returns \c bar (as it does for \c bar).
  std::string name() const { return path_.back(); }

  std::string render() override;

  /// Returns true if the identifier has a scope.
  bool isScoped() const { return path_.size() > 1; }

  /// Returns the scope of the ID. If it's not scoped, that's the empty string.
  std::string scope() const {
    component_list p = path_;
    p.pop_back();
    return util::strjoin(p, "::");
  }

  bool operator==(const ID& other) const {
    return pathAsString() == other.pathAsString();
  }

  bool operator==(const std::string& other) const {
    return pathAsString() == other;
  }

  bool operator<(const ID& other) const {
    return pathAsString() < other.pathAsString();
  }

  bool prefixOf(const ID& other) const {
    auto opath = other.path();
    auto it = opath.begin();
    for (const auto& e : path_) {
      if (it == opath.end()) return false;
      if (*it != e) return false;
      it++;
    }
    return true;
  }

  ACCEPT_VISITOR(Node)

  static node_ptr<spec::ast::ID> combineIDs(node_ptr<spec::ast::ID> first,
                                            node_ptr<spec::ast::ID> second);

 private:
  component_list path_;
};

}  // namespace ast
}  // namespace spec
}  // namespace diffingo

#endif  // SRC_SPEC_AST_ID_H_
