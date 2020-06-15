/*
 * code_generator.h
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

#ifndef SRC_GENERATION_OUTPUT_CODE_GENERATOR_H_
#define SRC_GENERATION_OUTPUT_CODE_GENERATOR_H_

#include <kode/file.h>
#include <memory>
#include <string>

#include "generation/compiler.h"  // NOLINT
#include "generation/output/translator.h"
#include "generation/parsing/parser_generator.h"
#include "generation/serializing/serializer_generator.h"
#include "spec/ast/declaration/function.h"
#include "spec/ast/declaration/transform.h"
#include "spec/ast/declaration/type.h"
#include "spec/ast/declaration/unit_instantiation.h"
#include "spec/ast/module.h"
#include "spec/ast/node.h"
#include "spec/ast/type/function.h"
#include "spec/ast/type/unit.h"
#include "spec/ast/visitor.h"

namespace diffingo {
namespace generation {
namespace output {

using spec::ast::node_ptr;

class CodeGenerator : public spec::ast::Visitor<> {
 public:
  explicit CodeGenerator(std::string file_name, std::string name_space);
  virtual ~CodeGenerator();

  bool run(node_ptr<spec::ast::Module> node, const Options& options);

  using Visitor::visit;

  void visit(node_ptr<spec::ast::declaration::Function> node) override;
  void visit(node_ptr<spec::ast::declaration::Transform> node) override;
  void visit(node_ptr<spec::ast::declaration::Type> node) override;
  void visit(node_ptr<spec::ast::declaration::unit_instantiation::Instantiation>
                 node) override;

  void visit(node_ptr<spec::ast::type::function::Result> node) override;
  void visit(node_ptr<spec::ast::type::function::Parameter> node) override;

  void visit(
      node_ptr<spec::ast::type::unit::item::field::AtomicType> node) override;
  void visit(node_ptr<spec::ast::type::unit::item::field::Unit> node) override;
  void visit(node_ptr<spec::ast::type::unit::item::field::Ctor> node) override;
  void visit(node_ptr<spec::ast::type::unit::item::field::switch_::Switch> node)
      override;
  void visit(node_ptr<spec::ast::type::unit::item::field::switch_::Case> node)
      override;
  void visit(node_ptr<spec::ast::type::unit::item::field::container::List> node)
      override;
  void visit(node_ptr<spec::ast::type::unit::item::field::container::Vector>
                 node) override;

  void visit(node_ptr<spec::ast::type::unit::item::Variable> node) override;

 private:
  void addSingleUnitField(
      spec::ast::node_ptr<spec::ast::type::unit::item::Item> node);
  void addUnitField(const std::string& name, const std::string& type);

  Translator translator_;
  parsing::ParserGenerator parser_generator_;
  serializing::SerializerGenerator serializer_generator_;

  node_ptr<spec::ast::Module> module_ = nullptr;
  KODE::File file_;
  KODE::Class* unit_cls_ = nullptr;
  KODE::Function* function_ = nullptr;

  const Options* options_ = nullptr;
};

}  // namespace output
}  // namespace generation
}  // namespace diffingo

#endif  // SRC_GENERATION_OUTPUT_CODE_GENERATOR_H_
