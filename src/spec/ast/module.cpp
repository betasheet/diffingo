/*
 * module.cpp
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

#include "spec/ast/module.h"

#include <list>
#include <memory>
#include <string>

#include "spec/ast/attribute.h"
#include "spec/ast/declaration/declaration.h"
#include "spec/ast/declaration/type.h"
#include "spec/ast/id.h"
#include "spec/ast/node.h"
#include "spec/ast/scope.h"
#include "spec/ast/type/type.h"
#include "spec/ast/type/unit.h"

namespace diffingo {
namespace spec {
namespace ast {

Module::Module(generation::CompilerContext* ctx, node_ptr<ID> id,
               const std::string& path, const Location& l)
    : Node(l),
      id_(id),
      context_(ctx),
      path_(path),
      properties_(std::make_shared<AttributeMap>()),
      scope_(std::make_shared<Scope>()) {
  addChild(id_);
  addChild(properties_);
}

Module::~Module() {}

void Module::addProperty(node_ptr<Attribute> prop) { properties_->add(prop); }

node_ptr<Attribute> Module::property(const std::string& prop) const {
  return properties_->lookup(prop);
}

void Module::addDeclaration(node_ptr<declaration::Declaration> decl) {
  // TODO(ES): enable adding comment node
  // addComment(decl);

  decls_.push_back(decl);
  addChild(decls_.back());

  if (auto t_decl = tryCast<declaration::Type>(decl)) {
    if (auto unit = tryCast<type::unit::Unit>(t_decl->type())) {
      unit->set_module(this);
    }
  }
}

void Module::addDeclarations(declaration_list decls) {
  for (auto d : decls) {
    addDeclaration(d);
  }
}

declaration_list Module::declarations() const {
  declaration_list decls;
  for (auto s : decls_) {
    decls.push_back(s);
  }

  return decls;
}

void Module::exportType(node_ptr<type::Type> type) {
  // TODO(Eric Seckler): Support exporting
}

void Module::import(node_ptr<ID> id) {
  // TODO(Eric Seckler): Support importing
}

std::string Module::render() { return id_->render() + " (" + path_ + ")"; }

}  // namespace ast
}  // namespace spec
}  // namespace diffingo
