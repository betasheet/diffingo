/*
 * instantiation_dependency_resolver.h
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

#ifndef SRC_GENERATION_PREPROCESSING_INSTANTIATION_DEPENDENCY_RESOLVER_H_
#define SRC_GENERATION_PREPROCESSING_INSTANTIATION_DEPENDENCY_RESOLVER_H_

#include <list>
#include <map>
#include <memory>
#include <set>
#include <stack>
#include <string>

#include "generation/pass.h"
#include "spec/ast/declaration/unit_instantiation.h"
#include "spec/ast/expression/conditional.h"
#include "spec/ast/expression/constant.h"
#include "spec/ast/expression/find.h"
#include "spec/ast/expression/id.h"
#include "spec/ast/expression/lambda.h"
#include "spec/ast/expression/list_comprehension.h"
#include "spec/ast/expression/member_attribute.h"
#include "spec/ast/expression/operator.h"
#include "spec/ast/expression/parser_state.h"
#include "spec/ast/expression/variable.h"
#include "spec/ast/id.h"
#include "spec/ast/module.h"
#include "spec/ast/node.h"
#include "spec/ast/type/unit.h"
#include "spec/ast/visitor.h"

namespace diffingo {
namespace generation {
namespace preprocessing {

typedef std::map<
    spec::ast::ID,
    std::list<spec::ast::declaration::unit_instantiation::Dependency>>
    DependencyMap;

class InstantiationDependencyResolver : public generation::Pass<> {
 public:
  InstantiationDependencyResolver();
  virtual ~InstantiationDependencyResolver();

  bool run(node_ptr<spec::ast::Module> node,
           const Options& options) override;

  using Visitor::visit;

  void visit(
      node_ptr<spec::ast::declaration::unit_instantiation::Instantiation> node);
};

class DependencyMapBuilder : public spec::ast::Visitor<> {
 public:
  DependencyMapBuilder();
  virtual ~DependencyMapBuilder();

  std::shared_ptr<DependencyMap> run(
      node_ptr<spec::ast::type::unit::Unit> node);

  using Visitor::visit;

  void visit(node_ptr<spec::ast::type::unit::Unit> node);
  void visit(node_ptr<spec::ast::type::unit::item::Item> node);
  void visit(node_ptr<spec::ast::type::unit::item::field::Field> node);
  void visit(node_ptr<spec::ast::type::unit::item::field::Unit> node);
  void visit(
      node_ptr<spec::ast::type::unit::item::field::switch_::Switch> node);
  void visit(
      node_ptr<spec::ast::type::unit::item::field::container::List> node);
  void visit(
      node_ptr<spec::ast::type::unit::item::field::container::Vector> node);
  void visit(node_ptr<spec::ast::expression::ID> node);
  void visit(node_ptr<spec::ast::expression::MemberAttribute> node);
  void visit(node_ptr<spec::ast::expression::Variable> node);
  void visit(node_ptr<spec::ast::expression::Operator> node);
  void visit(node_ptr<spec::ast::expression::Constant> node);
  void visit(node_ptr<spec::ast::expression::Conditional> node);
  void visit(node_ptr<spec::ast::expression::Find> node);
  void visit(node_ptr<spec::ast::expression::Lambda> node);
  void visit(node_ptr<spec::ast::expression::ListComprehension> node);
  void visit(node_ptr<spec::ast::expression::ParserState> node);

  const node_ptr<spec::ast::ID> length_tracking_id() {
    return length_tracking_id_;
  }

 private:
  void addDependency(node_ptr<spec::ast::ID> from, node_ptr<spec::ast::ID> to,
                     int context);
  void track(node_ptr<spec::ast::expression::ID> id);
  void track(node_ptr<spec::ast::ID> id);
  void trackElement();
  void pushTracking();
  node_ptr<spec::ast::ID> popTracking();

  std::shared_ptr<DependencyMap> map_;
  std::list<node_ptr<spec::ast::ID>> from_;

  std::stack<node_ptr<spec::ast::ID>> tracking_stack_;
  node_ptr<spec::ast::ID> cur_unit_;
  node_ptr<spec::ast::ID> cur_dollar_dollar_;
  node_ptr<spec::ast::ID> cur_find_list_;
  int cur_context_ = spec::ast::declaration::unit_instantiation::
                         DependencyContext::CONTEXT_PARSING &
                     spec::ast::declaration::unit_instantiation::
                         DependencyContext::CONTEXT_SERIALIZING;

  const node_ptr<spec::ast::ID> element_tracking_id_;
  const node_ptr<spec::ast::ID> length_tracking_id_;
  std::set<std::string> ignore_attributes_;
};

}  // namespace preprocessing
}  // namespace generation
}  // namespace diffingo

#endif  // SRC_GENERATION_PREPROCESSING_INSTANTIATION_DEPENDENCY_RESOLVER_H_
