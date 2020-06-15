/*
 * type_translator.cpp
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

#include "generation/output/type_translator.h"

#include <pantheios/pantheios.hpp>
#include <string>

#include "spec/ast/id.h"
#include "spec/ast/node.h"
#include "spec/ast/type/atomic_types.h"
#include "spec/ast/type/bitfield.h"
#include "spec/ast/type/bitset.h"
#include "spec/ast/type/container.h"
#include "spec/ast/type/enum.h"
#include "spec/ast/type/function.h"
#include "spec/ast/type/reg_exp.h"
#include "spec/ast/type/type.h"
#include "spec/ast/type/unit.h"
#include "spec/ast/visitor.h"

namespace ast = diffingo::spec::ast;

namespace diffingo {
namespace generation {
namespace output {

using ast::node_ptr;

const std::string TypeTranslator::kDefaultResult = "[[TYPE UNSUPPORTED]]";

TypeTranslator::TypeTranslator() { setDefaultResult(kDefaultResult); }

TypeTranslator::~TypeTranslator() {}

bool TypeTranslator::translate(node_ptr<ast::type::Type> node,
                               std::string* result) {
  // TODO(ES): should wildcard types really not be supported here?
  if (node->wildcard()) {
    log(pantheios::error, node, "trying to translate wildcard type reference");
    return false;
  }

  bool success = processOne(node, result);
  if (*result == kDefaultResult) {
    log(pantheios::error, node, "could not translate type");
    return false;
  }

  return success;
}

void TypeTranslator::visit(node_ptr<ast::type::Bitset> node) {
  // TODO(ES): support Bitset
}

void TypeTranslator::visit(node_ptr<ast::type::Bool> node) {
  setResult("bool");
}

void TypeTranslator::visit(node_ptr<ast::type::Bytes> node) {
  // TODO(ES): support constant-length bytes fields?
  setResult("dr::unit::var_bytes");
}

void TypeTranslator::visit(node_ptr<ast::type::CAddr> node) {
  // TODO(ES): support CAddr
}

void TypeTranslator::visit(node_ptr<ast::type::Double> node) {
  setResult("double");
}

void TypeTranslator::visit(node_ptr<ast::type::Enum> node) {
  if (!node->id()) {
    log(pantheios::error, node, "enum type without id");
    return;
  }

  setResult(node->id()->name());
}

void TypeTranslator::visit(node_ptr<ast::type::Integer> node) {
  std::string r = "";
  if (!node->_signed()) r += "u";
  r += "int";
  r += std::to_string(node->width());
  r += "_t";
  setResult(r);
}

void TypeTranslator::visit(node_ptr<ast::type::List> node) {
  // TODO(ES): support List
}

void TypeTranslator::visit(node_ptr<ast::type::Map> node) {
  // TODO(ES): support Map
}

void TypeTranslator::visit(node_ptr<ast::type::RegExp> node) {
  // TODO(ES): support RegExp
}

void TypeTranslator::visit(node_ptr<ast::type::Set> node) {
  // TODO(ES): support Set
}

void TypeTranslator::visit(node_ptr<ast::type::Sink> node) {
  setResult("SINK");
}

void TypeTranslator::visit(node_ptr<ast::type::String> node) {
  setResult("dr::unit::var_string");
}

void TypeTranslator::visit(node_ptr<ast::type::Tuple> node) {
  // TODO(ES): support Tuple
}

void diffingo::generation::output::TypeTranslator::visit(
    node_ptr<spec::ast::type::TypeType> node) {
  // TODO(ES): support TypeType
}

void TypeTranslator::visit(node_ptr<ast::type::Vector> node) {
  // TODO(ES): support Vector
}

void TypeTranslator::visit(node_ptr<ast::type::Void> node) {
  setResult("void");
}

void TypeTranslator::visit(node_ptr<ast::type::bitfield::Bitfield> node) {
  // TODO(ES): support Bitfield
}

void TypeTranslator::visit(node_ptr<ast::type::unit::Unit> node) {
  if (!node->id()) {
    log(pantheios::error, node, "unit type without id");
    return;
  }

  setResult(node->id()->name());
}

}  // namespace output
}  // namespace generation
}  // namespace diffingo
