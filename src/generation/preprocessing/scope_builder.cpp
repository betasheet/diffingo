/*
 * scope_builder.cpp
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

#include "generation/preprocessing/scope_builder.h"

#include <list>
#include <memory>
#include <string>
#include <utility>

#include "spec/ast/declaration/constant.h"
#include "spec/ast/declaration/declaration.h"
#include "spec/ast/declaration/function.h"
#include "spec/ast/declaration/transform.h"
#include "spec/ast/declaration/type.h"
#include "spec/ast/expression/function.h"
#include "spec/ast/expression/transform.h"
#include "spec/ast/expression/type.h"
#include "spec/ast/function.h"
#include "spec/ast/id.h"
#include "spec/ast/module.h"
#include "spec/ast/node.h"
#include "spec/ast/scope.h"
#include "spec/ast/transform.h"
#include "spec/ast/type/atomic_types.h"
#include "spec/ast/type/bitset.h"
#include "spec/ast/type/enum.h"
#include "spec/ast/type/function.h"
#include "spec/ast/type/type.h"
#include "spec/ast/visitor.h"

namespace ast = diffingo::spec::ast;

namespace diffingo {
namespace generation {
namespace preprocessing {

ScopeBuilder::ScopeBuilder() {}

ScopeBuilder::~ScopeBuilder() {}

bool ScopeBuilder::run(node_ptr<ast::Module> node,
                       const Options& options) {
  addBuiltins(node);
  return processAllPreOrder(node);
  // TODO(ES): support importing ids in modules
}

void ScopeBuilder::visit(node_ptr<ast::declaration::Type> node) {
  auto module = current<ast::Module>();
  auto scope = module->scope();

  auto type = node->type();
  auto expr = node_ptr<ast::expression::Expression>(
      std::make_shared<ast::expression::Type>(type, type->location()));
  scope->insert(node->id(), expr, true);

  // Link in any type-specific scope the type may define.
  // Units are handled in UnitScopeBuilder.
  if (auto bitSet = ast::tryCast<ast::type::Bitset>(type)) {
    auto tscope = bitSet->typeScope();
    tscope->set_parent(scope);
    scope->addChild(node->id(), tscope);
  } else if (auto _enum = ast::tryCast<ast::type::Enum>(type)) {
    auto tscope = _enum->typeScope();
    tscope->set_parent(scope);
    scope->addChild(node->id(), tscope);
  }

  type->set_id(node->id());
  type->set_scope(module->id()->name());
}

void ScopeBuilder::visit(node_ptr<spec::ast::declaration::Constant> node) {
  auto module = current<ast::Module>();
  auto scope = module->scope();

  scope->insert(node->id(), node->value(), true);
}

void ScopeBuilder::visit(node_ptr<spec::ast::declaration::Function> node) {
  auto module = current<ast::Module>();
  auto scope = module->scope();

  auto func = node->function();
  auto expr = node_ptr<ast::expression::Expression>(
      std::make_shared<ast::expression::Function>(func, func->location()));

  scope->insert(node->id(), expr, true);
}

void diffingo::generation::preprocessing::ScopeBuilder::visit(
    node_ptr<spec::ast::declaration::Transform> node) {
  auto module = current<ast::Module>();
  auto scope = module->scope();

  auto transform = node->transform();
  auto expr = node_ptr<ast::expression::Expression>(
      std::make_shared<ast::expression::Transform>(transform,
                                                   transform->location()));

  scope->insert(node->id(), expr, true);
}

void ScopeBuilder::addBuiltins(node_ptr<spec::ast::Module> module) {
  // ByteOrder type
  ast::type::Enum::label_list bo_labels;
  auto bo_lb_big = ast::newNodePtr(std::make_shared<ast::ID>("big"));
  auto bo_lb_little = ast::newNodePtr(std::make_shared<ast::ID>("little"));
  bo_labels.push_back(std::make_pair(bo_lb_big, 0));
  bo_labels.push_back(std::make_pair(bo_lb_little, 1));
  auto bo_enum = ast::newNodePtr(std::make_shared<ast::type::Enum>(bo_labels));
  auto bo_id = ast::newNodePtr(std::make_shared<ast::ID>("ByteOrder"));
  auto bo_decl = ast::newNodePtr(std::make_shared<ast::declaration::Type>(
      bo_id, ast::declaration::Declaration::Linkage::IMPORTED, bo_enum));
  module->addDeclaration(bo_decl);

  // uppercase function
  auto uc_id = ast::newNodePtr(std::make_shared<ast::ID>("uppercase"));
  auto uc_res_type = ast::newNodePtr(std::make_shared<ast::type::String>());
  auto uc_res = ast::newNodePtr(
      std::make_shared<ast::type::function::Result>(uc_res_type, false));
  auto uc_params = ast::parameter_list();
  auto uc_param_id = ast::newNodePtr(std::make_shared<ast::ID>("arg"));
  auto uc_param_type = ast::newNodePtr(std::make_shared<ast::type::String>());
  uc_params.push_back(
      ast::newNodePtr(std::make_shared<ast::type::function::Parameter>(
          uc_param_id, uc_param_type, false, nullptr)));
  auto uc_type =
      ast::newNodePtr(std::make_shared<ast::type::function::Function>());
  auto uc_func = ast::newNodePtr(
      std::make_shared<ast::Function>(uc_id, uc_type, module, ""));
  auto uc_decl = ast::newNodePtr(std::make_shared<ast::declaration::Function>(
      uc_func, ast::declaration::Declaration::Linkage::IMPORTED));
  module->addDeclaration(uc_decl);

  // TODO(ES): transforms
  // stringEncodedUint64 transform
  auto seui64_id =
      ast::newNodePtr(std::make_shared<ast::ID>("stringEncodedUint64"));
  auto seui64_wire_type =
      ast::newNodePtr(std::make_shared<ast::type::String>());
  auto seui64_internal_type =
      ast::newNodePtr(std::make_shared<ast::type::Integer>(64, false));
  auto seui64_transform = ast::newNodePtr(std::make_shared<ast::Transform>(
      seui64_id, seui64_wire_type, seui64_internal_type, nullptr, nullptr));
  auto seui64_decl =
      ast::newNodePtr(std::make_shared<ast::declaration::Transform>(
          seui64_transform, ast::declaration::Declaration::Linkage::IMPORTED));
  module->addDeclaration(seui64_decl);

  // hexStringEncodedUint64 transform
  auto hseui64_id =
      ast::newNodePtr(std::make_shared<ast::ID>("hexStringEncodedUint64"));
  auto hseui64_wire_type =
      ast::newNodePtr(std::make_shared<ast::type::String>());
  auto hseui64_internal_type =
      ast::newNodePtr(std::make_shared<ast::type::Integer>(64, false));
  auto hseui64_transform = ast::newNodePtr(std::make_shared<ast::Transform>(
      hseui64_id, hseui64_wire_type, hseui64_internal_type, nullptr, nullptr));
  auto hseui64_decl =
      ast::newNodePtr(std::make_shared<ast::declaration::Transform>(
          hseui64_transform, ast::declaration::Declaration::Linkage::IMPORTED));
  module->addDeclaration(hseui64_decl);

  // stringEncodedUint32 transform
  auto seui32_id =
      ast::newNodePtr(std::make_shared<ast::ID>("stringEncodedUint32"));
  auto seui32_wire_type =
      ast::newNodePtr(std::make_shared<ast::type::String>());
  auto seui32_internal_type =
      ast::newNodePtr(std::make_shared<ast::type::Integer>(32, false));
  auto seui32_transform = ast::newNodePtr(std::make_shared<ast::Transform>(
      seui32_id, seui32_wire_type, seui32_internal_type, nullptr, nullptr));
  auto seui32_decl =
      ast::newNodePtr(std::make_shared<ast::declaration::Transform>(
          seui32_transform, ast::declaration::Declaration::Linkage::IMPORTED));
  module->addDeclaration(seui32_decl);
}

}  // namespace preprocessing
}  // namespace generation
}  // namespace diffingo
