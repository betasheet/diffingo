/*
 * instantiation_type_generator.cpp
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

#include "generation/preprocessing/instantiation_type_generator.h"

#include <pantheios/pantheios.hpp>
#include <list>
#include <memory>

#include "spec/ast/attribute.h"
#include "spec/ast/declaration/declaration.h"
#include "spec/ast/declaration/type.h"
#include "spec/ast/declaration/unit_instantiation.h"
#include "spec/ast/expression/operator.h"
#include "spec/ast/expression/parser_state.h"
#include "spec/ast/id.h"
#include "spec/ast/module.h"
#include "spec/ast/node.h"
#include "spec/ast/type/atomic_types.h"
#include "spec/ast/type/type.h"
#include "spec/ast/type/unit.h"
#include "spec/ast/visitor.h"
#include "util/util.h"

namespace ast = diffingo::spec::ast;

namespace diffingo {
namespace generation {
namespace preprocessing {

using ast::declaration::unit_instantiation::DependencyContext;
using ast::declaration::unit_instantiation::Dependency;

SelfReplacerVisitor::SelfReplacerVisitor() {
  // enableDebugging(true);
}

SelfReplacerVisitor::~SelfReplacerVisitor() {}

bool SelfReplacerVisitor::run(node_ptr<spec::ast::Node> node,
                              node_ptr<spec::ast::type::unit::Unit> unit) {
  unit_ = unit;
  return processOne(node);
}

void SelfReplacerVisitor::visit(node_ptr<spec::ast::type::unit::Unit> node) {
  for (auto c : node->children(false)) {
    processOne(c);
  }
}

void SelfReplacerVisitor::visit(
    node_ptr<spec::ast::type::unit::item::Item> node) {
  for (auto c : node->children(false)) {
    if (ast::tryCast<ast::expression::Expression>(c)) {
      processOne(c);
    } else if (auto m = ast::tryCast<ast::AttributeMap>(c)) {
      for (auto a : m->attributes()) {
        processOne(a->value());
      }
    } else if (auto a = ast::tryCast<ast::Attribute>(c)) {
      processOne(a->value());
    }
  }
}

void SelfReplacerVisitor::visit(
    node_ptr<spec::ast::expression::Expression> node) {
  for (auto c : node->children(false)) {
    if (ast::tryCast<ast::expression::Expression>(c)) {
      processOne(c);
    }
  }
}

void SelfReplacerVisitor::visit(
    node_ptr<spec::ast::expression::ParserState> node) {
  // TODO(ES): also replace params and dollar-dollars
  // maybe use an additional unit scope building + id resolving pass?
  node->set_unit(unit_);
  if (node->kind() == spec::ast::expression::ParserState::SELF) {
    node->set_type(unit_);
  } else if (node->kind() == spec::ast::expression::ParserState::DOLLARDOLLAR) {
  } else if (node->kind() == spec::ast::expression::ParserState::PARAMETER) {
  }
}

InstantiationTypeGenerator::InstantiationTypeGenerator() {}

InstantiationTypeGenerator::~InstantiationTypeGenerator() {}

bool InstantiationTypeGenerator::run(node_ptr<spec::ast::Module> node,
                                     const Options& options) {
  for (auto d : node->declarations()) {
    processOne(d);
  }
  return !errors();
}

void InstantiationTypeGenerator::visit(
    node_ptr<spec::ast::declaration::unit_instantiation::Instantiation> node) {
  unit_decls_.clear();
  deps_ = node->dependencies();

  cur_unit_name_ = node->id();
  cur_unit_id_ =
      ast::newNodePtr(std::make_shared<ast::ID>(ast::ID::component_list()));

  processOne(node->unit());

  node->set_compacted_units(unit_decls_);
}

void InstantiationTypeGenerator::visit(
    node_ptr<spec::ast::type::unit::Unit> node) {
  cur_unit_items_.clear();

  for (auto i : node->items()) {
    processOne(i);
  }
  compactAndAddItems();

  auto compact_unit = ast::newNodePtr(std::make_shared<ast::type::unit::Unit>(
      node->parameters(), cur_unit_items_));
  compact_unit->set_id(cur_unit_name_);
  compact_unit->set_scope(node->scope());
  auto decl = ast::newNodePtr(std::make_shared<ast::declaration::Type>(
      cur_unit_name_, ast::declaration::Type::Linkage::EXPORTED, compact_unit));

  // TODO(ES): make sure this replaces types correctly in case of embedded units
  replacer_.run(compact_unit, compact_unit);

  unit_decls_.push_back(decl);
}

void InstantiationTypeGenerator::visit(
    node_ptr<spec::ast::type::unit::item::Item> node) {
  // TODO(ES): support embedded units, switch fields and containers
  auto path = ast::ID::combineIDs(cur_unit_id_, node->id());
  int context = dependencyContextFor(path);

  if (context) {
    if (!ast::tryCast<ast::type::unit::item::Variable>(node)) {
      // add compactable fields before this field
      compactAndAddItems();
    }

    auto item = copyItem(node);

    // TODO(ES): compaction across parsing-only fields/variables
    // (this may need to happen later during generation of parser?)
    if (context == DependencyContext::CONTEXT_PARSING) {
      // mark that item is only required during parsing
      item->set_parsing_only();
    }

    cur_unit_items_.push_back(item);
  } else {
    if (auto f = ast::tryCast<ast::type::unit::item::field::Field>(node)) {
      // no dependency to this field - see if we can compact it
      cur_compactable_fields_.push_back(f);
    } else if (ast::tryCast<ast::type::unit::item::Variable>(node)) {
      // nothing to do: variables that aren't required won't be computed
    } else if (ast::tryCast<ast::type::unit::item::Property>(node)) {
      // just add the property
      cur_unit_items_.push_back(copyItem(node));
    }
  }
}

void InstantiationTypeGenerator::compactAndAddItems() {
  node_ptr<ast::expression::Expression> len = nullptr;
  for (auto f : cur_compactable_fields_) {
    // TODO(ES): support embedded units, switch fields and containers
    auto f_len = f->serialized_length();
    if (f_len) {
      if (len) {
        // TODO(ES): what if cond / attrs / params / sinks are set on field?
        ast::expression_list ops;
        ops.push_back(len);
        ops.push_back(f_len);
        len = ast::newNodePtr(std::make_shared<ast::expression::Operator>(
            ast::expression::Operator::Kind::Plus, ops));
      } else {
        len = f_len;
      }
    } else {
      // can't determine length of this field (e.g. list or switch)
      if (len) {
        addFieldOfLength(len);
        len = nullptr;
      }
      cur_unit_items_.push_back(copyItem(f));
    }
  }

  if (len) {
    addFieldOfLength(len);
  }

  cur_compactable_fields_.clear();
}

int InstantiationTypeGenerator::dependencyContextFor(
    node_ptr<spec::ast::ID> id) {
  int context = 0;
  for (auto dep : deps_) {
    if (dep.id->prefixOf(*id)) {
      context |= dep.context;
    }
  }
  return context;
}

node_ptr<spec::ast::type::unit::item::Item>
InstantiationTypeGenerator::copyItem(
    node_ptr<spec::ast::type::unit::item::Item> item) {
  // TODO(ES): cloning switches and containers
  return item->clone();
}

void InstantiationTypeGenerator::addFieldOfLength(
    node_ptr<spec::ast::expression::Expression> length) {
  log(pantheios::informational, length,
      util::fmt("adding field of length %s", length->render()));

  // TODO(ES): copy contents rather than create a non-app bytes field if length
  // is very small?
  auto type = ast::newNodePtr(std::make_shared<ast::type::Bytes>());
  auto field = ast::newNodePtr(
      std::make_shared<ast::type::unit::item::field::AtomicType>(nullptr,
                                                                 type));

  // mark field as internal only ->
  // store only range ref (to support serialization) or discard contents
  field->set_application_accessible(false);

  auto len_attr = ast::newNodePtr(
      std::make_shared<ast::Attribute>("length", length->clone()));
  field->attributes()->add(len_attr);

  cur_unit_items_.push_back(field);

  // todos before/during generation of parser:

  // TODO(ES): constant propagation / expression elimination of length
  // expression.
  // TODO(ES): if length is a static constant, record it as a constant length
  // range ref field (in this case, no length needs to be stored in unit struct
  // field)

  // TODO(ES): optimally, also determine if length of fields added between this
  // and last range field is static constant - then we don't even need to store
  // a pointer to beginning of this range ref field, but calculate it
  // statically.
}

}  // namespace preprocessing
}  // namespace generation
}  // namespace diffingo
