/*
 * instantiation.h
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

#ifndef SRC_SPEC_AST_DECLARATION_UNIT_INSTANTIATION_H_
#define SRC_SPEC_AST_DECLARATION_UNIT_INSTANTIATION_H_

#include <list>
#include <string>

#include "spec/ast/attribute.h"
#include "spec/ast/declaration/declaration.h"
#include "spec/ast/declaration/type.h"
#include "spec/ast/expression/expression.h"
#include "spec/ast/id.h"
#include "spec/ast/location.h"
#include "spec/ast/node.h"
#include "spec/ast/type/unit.h"
#include "spec/ast/visitor.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace declaration {
namespace unit_instantiation {

enum DependencyContext {
  // requested by application
  CONTEXT_APPLICATION = 1,
  // required to parse a field (or determine end of msg during parsing)
  CONTEXT_PARSING = 2,
  // required to serialize a field
  CONTEXT_SERIALIZING = 4,
  // needs to be updated when a field is serialized
  CONTEXT_SERIALIZING_UPDATE = 8
};

struct Dependency {
  node_ptr<ast::ID> id;
  int context;
};

class Instantiation : public Declaration {
 public:
  Instantiation(node_ptr<ID> id, node_ptr<ID> unit_id, inst_item_list items,
                const Location& l = Location::None);
  virtual ~Instantiation();

  node_ptr<ID> unit_id() const { return unit_id_; }
  node_ptr<type::unit::Unit> unit() const { return unit_; }
  void set_unit(node_ptr<type::unit::Unit> unit) { unit_ = unit; }

  inst_item_list items() { return items_; }

  std::list<Dependency> dependencies() const { return dependencies_; }
  void set_dependencies(std::list<Dependency> deps) { dependencies_ = deps; }

  std::list<node_ptr<spec::ast::declaration::Type>> compacted_units() const {
    return compacted_units_;
  }

  void set_compacted_units(
      std::list<node_ptr<spec::ast::declaration::Type>> compacted_units) {
    for (auto u : compacted_units_) {
      removeChild(u);
    }
    compacted_units_ = compacted_units;
    for (auto u : compacted_units_) {
      addChild(u);
    }
  }

  std::string render() override;

  ACCEPT_VISITOR(Declaration)

 private:
  node_ptr<ID> unit_id_;
  node_ptr<type::unit::Unit> unit_;
  inst_item_list items_;
  std::list<Dependency> dependencies_;

  std::list<node_ptr<spec::ast::declaration::Type>> compacted_units_;
};

class Item : public Node {
 public:
  Item(node_ptr<ID> id, const attribute_list& attrs = attribute_list(),
       bool dependency = false, const Location& l = Location::None);
  virtual ~Item();

  node_ptr<ID> id() const { return id_; }
  node_ptr<AttributeMap> attributes() const { return attributes_; }

  std::string render() override;
  bool dependency() { return dependency_; }

  ACCEPT_VISITOR(Node)

 private:
  node_ptr<ID> id_;
  node_ptr<AttributeMap> attributes_;
  bool dependency_;
};

class Property : public Item {
 public:
  explicit Property(node_ptr<Attribute> prop,
                    const Location& l = Location::None);
  virtual ~Property();

  node_ptr<Attribute> property() { return property_; }

  std::string render() override;

  ACCEPT_VISITOR(Item)

 private:
  node_ptr<Attribute> property_;
};

class Field : public Item {
 public:
  Field(node_ptr<ID> id, const attribute_list& attrs = attribute_list(),
        bool dependency = false, const Location& l = Location::None);
  virtual ~Field();

  std::string render() override;

  ACCEPT_VISITOR(Item)
};

}  // namespace unit_instantiation
}  // namespace declaration
}  // namespace ast
}  // namespace spec
}  // namespace diffingo

#endif  // SRC_SPEC_AST_DECLARATION_UNIT_INSTANTIATION_H_
