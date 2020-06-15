/*
 * parser_generator.cpp
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

#include "generation/parsing/parser_generator.h"

#include <kode/class.h>
#include <kode/code.h>
#include <kode/function.h>
#include <kode/membervariable.h>
#include <pantheios/pantheios.hpp>
#include <iostream>
#include <list>
#include <string>
#include <utility>

#include "generation/compiler.h"
#include "spec/ast/attribute.h"
#include "spec/ast/constant/constant.h"
#include "spec/ast/constant/enum.h"
#include "spec/ast/expression/constant.h"
#include "spec/ast/id.h"
#include "spec/ast/node.h"
#include "spec/ast/type/atomic_types.h"
#include "spec/ast/type/bitfield.h"
#include "spec/ast/type/bitset.h"
#include "spec/ast/type/container.h"
#include "spec/ast/type/enum.h"
#include "spec/ast/type/type.h"
#include "spec/ast/type/unit.h"
#include "spec/ast/visitor.h"
#include "util/util.h"

namespace ast = diffingo::spec::ast;

namespace diffingo {
namespace generation {
namespace parsing {

ParserGenerator::ParserGenerator() {}

ParserGenerator::~ParserGenerator() {}

bool ParserGenerator::run(node_ptr<ast::type::unit::Unit> node,
                          KODE::Class* parser_cls, const Options& options) {
  cls_ = parser_cls;
  root_instr_ = newInstructionLabel("root");
  unit_ = node;
  options_ = &options;

  // -- create code blocks that will be filled during visiting unit elements --
  KODE::Code init_consts_body;
  init_consts_code_ = &init_consts_body;
  KODE::Code parse_body_inner;
  code_ = &parse_body_inner;

  // parse sequence of items and fill body of parse method
  for (auto item : node->items()) {
    if (!ast::tryCast<ast::type::unit::item::Property>(item)) {
      parse(item);
    }
  }

  // -- add code to parser class --

  KODE::Code parse_body;
  code_ = &parse_body;

  // add temp var declarations
  code_->addLine("char* parse_dest;");
  code_->addLine("dr::parsing::ParseResult parse_res;");
  code_->addLine("char* unit;");
  for (auto tmp_var : temp_vars_) {
    code_->addLine(util::fmt("%s %s;", tmp_var.second, tmp_var.first));
  }
  code_->newLine();

  // root instruction
  code_->addLine(root_instr_ + ":");
  code_->addLine("if (state->instruction()) {");
  code_->addLine("  unit = BLOCKSTATE->unit;");
  code_->addLine("  goto *state->instruction();");
  code_->addLine("}");
  code_->newLine();

  // push new block state, allocate space for unit in area and set pointer
  // within state to point to it
  emitPushBlockState();
  emitAllocateIntoPointerPointer(exprCurrentUnitPP());
  code_->addLine("unit = BLOCKSTATE->unit;");

  // init stream position
  code_->addLine("*POS = in_buf_start;");
  code_->newLine();

  code_->addBlock(parse_body_inner);

  // update bytes_read and return
  code_->addLine("*bytes_read = *POS - in_buf_start;");
  code_->addLine("return dr::parsing::ParseResult::DONE;");

  // define macros
  cls_->addDeclarationMacro("#define POS state->stream_pos()");
  cls_->addDeclarationMacro("#define BLOCKSTATE state->peek<BlockState>()");
  cls_->addDeclarationMacro(
      "#define UNIT(type) reinterpret_cast<type*>(unit)");
  cls_->addDeclarationMacro(
      "#define UNITPP(type) reinterpret_cast<type**>(&BLOCKSTATE->unit)");
  cls_->addDeclarationMacro("#define SELF(type) (*UNIT(type))");

  // create parser constants struct
  KODE::Class consts("ParserConstants");
  for (auto m : consts_) {
    consts.addMemberVariable(m);
  }
  parser_cls->addNestedClass(consts);
  KODE::MemberVariable consts_var("consts_", "ParserConstants", false, true);
  parser_cls->addMemberVariable(consts_var);
  KODE::Function init_consts("initParserConstants", "void",
                             KODE::Function::Private, false);
  init_consts.setBody(init_consts_body);
  parser_cls->addFunction(init_consts);

  // TODO(ES): create block state entry struct(s)
  KODE::Class block_state("BlockState");
  KODE::MemberVariable bs_unit("unit", "char*", false, true);
  block_state.addMemberVariable(bs_unit);
  parser_cls->addNestedClass(block_state);

  // generate and add parse function
  KODE::Function parse_func("parse", "dr::parsing::ParseResult");
  parse_func.addArgument("char* in_buf_start");
  parse_func.addArgument("char* in_buf_end");
  parse_func.addArgument("dr::unit::UnitArea* area");
  parse_func.addArgument("dr::parsing::ParserState* state");
  parse_func.addArgument("size_t* bytes_read");
  parse_func.setBody(parse_body);
  parser_cls->addFunction(parse_func);

  // TODO(ES): add a reset function

  return !errors();
}

void ParserGenerator::visit(
    node_ptr<ast::type::unit::item::field::AtomicType> node) {
  if (!node->application_accessible() && options_->input_pointers) {
    // parse without storing
    use_input_pointer_ = true;
  }

  if (node->serialized_type() != node->type()) {
    // TODO(ES): support var_bytes/string parsing here
    // (where to allocate temp bytes/string storage?)

    // need transform between serialized and internal types.
    // parse into temporary variable first
    auto temp_name = addTemp(translator_.type(node->serialized_type()));
    code_->addLine(util::fmt("parse_dest = (char*) &%s;", temp_name));

    // parse value into location
    processOne(node->serialized_type());

    if (node->attributes()->has("transform_to")) {
      // TODO(ES): we assume it's possible to cast here for now.
      code_->addLine(util::fmt("%s->%s = (%s) %s;", exprCurrentUnit(),
                               translator_.unitFieldName(node->id()->name()),
                               translator_.type(node->type()), temp_name));
    } else if (node->attributes()->has("transform")) {
      // TODO(ES): support custom transforms
    } else {
      log(pantheios::error, node,
          "unknown case for serialized type != internal type");
    }
  } else {
    // no transform required - parse directly into field.
    // set up pointer into field.
    code_->addLine(util::fmt("parse_dest = (char*) &%s->%s;", exprCurrentUnit(),
                             translator_.unitFieldName(node->id()->name())));

    // parse value into location
    processOne(node->serialized_type());
  }

  // reset input pointer switch
  use_input_pointer_ = false;
}

void ParserGenerator::visit(
    node_ptr<ast::type::unit::item::field::Constant> node) {
  // TODO(ES)
  // statically convert constant to byte array in serialized format
  // add constant field to unit's "ParserConstants" struct
  // check for length
  // check for equality to constant
}

void ParserGenerator::visit(node_ptr<ast::type::unit::item::field::Unit> node) {
  // TODO(ES): recurse into embedded unit
}

void ParserGenerator::visit(node_ptr<ast::type::unit::item::field::Ctor> node) {
  // TODO(ES): parse ctor - regexp / bytes
}

void ParserGenerator::visit(
    node_ptr<ast::type::unit::item::field::Unknown> node) {
  this->log(pantheios::error, node, "encountered field of unknown type");
}

void ParserGenerator::visit(
    node_ptr<ast::type::unit::item::field::container::List> node) {
  // TODO(ES): support lists
}

void ParserGenerator::visit(
    node_ptr<ast::type::unit::item::field::container::Vector> node) {
  // TODO(ES): support vectors
}

void ParserGenerator::visit(
    node_ptr<spec::ast::type::unit::item::field::switch_::Switch> node) {
  // TODO(ES): support switch as union struct - also store which switch path /
  // fields are used in the unit
  if (node->expression()) {
    code_->addLine(
        util::fmt("switch (%s){", translator_.expression(node->expression())));
    for (auto c : node->cases()) {
      for (auto e : c->expressions()) {
        code_->addLine(util::fmt("case %s:", translator_.expression(e)));
      }
      if (c->_default()) {
        code_->addLine("default:");
      }
      code_->indent();
      for (auto x : c->items()) {
        parse(x);
      }
      code_->addLine("break;");
      code_->unindent();
    }
    code_->addLine("}");
  } else {
    // TODO(ES): support look-ahead switch
  }
}

void ParserGenerator::visit(node_ptr<ast::type::unit::item::Variable> node) {
  if (node->attributes()->has("parse")) {
    // evaluate value expression and store into variable field
    auto pattr = node->attributes()->lookup("parse");
    code_->addLine(util::fmt("%s->%s = %s;", exprCurrentUnit(),
                             translator_.unitFieldName(node->id()->name()),
                             translator_.expression(pattr->value())));
    // TODO(ES): support strings, bytes constructors, ..
  } else if (node->default_value()) {
    // evaluate value expression and store into variable field
    code_->addLine(util::fmt("%s->%s = %s;", exprCurrentUnit(),
                             translator_.unitFieldName(node->id()->name()),
                             translator_.expression(node->default_value())));
    // TODO(ES): support strings, bytes constructors, ..
  }
}

void ParserGenerator::visit(node_ptr<spec::ast::type::Any> node) {
  this->log(pantheios::error, node, "encountered field of type any");
}

void ParserGenerator::visit(node_ptr<spec::ast::type::Bool> node) {
  // TODO(ES): support bool parsing
}

void ParserGenerator::visit(node_ptr<spec::ast::type::Bytes> node) {
  auto item = current<ast::type::unit::item::Item>();
  if (item->attributes()->has("length")) {
    // TODO(ES): support incremental copy of byte fields
    // only if "chunked" attr is given?
    auto length = item->attributes()->lookup("length")->value();
    auto length_str = translator_.expression(length);

    if (!use_input_pointer_) {
      code_->addLine(util::fmt(
          "(*((dr::unit::var_bytes*) parse_dest)).len_ = %s;", length_str));
      code_->addLine(
          "parse_res = dr::parsing::util::allocateCopyBytes("
          "POS, in_buf_end, &(*((dr::unit::var_bytes*) parse_dest)).data_, "
          "(*((dr::unit::var_bytes*) parse_dest)).len_, area);");
    } else {
      code_->addLine(
          util::fmt("(*((dr::unit::var_stream_range*) parse_dest)).len_ = %s;",
                    length_str));
      code_->addLine(
          "(*((dr::unit::var_stream_range*) parse_dest)).start_ = *POS;");
      code_->addLine(
          "parse_res = dr::parsing::util::advance(POS, in_buf_end, "
          "(*((dr::unit::var_stream_range*) parse_dest)).len_);");
      // TODO(ES): increment reference counting of input buffer
    }
    emitCheckParseResult();
  } else {
    // TODO(ES): support "until" and eod parsing of bytes
  }
  // TODO(ES): support "chunked" byte fields? / embedded units
}

void ParserGenerator::visit(node_ptr<spec::ast::type::CAddr> node) {
  // TODO(ES): support caddr parsing
}

void ParserGenerator::visit(node_ptr<spec::ast::type::Double> node) {
  // TODO(ES): support double parsing
}

void ParserGenerator::visit(node_ptr<spec::ast::type::Enum> node) {
  // TODO(ES): support enum parsing
}

void ParserGenerator::visit(node_ptr<spec::ast::type::Integer> node) {
  auto byteorder = byteOrderLabel(node);
  code_->addLine(util::fmt(
      "parse_res = dr::parsing::util::parseInt%d_%s_%s("
      "POS, in_buf_end, parse_dest);",
      node->width(), node->_signed() ? "signed" : "unsigned", byteorder));
  emitCheckParseResult();
}

void ParserGenerator::visit(node_ptr<spec::ast::type::List> node) {
  // TODO(ES): support list parsing
}

void ParserGenerator::visit(node_ptr<spec::ast::type::Map> node) {
  // TODO(ES): support map parsing
}

void ParserGenerator::visit(node_ptr<spec::ast::type::Set> node) {
  // TODO(ES): support set parsing
}

void ParserGenerator::visit(node_ptr<spec::ast::type::Sink> node) {
  // TODO(ES): support sinks during parsing
}

void ParserGenerator::visit(node_ptr<spec::ast::type::String> node) {
  auto item = current<ast::type::unit::item::Item>();
  if (item->attributes()->has("length")) {
    // TODO(ES): support incremental copy of string fields
    // only if "chunked" attr is given?
    auto length = item->attributes()->lookup("length")->value();
    auto length_str = translator_.expression(length);
    // TODO(ES): assuming ascii here, what about other encodings?
    code_->addLine(util::fmt(
        "(*((dr::unit::var_string*) parse_dest)).len_ = %s;", length_str));
    code_->addLine(
        "parse_res = dr::parsing::util::allocateCopyBytes("
        "POS, in_buf_end, &(*((dr::unit::var_string*) parse_dest)).data_, "
        "(*((dr::unit::var_string*) parse_dest)).len_, area);");
    emitCheckParseResult();
  } else {
    // TODO(ES): support "until" and eod parsing of strings
  }
  // TODO(ES): support "chunked" string fields?
}

void ParserGenerator::visit(node_ptr<spec::ast::type::Tuple> node) {
  // TODO(ES): support tuple parsing
}

void ParserGenerator::visit(node_ptr<spec::ast::type::TypeType> node) {
  // TODO(ES): support typetype parsing
}

void ParserGenerator::visit(node_ptr<spec::ast::type::Unknown> node) {
  this->log(pantheios::error, node, "encountered field of unknown type");
}

void ParserGenerator::visit(node_ptr<spec::ast::type::Vector> node) {
  // TODO(ES): support vector parsing
}

void ParserGenerator::visit(node_ptr<spec::ast::type::Void> node) {
  this->log(pantheios::error, node, "encountered field of type void");
}

void ParserGenerator::visit(node_ptr<spec::ast::type::Bitset> node) {
  // TODO(ES): support bitset parsing
}

void ParserGenerator::visit(
    node_ptr<spec::ast::type::bitfield::Bitfield> node) {
  // TODO(ES): support bitfield parsing
}

void ParserGenerator::parse(node_ptr<ast::type::unit::item::Item> item) {
  std::string instr_label = newInstructionLabel(
      util::fmt("parse_%s_%s", unit_->id()->name(), item->id()->name()));

  KODE::Code instr;

  auto item_tmp = item_;
  auto code_tmp = code_;
  item_ = item;
  code_ = &instr;

  emitInitInstruction(instr_label);
  processOne(item);

  item_ = item_tmp;
  code_ = code_tmp;

  instr.newLine();
  code_->addBlock(instr);
}

std::string ParserGenerator::addTemp(std::string type) {
  auto name = newTempVarName();
  temp_vars_.push_back(std::make_pair(name, type));
  return name;
}

void ParserGenerator::emitInitInstruction(const std::string& instr_label) {
  code_->addLine(instr_label + ":");
  code_->addLine(util::fmt("state->advanceToInstruction(&&%s);", instr_label));
}

void ParserGenerator::emitAllocateIntoPointer(const std::string& pointer) {
  emitAllocateIntoPointerPointer("&" + pointer);
}

void ParserGenerator::emitAllocateIntoPointerPointer(
    const std::string& pointer) {
  code_->addLine(util::fmt("if (!area->allocate(%s))", pointer));
  code_->addLine("  return dr::parsing::ParseResult::AREA_FULL;");
  code_->newLine();
}

void ParserGenerator::emitCheckParseResult() {
  code_->addLine("if (parse_res != dr::parsing::ParseResult::DONE)");
  code_->addLine("  return parse_res;");
}

void ParserGenerator::emitPushBlockState() {
  code_->addLine("state->push<BlockState>();");
  // TODO(ES): initialization of block state members?
  // currently, unit pointer is written into by subsequent allocate unit
  code_->newLine();
}

std::string ParserGenerator::newInstructionLabel(std::string label_desc) {
  return util::fmt("lbl%i_%s", ++lastLabelId_, label_desc);
}

std::string ParserGenerator::newTempVarName() {
  return util::fmt("tmp%i", ++lastTempId_);
}

std::string ParserGenerator::byteOrderLabel(
    const node_ptr<spec::ast::type::Integer>& node) {
  auto bo_prop =
      current<spec::ast::type::unit::item::Item>()->inheritedProperty(
          "byteorder");
  if (!bo_prop) {
    log(pantheios::warning, node,
        "missing byteorder specification, assuming big");
    return "big";
  }
  auto bo_const_expr = ast::tryCast<ast::expression::Constant>(bo_prop);
  auto bo_const = ast::tryCast<ast::constant::Enum>(bo_const_expr->constant());
  return bo_const->label()->name();
}

std::string ParserGenerator::exprCurrentUnit() {
  return util::fmt("UNIT(%s)", translator_.type(unit_));
}

std::string ParserGenerator::exprCurrentUnitPP() {
  return util::fmt("UNITPP(%s)", translator_.type(unit_));
}

}  // namespace parsing
}  // namespace generation
}  // namespace diffingo
