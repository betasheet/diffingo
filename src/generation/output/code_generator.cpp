/*
 * code_generator.cpp
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

#include "generation/output/code_generator.h"

#include <kode/class.h>
#include <kode/code.h>
#include <kode/enum.h>
#include <kode/function.h>
#include <kode/membervariable.h>
#include <kode/printer.h>
#include <pantheios/pantheios.hpp>
#include <list>
#include <memory>
#include <string>

#include "generation/output/translator.h"
#include "spec/ast/declaration/declaration.h"
#include "spec/ast/declaration/function.h"
#include "spec/ast/declaration/transform.h"
#include "spec/ast/declaration/type.h"
#include "spec/ast/declaration/unit_instantiation.h"
#include "spec/ast/expression/expression.h"
#include "spec/ast/function.h"
#include "spec/ast/id.h"
#include "spec/ast/module.h"
#include "spec/ast/node.h"
#include "spec/ast/type/enum.h"
#include "spec/ast/type/function.h"
#include "spec/ast/type/unit.h"
#include "spec/ast/visitor.h"

namespace ast = diffingo::spec::ast;
namespace unit = diffingo::spec::ast::type::unit;
namespace function = diffingo::spec::ast::type::function;

namespace diffingo {
namespace generation {
namespace output {

CodeGenerator::CodeGenerator(std::string file_name, std::string name_space) {
  file_.setFilename(file_name);
  file_.headerCode()->addLine("namespace dr = diffingo::runtime;");
  file_.setNameSpace(name_space);
}

CodeGenerator::~CodeGenerator() {}

bool CodeGenerator::run(node_ptr<ast::Module> node, const Options& options) {
  module_ = node;
  options_ = &options;

  // using custom traversal so that "type" children of fields aren't visited
  bool result = true;
  for (auto c : node->children(false)) {
    if (options.instantiation_only) {
      if (auto decl = ast::tryCast<ast::declaration::Type>(c)) {
        if (ast::tryCast<ast::type::unit::Unit>(decl->type())) {
          continue;
        }
      }
    }
    result &= processOne(c);
  }
  if (!result) return false;

  KODE::Printer printer;
  printer.printHeader(file_);
  printer.printImplementation(file_);
  return true;
}

void CodeGenerator::visit(node_ptr<spec::ast::declaration::Function> node) {
  // TODO(ES): for now, we don't process imported declarations (built-ins)
  if (node->linkage() == ast::declaration::Declaration::Linkage::IMPORTED)
    return;

  KODE::Function func;
  func.setName(translator_.functionName(node->id()->name()));
  func.setBody(node->function()->body());
  function_ = file_.addFileFunction(func);

  for (auto c : node->children(false)) {
    processOne(c);
  }
}

void CodeGenerator::visit(node_ptr<spec::ast::declaration::Transform> node) {
  // TODO(ES): for now, we don't process imported declarations (built-ins)
  if (node->linkage() == ast::declaration::Declaration::Linkage::IMPORTED)
    return;

  // TODO(ES): support transform declaration code generation

  for (auto c : node->children(false)) {
    processOne(c);
  }
}

void CodeGenerator::visit(node_ptr<spec::ast::declaration::Type> decl) {
  // TODO(ES): for now, we don't process imported declarations (built-ins)
  if (decl->linkage() == ast::declaration::Declaration::Linkage::IMPORTED)
    return;

  node_ptr<ast::type::Type> type = decl->type();

  if (auto unit = ast::tryCast<unit::Unit>(type)) {
    KODE::Class cls(translator_.unitName(decl->id()->name()));
    unit_cls_ = file_.insertClass(cls);
    unit_cls_->addHeaderInclude("runtime/runtime.h");
    unit_cls_->addHeaderInclude("stddef.h");
    unit_cls_->addHeaderInclude("cstdint");

    // generate and add parser class
    KODE::Class parser(translator_.unitParserName(decl->id()->name()));
    auto ptr_parser = file_.insertClass(parser);
    if (!parser_generator_.run(unit, ptr_parser, *options_)) {
      log(pantheios::error, unit, "error during parser generation");
    }

    // generate and add serializer class
    KODE::Class serializer(translator_.unitSerializerName(decl->id()->name()));
    auto ptr_serializer = file_.insertClass(serializer);
    if (!serializer_generator_.run(unit, ptr_serializer, *options_)) {
      log(pantheios::error, unit, "error during parser generation");
    }
  } else if (auto enum_t = ast::tryCast<ast::type::Enum>(type)) {
    std::list<KODE::Enum::label_with_value> labels;
    for (const auto& l : enum_t->labels()) {
      KODE::Enum::label_with_value lv(translator_.enumLabel(l.first->name()),
                                      true, l.second);
      labels.push_back(lv);
    }
    KODE::Enum _enum(translator_.enumName(decl->id()->name()), labels, true);
    file_.addFileEnum(_enum);
  } else {
    // TODO(ES) support other declarations
  }

  for (auto c : type->children(false)) {
    processOne(c);
  }
}

void CodeGenerator::visit(
    node_ptr<spec::ast::declaration::unit_instantiation::Instantiation> decl) {
  // TODO(ES): for now, we don't process imported declarations (built-ins)
  if (decl->linkage() == ast::declaration::Declaration::Linkage::IMPORTED)
    return;

  for (auto c : decl->compacted_units()) {
    processOne(c);
  }
}

void CodeGenerator::visit(node_ptr<function::Result> node) {
  function_->setReturnType(translator_.type(node->type()));
}

void CodeGenerator::visit(node_ptr<function::Parameter> node) {
  // TODO(ES): support parameters for units
  if (!function_) return;

  std::string arg = translator_.type(node->type()) + " " +
                    translator_.functionParamName(node->id()->name());
  if (node->default_value()) {
    function_->addArgument(arg, translator_.expression(node->default_value()));
  } else {
    function_->addArgument(arg);
  }
}

void CodeGenerator::visit(node_ptr<unit::item::field::AtomicType> node) {
  addSingleUnitField(node);
}

void CodeGenerator::visit(node_ptr<unit::item::field::Unit> node) {
  // units are embedded if they're a single field.
  addSingleUnitField(node);
}

void CodeGenerator::visit(node_ptr<unit::item::field::Ctor> node) {
  // TODO(ES): support ctor fields correctly
  addSingleUnitField(node);
}

void diffingo::generation::output::CodeGenerator::visit(
    node_ptr<spec::ast::type::unit::item::field::switch_::Switch> node) {
  // TODO(ES): support switch as union field

  for (auto c : node->children(false)) {
    processOne(c);
  }
}

void diffingo::generation::output::CodeGenerator::visit(
    node_ptr<spec::ast::type::unit::item::field::switch_::Case> node) {
  // TODO(ES): support cases in switch as union fields

  for (auto c : node->children(false)) {
    processOne(c);
  }
}

void CodeGenerator::visit(node_ptr<unit::item::field::container::List> node) {
  // TODO(ES): support list fields (always variable size list)
  addSingleUnitField(node);
}

void CodeGenerator::visit(node_ptr<unit::item::field::container::Vector> node) {
  // TODO(ES): support vector fields (may be variable or fixed size)
  // (initially we should probably handle all as variable size here)
  addSingleUnitField(node);
}

void CodeGenerator::visit(node_ptr<unit::item::Variable> node) {
  // TODO(ES): support variables that aren't atomic types
  addSingleUnitField(node);
}

void CodeGenerator::addSingleUnitField(node_ptr<unit::item::Item> node) {
  if (node->parsing_only() && !options_->store_parsing_only) {
    // item doesn't need to be stored in parsed unit
    // TODO(ES): still need to add a stream range field of correct size instead
    // (combine with previous/succeeding if available)
    return;
  }

  std::string name = translator_.unitFieldName(node->id()->name());
  std::string type = translator_.type(node->type());

  auto field = ast::tryCast<unit::item::field::Field>(node);
  if (field && !field->application_accessible() && options_->input_pointers) {
    // TODO(ES): figure out if field size is static constant => only use start
    // pointer. also figure out if start pointer is constant respective to other
    // field start pointer => only use length (or neither, if both are static.
    // then, nothing needs to be stored)
    type = "dr::unit::var_stream_range";
  }

  addUnitField(name, type);
}

void CodeGenerator::addUnitField(const std::string& name,
                                 const std::string& type) {
  KODE::MemberVariable v(name, type, false, KODE::MemberVariable::Public);
  unit_cls_->addMemberVariable(v);
}

}  // namespace output
}  // namespace generation
}  // namespace diffingo
