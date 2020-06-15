/*
 * instantiation_dependency_resolver.cpp
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

#include "generation/preprocessing/instantiation_dependency_resolver.h"

#include <pantheios/pantheios.hpp>
#include <list>
#include <map>
#include <memory>
#include <stack>
#include <string>

#include "spec/ast/attribute.h"
#include "spec/ast/constant/tuple.h"
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

namespace ast = diffingo::spec::ast;

namespace diffingo {
namespace generation {
namespace preprocessing {

using ast::declaration::unit_instantiation::DependencyContext;
using ast::declaration::unit_instantiation::Dependency;

InstantiationDependencyResolver::InstantiationDependencyResolver() {}

InstantiationDependencyResolver::~InstantiationDependencyResolver() {}

bool InstantiationDependencyResolver::run(node_ptr<spec::ast::Module> node,
                                          const Options& options) {
  return processAllPreOrder(node);
}

void InstantiationDependencyResolver::visit(
    node_ptr<spec::ast::declaration::unit_instantiation::Instantiation> node) {
  // TODO(ES) support nested instantiations?

  DependencyMapBuilder map_builder;
  auto map = map_builder.run(node->unit());

  std::cout << "dep map:" << std::endl;
  for (const auto& entry : *map) {
    for (const auto& to : entry.second) {
      auto id = entry.first;
      std::cout << id.render() << " > " << to.id->render() << " : "
                << std::to_string(to.context) << std::endl;
    }
  }

  std::list<Dependency> dependencies;

  // add a dependency to __length.
  Dependency length_dep;
  length_dep.id = map_builder.length_tracking_id();
  length_dep.context = DependencyContext::CONTEXT_PARSING;
  dependencies.push_back(length_dep);

  // add dependencies to instantiation items
  for (const auto& i : node->items()) {
    Dependency dep;
    dep.id = i->id();
    dep.context = DependencyContext::CONTEXT_APPLICATION;
    dependencies.push_back(dep);
  }

  // iterate to resolve dependencies
  bool changed = true;
  while (changed) {
    changed = false;

    std::list<Dependency> to_add;
    for (const auto& dep : dependencies) {
      for (auto depdep : (*map)[*dep.id]) {
        // TODO(ES): maybe only pass on with minimum of both contexts
        // i.e. don't allow a "parsing" dep to pull in new "serialize" dep?
        if ((dep.context & DependencyContext::CONTEXT_APPLICATION)) {
          // add all kind of dependencies for application dependencies
          to_add.push_back(depdep);
        } else if (dep.context & depdep.context) {
          depdep.context = dep.context & depdep.context;
          to_add.push_back(depdep);
        }
      }
    }

    for (const auto& dep : to_add) {
      bool found = false;
      for (auto& dep_ : dependencies) {
        if (*dep.id == *dep_.id) {
          int old_context = dep_.context;
          dep_.context |= dep.context;
          changed |= dep_.context != old_context;
          found = true;
          break;
        }
      }

      if (!found) {
        dependencies.push_back(dep);
        changed = true;
      }
    }
  }

  // remove dependency to __length.
  dependencies.pop_front();

  std::cout << "dep list:" << std::endl;
  for (const auto& dep : dependencies) {
    std::cout << dep.id->render() << " : " << std::to_string(dep.context)
              << std::endl;
  }

  node->set_dependencies(dependencies);
}

DependencyMapBuilder::DependencyMapBuilder()
    : map_(std::make_shared<DependencyMap>()),
      element_tracking_id_(ast::newNodePtr(std::make_shared<ast::ID>("[]"))),
      length_tracking_id_(
          ast::newNodePtr(std::make_shared<ast::ID>("__length"))) {
  ignore_attributes_.insert("transform");
  ignore_attributes_.insert("transform_to");
}

DependencyMapBuilder::~DependencyMapBuilder() {}

std::shared_ptr<DependencyMap> DependencyMapBuilder::run(
    node_ptr<spec::ast::type::unit::Unit> node) {
  // create dependency map with ids relative to the given unit

  cur_unit_ =
      ast::newNodePtr(std::make_shared<ast::ID>(ast::ID::component_list()));
  processOne(node);
  return map_;
}

void DependencyMapBuilder::visit(node_ptr<spec::ast::type::unit::Unit> node) {
  // TODO(ES): figure out what is required to find end of unit item (?)
  // in case of sized fields: all fields that store dynamic lengths
  // in case of delimited fields: ???

  // TODO(ES) support dependencies for finding end of delimited fields here.

  for (auto i : node->items()) {
    cur_dollar_dollar_ = ast::ID::combineIDs(cur_unit_, i->id());
    from_.push_back(cur_dollar_dollar_);

    processOne(i);

    from_.clear();
  }
}

void DependencyMapBuilder::visit(
    node_ptr<spec::ast::type::unit::item::Item> node) {
  for (auto a : node->attributes()->attributes()) {
    if (ignore_attributes_.find(a->key()) != ignore_attributes_.end()) {
      // skip this attribute
      continue;
    } else if (a->key() == "parse") {
      cur_context_ = DependencyContext::CONTEXT_PARSING;
    } else if (a->key() == "serialize") {
      cur_context_ = DependencyContext::CONTEXT_SERIALIZING;
    } else if (a->key() == "length") {
      from_.push_back(
          length_tracking_id_);  // also track this as a length dependency
      cur_context_ = DependencyContext::CONTEXT_PARSING |
                     DependencyContext::CONTEXT_SERIALIZING_UPDATE;
    } else {
      log(pantheios::warning, node, util::fmt(
                                        "processing unrecognized attribute %s "
                                        "as a PARSING/SERIALIZING one",
                                        a->key()));
      cur_context_ = DependencyContext::CONTEXT_PARSING |
                     DependencyContext::CONTEXT_SERIALIZING;
    }

    if (a->value()) {
      pushTracking();
      processOne(a->value());
      popTracking();
    }

    if (a->key() == "length") {
      from_.pop_back();
    }
  }

  cur_context_ = DependencyContext::CONTEXT_PARSING |
                 DependencyContext::CONTEXT_SERIALIZING;
}

void DependencyMapBuilder::visit(
    node_ptr<spec::ast::type::unit::item::field::Field> node) {
  // TODO(ES): add condition as length dep if condition is set?
  if (node->condition()) {
    pushTracking();
    processOne(node->condition());
    popTracking();
  }
}

void DependencyMapBuilder::visit(
    node_ptr<spec::ast::type::unit::item::field::Unit> node) {
  for (auto p : node->parameters()) {
    pushTracking();
    processOne(p);
    popTracking();
  }

  // TODO(ES) enter into subunit with params set accordingly.
  auto old_unit = cur_unit_;
  cur_unit_ = cur_dollar_dollar_;
  processOne(node->type());
  cur_unit_ = old_unit;
}

void DependencyMapBuilder::visit(
    node_ptr<spec::ast::type::unit::item::field::switch_::Switch> node) {
  // add switch to parse dependency if different lengths for cases
  auto len = node->static_serialized_length();
  if (len < 0) {
    // need to process switch to determine length at runtime
    from_.push_back(length_tracking_id_);

    pushTracking();
    track(cur_dollar_dollar_);
    popTracking();
  }

  if (node->expression()) {
    pushTracking();
    processOne(node->expression());
    popTracking();
  }

  auto switch_id = cur_dollar_dollar_;

  // process cases
  for (auto c : node->cases()) {
    for (auto i : c->items()) {
      from_.clear();
      cur_dollar_dollar_ = ast::ID::combineIDs(cur_unit_, i->id());
      from_.push_back(cur_dollar_dollar_);

      // add dep to switch field
      addDependency(cur_dollar_dollar_, switch_id,
                    DependencyContext::CONTEXT_PARSING |
                        DependencyContext::CONTEXT_SERIALIZING);

      processOne(i);
    }
  }
}

void DependencyMapBuilder::visit(
    node_ptr<spec::ast::type::unit::item::field::container::List> node) {
  // TODO(ES): contained field may need special handling?
  cur_dollar_dollar_ =
      ast::ID::combineIDs(cur_dollar_dollar_, element_tracking_id_);
  from_.push_back(cur_dollar_dollar_);
  processOne(node->contained_field());
}

void DependencyMapBuilder::visit(
    node_ptr<spec::ast::type::unit::item::field::container::Vector> node) {
  cur_context_ = DependencyContext::CONTEXT_PARSING |
                 DependencyContext::CONTEXT_SERIALIZING_UPDATE;

  from_.push_back(
      length_tracking_id_);  // also track this as a length dependency
  pushTracking();
  processOne(node->length());
  popTracking();
  from_.pop_back();

  // TODO(ES): contained field may need special handling?
  cur_context_ = DependencyContext::CONTEXT_PARSING |
                 DependencyContext::CONTEXT_SERIALIZING;
  cur_dollar_dollar_ =
      ast::ID::combineIDs(cur_dollar_dollar_, element_tracking_id_);
  from_.push_back(cur_dollar_dollar_);
  processOne(node->contained_field());
}

void DependencyMapBuilder::visit(
    node_ptr<spec::ast::expression::MemberAttribute> node) {
  if (from_.empty()) return;

  track(node->attribute());
}

void DependencyMapBuilder::visit(
    node_ptr<spec::ast::expression::Variable> node) {
  if (from_.empty()) return;

  // currently variables only occur within lambda expressions
  // => track access to elements of list of find expression
  track(cur_find_list_);
  trackElement();
}

void DependencyMapBuilder::visit(
    node_ptr<spec::ast::expression::Operator> node) {
  if (from_.empty()) return;

  // TODO(ES) support tracking assignment destinations
  // (reverse dependency):
  // e.g. total_len depends on value_len during serializing (update)

  auto ops = node->operands();
  auto it = ops.begin();
  switch (node->kind()) {
    case ast::expression::Operator::Attribute:
      processOne(*it++);
      processOne(*it++);
      break;
    case ast::expression::Operator::AttributeAssign:
      if (cur_context_ & DependencyContext::CONTEXT_SERIALIZING) {
        auto old_context = cur_context_;
        cur_context_ = DependencyContext::CONTEXT_SERIALIZING_UPDATE;

        pushTracking();
        processOne(*it++);
        processOne(*it++);
        popTracking();
        cur_context_ = old_context;
      }
      processOne(*it++);
      break;
    case ast::expression::Operator::Call:
      processOne(*it++);
      processOne(*it++);
      break;
    case ast::expression::Operator::HasAttribute:
      processOne(*it++);
      processOne(*it++);
      break;
    case ast::expression::Operator::Index:
      processOne(*it++);
      trackElement();

      pushTracking();
      processOne(*it++);
      popTracking();
      break;
    case ast::expression::Operator::IndexAssign:
      if (cur_context_ & DependencyContext::CONTEXT_SERIALIZING) {
        auto old_context = cur_context_;
        cur_context_ = DependencyContext::CONTEXT_SERIALIZING_UPDATE;

        pushTracking();
        processOne(*it++);
        trackElement();
        popTracking();
        cur_context_ = old_context;
      }
      it++;
      processOne(*it++);
      break;
    case ast::expression::Operator::MethodCall:
      processOne(*it++);
      it++;
      processOne(*it++);  // tuple, tracking will be pushed inside
      break;
    case ast::expression::Operator::MinusAssign:
      if (cur_context_ & DependencyContext::CONTEXT_SERIALIZING) {
        auto old_context = cur_context_;
        cur_context_ = DependencyContext::CONTEXT_SERIALIZING_UPDATE;

        pushTracking();
        processOne(*it++);
        popTracking();
        cur_context_ = old_context;
      }
      processOne(*it++);
      break;
    case ast::expression::Operator::PlusAssign:
      if (cur_context_ & DependencyContext::CONTEXT_SERIALIZING) {
        auto old_context = cur_context_;
        cur_context_ = DependencyContext::CONTEXT_SERIALIZING_UPDATE;

        pushTracking();
        processOne(*it++);
        popTracking();
        cur_context_ = old_context;
      }
      processOne(*it++);
      break;
    default:
      while (it != ops.end()) {
        pushTracking();
        processOne(*it++);
        popTracking();
      }
      break;
  }
}

void DependencyMapBuilder::visit(
    node_ptr<spec::ast::expression::Constant> node) {
  if (from_.empty()) return;

  auto tuple = ast::tryCast<ast::constant::Tuple>(node);
  if (tuple) {
    for (auto elem : tuple->value()) {
      pushTracking();
      processOne(elem);
      popTracking();
    }
  }
}

void DependencyMapBuilder::visit(node_ptr<spec::ast::expression::Find> node) {
  if (from_.empty()) return;

  // TODO(ES): optimization: track only accesses to elements with certain values
  // i.e. figure out that we only need to parse some specific element

  pushTracking();
  processOne(node->list_expr());
  cur_find_list_ = popTracking();

  pushTracking();
  processOne(node->cond_expr());
  popTracking();

  pushTracking();
  processOne(node->found_expr());
  popTracking();

  pushTracking();
  node->not_found_expr();
  popTracking();
}

void DependencyMapBuilder::visit(
    node_ptr<spec::ast::expression::Conditional> node) {
  if (from_.empty()) return;

  pushTracking();
  processOne(node->cond());
  popTracking();

  pushTracking();
  processOne(node->_true());
  popTracking();

  if (node->_false()) {
    pushTracking();
    processOne(node->_false());
    popTracking();
  }
}

void DependencyMapBuilder::visit(node_ptr<spec::ast::expression::ID> node) {
  if (from_.empty()) return;

  log(pantheios::error, node, "found unexpected id");
}

void DependencyMapBuilder::visit(node_ptr<spec::ast::expression::Lambda> node) {
  if (from_.empty()) return;

  processOne(node->elem_expr());
}

void DependencyMapBuilder::visit(
    node_ptr<spec::ast::expression::ListComprehension> node) {
  if (from_.empty()) return;

  // TODO(ES): support list comprehensions
}

void DependencyMapBuilder::visit(
    node_ptr<spec::ast::expression::ParserState> node) {
  if (from_.empty()) return;

  switch (node->kind()) {
    case ast::expression::ParserState::SELF:
      track(cur_unit_);
      break;
    case ast::expression::ParserState::DOLLARDOLLAR:
      track(cur_dollar_dollar_);
      break;
    case ast::expression::ParserState::PARAMETER:
      // TODO(ES): support units as parameters
      log(pantheios::error, node,
          "TODO: support parameters during dependency resolving");
      break;
    default:
      log(pantheios::error, node,
          "unexpected parser state expression during dependency resolving");
      break;
  }
}

void DependencyMapBuilder::addDependency(node_ptr<spec::ast::ID> from,
                                         node_ptr<spec::ast::ID> to,
                                         int context) {
  // if dependency already exists, add context.
  for (auto& d : (*map_)[*from]) {
    if (*d.id == *to) {
      d.context |= context;
      return;
    }
  }

  // otherwise add new dependency
  Dependency d;
  d.id = to;
  d.context = context;
  (*map_)[*from].push_back(d);
}

void DependencyMapBuilder::track(node_ptr<spec::ast::expression::ID> id) {
  if (!id) return;

  track(id->id());
}

void DependencyMapBuilder::track(node_ptr<spec::ast::ID> id) {
  if (!id) return;

  auto path = id->path();
  if (!path.empty()) tracking_stack_.top()->addPath(path);
}

void DependencyMapBuilder::trackElement() {
  tracking_stack_.top()->addPath(element_tracking_id_->path());
}

void DependencyMapBuilder::pushTracking() {
  tracking_stack_.push(
      ast::newNodePtr(std::make_shared<ast::ID>(ast::ID::component_list())));
}

node_ptr<spec::ast::ID> DependencyMapBuilder::popTracking() {
  auto id = tracking_stack_.top();
  tracking_stack_.pop();
  if (id->path().empty()) return id;

  // record dependency if id is not empty
  for (auto from : from_) {
    addDependency(from, id, cur_context_);
  }
  return id;
}

}  // namespace preprocessing
}  // namespace generation
}  // namespace diffingo
