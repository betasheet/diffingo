/*
 * unit_scope_builder.cpp
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

#include "generation/preprocessing/unit_scope_builder.h"

#include <memory>

#include "spec/ast/declaration/type.h"
#include "spec/ast/expression/parser_state.h"
#include "spec/ast/id.h"
#include "spec/ast/module.h"
#include "spec/ast/node.h"
#include "spec/ast/scope.h"
#include "spec/ast/type/atomic_types.h"
#include "spec/ast/type/function.h"
#include "spec/ast/type/reg_exp.h"
#include "spec/ast/type/unit.h"
#include "spec/ast/visitor.h"
#include "util/util.h"

namespace ast = diffingo::spec::ast;
namespace unit = diffingo::spec::ast::type::unit;

namespace diffingo {
namespace generation {
namespace preprocessing {

UnitScopeBuilder::UnitScopeBuilder() {}

UnitScopeBuilder::~UnitScopeBuilder() {}

bool UnitScopeBuilder::run(node_ptr<spec::ast::Module> node,
                           const Options& options) {
  return processAllPreOrder(node);
}

void UnitScopeBuilder::visit(node_ptr<spec::ast::declaration::Type> node) {
  // If this is a unit, populate it's scope.
  auto unit = ast::tryCast<unit::Unit>(node->type());

  if (!unit) return;

  auto uscope = unit->typeScope();
  uscope->insert(ast::newNodePtr(std::make_shared<ast::ID>("self")),
                 ast::newNodePtr(std::make_shared<ast::expression::ParserState>(
                     ast::expression::ParserState::SELF, nullptr, unit, unit)));
  uscope->set_parent(current<ast::Module>()->scope());

  for (auto p : unit->parameters())
    uscope->insert(
        p->id(), ast::newNodePtr(std::make_shared<ast::expression::ParserState>(
                     ast::expression::ParserState::PARAMETER, p->id(), unit,
                     p->type())));

  for (auto i : unit->flattenedItems()) {
    auto iscope = i->scope();

    iscope->set_parent(uscope);
    uscope->addChild(ast::newNodePtr(std::make_shared<ast::ID>(
                         util::fmt("__item_%s", i->id()->name()))),
                     iscope);

    auto dd = i->type();
    if (ast::isA<ast::type::RegExp>(dd)) {
      // TODO(ES) should regexps be string types really?
      dd = node_ptr<ast::type::Type>(std::make_shared<ast::type::Bytes>());
    }

    iscope->insert(
        ast::newNodePtr(std::make_shared<ast::ID>("$$")),
        ast::newNodePtr(std::make_shared<ast::expression::ParserState>(
            ast::expression::ParserState::DOLLARDOLLAR, nullptr, unit, dd)));

    // TODO(ES) support parseUntil => $$ becomes an element of a container/list

    /*
     for (auto h : i->hooks()) {
     h->setUnit(unit);

     if (h->body()) {
     h->body()->scope()->setParent(iscope);
     iscope->addChild(std::make_shared<ID>(util::fmt("__hook_%p", h.get())),
     iscope);
     }

     if (h->foreach ()) {
     // The overrides the item's scope's $$.
     auto dd = ast::type::checkedTrait<type::trait::Container>(i->type())
     ->elementType();
     assert(dd);
     h->body()->scope()->insert(
     std::make_shared<ID>("$$"),
     std::make_shared<expression::ParserState>(
     expression::ParserState::DOLLARDOLLAR, nullptr, unit, dd));
     }
     }
     */
  }
}

}  // namespace preprocessing
}  // namespace generation
}  // namespace diffingo
