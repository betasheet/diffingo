/*
 * module.h
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

#ifndef SRC_SPEC_AST_MODULE_H_
#define SRC_SPEC_AST_MODULE_H_

#include <list>
#include <memory>
#include <string>

#include "generation/compiler_context.h"
#include "spec/ast/attribute.h"
#include "spec/ast/declaration/declaration.h"
#include "spec/ast/id.h"
#include "spec/ast/location.h"
#include "spec/ast/node.h"
#include "spec/ast/scope.h"
#include "spec/ast/type/type.h"
#include "spec/ast/visitor.h"

namespace diffingo {
namespace spec {
namespace ast {

class Module : public Node {
 public:
  Module(generation::CompilerContext* ctx, node_ptr<ID> id,
         const std::string& path = "-", const Location& l = Location::None);
  virtual ~Module();

  node_ptr<ID> id() const { return id_; }

  generation::CompilerContext* context() const { return context_; }

  const std::string& path() const { return path_; }

  void addProperty(node_ptr<Attribute> prop);
  node_ptr<AttributeMap> properties() const { return properties_; }
  node_ptr<Attribute> property(const std::string& prop) const;

  void addDeclaration(node_ptr<declaration::Declaration> decl);
  void addDeclarations(declaration_list decls);
  declaration_list declarations() const;

  std::shared_ptr<Scope> scope() const { return scope_; }

  void exportType(node_ptr<type::Type> type);

  void import(node_ptr<ID> id);

  std::string render() override;

  ACCEPT_VISITOR(Node)

 private:
  node_ptr<ID> id_;
  generation::CompilerContext* context_;
  std::string path_;
  node_ptr<AttributeMap> properties_;
  std::list<node_ptr<declaration::Declaration>> decls_;

  std::shared_ptr<Scope> scope_;
};

}  // namespace ast
}  // namespace spec
}  // namespace diffingo

#endif  // SRC_SPEC_AST_MODULE_H_
