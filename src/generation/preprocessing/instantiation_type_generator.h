/*
 * instantiation_type_generator.h
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

#ifndef SRC_GENERATION_PREPROCESSING_INSTANTIATION_TYPE_GENERATOR_H_
#define SRC_GENERATION_PREPROCESSING_INSTANTIATION_TYPE_GENERATOR_H_

#include <list>

#include "generation/pass.h"
#include "spec/ast/declaration/type.h"
#include "spec/ast/declaration/unit_instantiation.h"
#include "spec/ast/expression/parser_state.h"
#include "spec/ast/id.h"
#include "spec/ast/module.h"
#include "spec/ast/node.h"
#include "spec/ast/type/unit.h"
#include "spec/ast/visitor.h"

namespace diffingo {
namespace generation {
namespace preprocessing {

using spec::ast::node_ptr;

class SelfReplacerVisitor : public spec::ast::Visitor<> {
 public:
  SelfReplacerVisitor();
  virtual ~SelfReplacerVisitor();

  bool run(node_ptr<spec::ast::Node> node,
           node_ptr<spec::ast::type::unit::Unit> unit);

  using Visitor::visit;

  void visit(node_ptr<spec::ast::type::unit::Unit> node);
  void visit(node_ptr<spec::ast::type::unit::item::Item> node);
  void visit(node_ptr<spec::ast::expression::Expression> node);
  void visit(node_ptr<spec::ast::expression::ParserState> node);

 private:
  node_ptr<spec::ast::type::unit::Unit> unit_;
};

class InstantiationTypeGenerator : public generation::Pass<> {
 public:
  InstantiationTypeGenerator();
  virtual ~InstantiationTypeGenerator();

  bool run(node_ptr<spec::ast::Module> node, const Options& options) override;

  using Visitor::visit;

  void visit(
      node_ptr<spec::ast::declaration::unit_instantiation::Instantiation> node);
  void visit(node_ptr<spec::ast::type::unit::Unit> node);
  void visit(node_ptr<spec::ast::type::unit::item::Item> node);

 private:
  void compactAndAddItems();
  int dependencyContextFor(node_ptr<spec::ast::ID> id);
  node_ptr<spec::ast::type::unit::item::Item> copyItem(
      node_ptr<spec::ast::type::unit::item::Item> item);
  void addFieldOfLength(node_ptr<spec::ast::expression::Expression> length);

  std::list<spec::ast::declaration::unit_instantiation::Dependency> deps_;
  std::list<node_ptr<spec::ast::declaration::Type>> unit_decls_;
  node_ptr<spec::ast::ID> cur_unit_name_;
  node_ptr<spec::ast::ID> cur_unit_id_;
  spec::ast::unit_item_list cur_unit_items_;
  spec::ast::unit_field_list cur_compactable_fields_;

  SelfReplacerVisitor replacer_;
};

}  // namespace preprocessing
}  // namespace generation
}  // namespace diffingo

#endif  // SRC_GENERATION_PREPROCESSING_INSTANTIATION_TYPE_GENERATOR_H_
