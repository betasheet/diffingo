/*
 * id_resolver.cpp
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

#include "generation/preprocessing/id_resolver.h"

#include <pantheios/pantheios.hpp>
#include <cassert>
#include <iterator>
#include <list>
#include <memory>
#include <string>

#include "spec/ast/declaration/unit_instantiation.h"
#include "spec/ast/exception.h"
#include "spec/ast/expression/constant.h"
#include "spec/ast/expression/ctor.h"
#include "spec/ast/expression/expression.h"
#include "spec/ast/expression/find.h"
#include "spec/ast/expression/id.h"
#include "spec/ast/expression/lambda.h"
#include "spec/ast/expression/list_comprehension.h"
#include "spec/ast/expression/type.h"
#include "spec/ast/expression/variable.h"
#include "spec/ast/id.h"
#include "spec/ast/location.h"
#include "spec/ast/module.h"
#include "spec/ast/node.h"
#include "spec/ast/scope.h"
#include "spec/ast/type/atomic_types.h"
#include "spec/ast/type/type.h"
#include "spec/ast/type/unit.h"
#include "spec/ast/variable/variable.h"
#include "spec/ast/visitor.h"
#include "util/util.h"

namespace ast = diffingo::spec::ast;

namespace diffingo {
namespace generation {
namespace preprocessing {

using ast::Scope;

// A helper visitor that replaces an ID expression with a different node within
// a subtree.
class IdExprReplacer : public ast::Visitor<> {
 public:
  IdExprReplacer(node_ptr<ast::ID> old, node_ptr<ast::Node> _new) {
    old_ = old;
    new_ = _new;
  }
  virtual ~IdExprReplacer() {}

  bool run(node_ptr<ast::Node> node) { return processAllPreOrder(node); }

  using Visitor::visit;

  void visit(node_ptr<ast::expression::ID> node) override {
    if (node->id()->pathAsString() == old_->pathAsString()) node.replace(new_);
  }

 private:
  node_ptr<ast::ID> old_;
  node_ptr<ast::Node> new_;
};

IdResolver::IdResolver() {}

IdResolver::~IdResolver() {}

bool IdResolver::run(node_ptr<ast::Module> node,
                     const Options& options) {
  return processAllPreOrder(node);
}

void IdResolver::visit(node_ptr<ast::expression::ID> node) {
  // Find the nearest to scope.
  auto module = current<ast::Module>();
  std::shared_ptr<Scope> scope = nullptr;

  // std::cerr << "Lookup: " << node->id()->pathAsString() << std::endl;

  auto nodes = currentNodes();
  for (auto i = nodes.rbegin(); i != nodes.rend(); i++) {
    auto n = *i;

    if (auto item = ast::tryCast<ast::type::unit::item::Item>(n)) {
      // std::cerr << "  item: " << item.get() << std::endl;

      scope = item->scope();
      break;
    } else if (auto unit = ast::tryCast<ast::type::unit::Unit>(n)) {
      // std::cerr << "  unit: " << unit.get() << std::endl;

      scope = unit->typeScope();
      break;
    }
  }

  if (!scope) {
    // std::cerr << "  module: " << module.get() << std::endl;

    scope = module->scope();
  }

  // scope->dump(&std::cerr);
  auto vals = scope->lookup(node->id(), true);

  if (!vals.size()) {
    // Look it up in the current module if that's the namespace we are
    // interested in.
    auto path = node->id()->path();

    if (node->id()->isScoped() && path.front() == module->id()->name()) {
      path.pop_front();
      auto x = ast::newNodePtr(std::make_shared<ast::ID>(path));
      vals = scope->lookup(x, true);
    }
  }

  if (!vals.size()) {
    if (report_unresolved_)
      log(pantheios::error, node,
          util::fmt("unknown ID %s", node->id()->pathAsString().c_str()));
    return;
  }

  if (vals.size() > 1) {
    log(pantheios::error, node, util::fmt("ID %s defined more than once",
                                          node->id()->pathAsString().c_str()));
    return;
  }

  auto val = vals.front();
  if (node->id()->isScoped()) val->set_scope(node->id()->scope());

  node.replace(val);
}

void IdResolver::visit(node_ptr<ast::expression::ListComprehension> node) {
  // TODO(ES) support list comprehensions
}

void IdResolver::visit(node_ptr<spec::ast::expression::Lambda> node) {
  // determine where the type of this lambda variable comes from.
  // currently, we only support lambdas in Find expressions.
  node_ptr<ast::type::Type> elem_type = nullptr;
  auto find_expr = current<ast::expression::Find>();
  if (find_expr && find_expr->hasChild(node, false)) {
    auto l_expr = find_expr->list_expr();
    elem_type = ast::newNodePtr(
        std::make_shared<ast::type::UnknownElementType>(l_expr));
  } else {
    log(pantheios::error, node, "lambda outside find expression");
  }

  // replace lambda variable id expressions
  auto elem_id = node->elem_id();
  auto var = ast::newNodePtr(
      std::make_shared<ast::variable::Local>(elem_id, elem_type));
  auto var_expr =
      ast::newNodePtr(std::make_shared<ast::expression::Variable>(var));

  IdExprReplacer replacer(elem_id, var_expr);

  replacer.run(node->elem_expr());
}

void IdResolver::visit(
    node_ptr<spec::ast::declaration::unit_instantiation::Instantiation> node) {
  auto id = node->unit_id();

  auto module = current<ast::Module>();

  auto vals = module->scope()->lookup(id, true);

  if (!vals.size()) {
    if (report_unresolved_)
      log(pantheios::error, node,
          util::fmt("unknown type ID %s", id->pathAsString().c_str()));
    return;
  }

  if (vals.size() > 1) {
    log(pantheios::error, node,
        util::fmt("ID %s defined more than once", id->pathAsString().c_str()));
    return;
  }

  auto val = vals.front();

  auto nt = ast::tryCast<ast::expression::Type>(val);

  if (!nt) {
    log(pantheios::error, node, util::fmt("ID %s does not reference a type",
                                          id->pathAsString().c_str()));
    return;
  }

  auto tv = nt->contained_type();
  auto unit = ast::tryCast<ast::type::unit::Unit>(tv);
  if (!unit) {
    log(pantheios::error, node, util::fmt("ID %s does not reference a unit",
                                          id->pathAsString().c_str()));
    return;
  }

  node->set_unit(unit);
}

void IdResolver::visit(node_ptr<ast::type::Unknown> node) {
  auto id = node->id();
  if (!id) return;

  auto module = current<ast::Module>();

  auto vals = module->scope()->lookup(id, true);

  if (!vals.size()) {
    if (report_unresolved_)
      log(pantheios::error, node,
          util::fmt("unknown type ID %s", id->pathAsString().c_str()));
    return;
  }

  if (vals.size() > 1) {
    log(pantheios::error, node,
        util::fmt("ID %s defined more than once", id->pathAsString().c_str()));
    return;
  }

  auto val = vals.front();

  auto nt = ast::tryCast<ast::expression::Type>(val);

  if (!nt) {
    log(pantheios::error, node, util::fmt("ID %s does not reference a type",
                                          id->pathAsString().c_str()));
    return;
  }

  auto tv = nt->contained_type();
  if (!tv->id() || !(tv->id()->isScoped() && id->isScoped())) {
    tv->set_id(id);

    if (!id->isScoped()) tv->set_scope(module->id()->name());
  }

  node.replace(tv);
}

void IdResolver::visit(node_ptr<ast::type::unit::item::field::Unknown> node) {
  auto id = node->referenced_id();
  if (!id)
    throw ast::InternalError("field::Unknown without referenced ID",
                             node.get());

  auto module = current<ast::Module>();

  auto exprs = module->scope()->lookup(id, true);

  if (!exprs.size()) {
    if (report_unresolved_)
      log(pantheios::error, node,
          util::fmt("unknown field ID %s", id->pathAsString()));
    return;
  }

  if (exprs.size() > 1) {
    log(pantheios::error, node,
        util::fmt("ID %s references more than one object", id->pathAsString()));
    return;
  }

  auto expr = exprs.front();

  auto attributes = node->attributes()->attributes();
  auto condition = node->condition();
  auto name = node->id();
  auto location = node->location();
  auto params = node->parameters();
  auto sinks = node->sinks();

  node_ptr<ast::type::unit::item::field::Field> nfield = nullptr;

  if (auto ctor = ast::tryCast<ast::expression::Ctor>(expr)) {
    nfield =
        ast::newNodePtr(std::make_shared<ast::type::unit::item::field::Ctor>(
            name, ctor->ctor(), condition, attributes, sinks, location));
  } else if (auto constant = ast::tryCast<ast::expression::Constant>(expr)) {
    nfield = ast::newNodePtr(
        std::make_shared<ast::type::unit::item::field::Constant>(
            name, constant->constant(), condition, attributes, sinks,
            location));
  } else if (auto type = ast::tryCast<ast::expression::Type>(expr)) {
    nfield = ast::type::unit::item::field::Field::createByType(
        type->contained_type(), name, condition, attributes, params, sinks,
        location);
  } else {
    log(pantheios::error, node,
        util::fmt("ID %s references unexpected type of expression",
                  id->pathAsString()));
    return;
  }
  assert(nfield);

  nfield->scope()->set_parent(node->scope()->parent());
  if (node->anonymous()) nfield->set_anonymous();
  nfield->set_unit(node->unit().get());
  // if (node->aliased()) nfield->setAliased();

  node.replace(nfield);
}

}  // namespace preprocessing
}  // namespace generation
}  // namespace diffingo
