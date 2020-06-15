/*
 * serializer_generator.cpp
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

#include "generation/serializing/serializer_generator.h"

#include <kode/class.h>
#include <kode/code.h>
#include <kode/function.h>
#include <kode/membervariable.h>
#include <pantheios/pantheios.hpp>
#include <list>
#include <string>
#include <utility>

#include "generation/compiler.h"
#include "spec/ast/attribute.h"
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
namespace serializing {

SerializerGenerator::SerializerGenerator() {}

SerializerGenerator::~SerializerGenerator() {}

bool SerializerGenerator::run(node_ptr<ast::type::unit::Unit> node,
                              KODE::Class* serializer_cls,
                              const Options& options) {
  cls_ = serializer_cls;
  root_instr_ = newInstructionLabel("root");
  unit_ = node;
  options_ = &options;

  // -- create code blocks that will be filled during visiting unit elements --
  KODE::Code init_consts_body;
  init_consts_code_ = &init_consts_body;
  KODE::Code serialize_body_inner;
  code_ = &serialize_body_inner;

  // process length field updates
  for (auto item : node->items()) {
    if (auto f = ast::tryCast<ast::type::unit::item::field::Field>(item)) {
      updateLengthForField(f);
    }
  }
  code_->newLine();

  // execute variables first
  for (auto item : node->items()) {
    if (ast::tryCast<ast::type::unit::item::Variable>(item)) {
      serialize(item);
    }
  }
  code_->newLine();

  // serialize sequence of items and fill body of serialize method
  for (auto item : node->items()) {
    if (!ast::tryCast<ast::type::unit::item::Property>(item) &&
        !ast::tryCast<ast::type::unit::item::Variable>(item)) {
      serialize(item);
    }
  }

  // -- add code to serializer class --

  KODE::Code serialize_body;
  code_ = &serialize_body;

  // add temp var declarations
  code_->addLine("char* serialize_src;");
  code_->addLine("dr::serializing::SerializeResult serialize_res;");
  code_->addLine("char* dollar;");
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

  // push new block state, init unit pointer within state
  emitPushBlockState();
  code_->addLine("BLOCKSTATE->unit = unit;");

  // init stream position
  code_->addLine("*POS = out_buf_start;");
  code_->newLine();

  code_->addBlock(serialize_body_inner);

  // update bytes_read and return
  code_->addLine("*bytes_written = *POS - out_buf_start;");
  code_->addLine("return dr::serializing::SerializeResult::DONE;");

  // define macros
  cls_->addDeclarationMacro("#define POS state->stream_pos()");
  cls_->addDeclarationMacro("#define BLOCKSTATE state->peek<BlockState>()");
  cls_->addDeclarationMacro("#define UNIT(type) reinterpret_cast<type*>(unit)");
  cls_->addDeclarationMacro(
      "#define UNITPP(type) reinterpret_cast<type**>(&BLOCKSTATE->unit)");
  cls_->addDeclarationMacro("#define SELF(type) (*UNIT(type))");
  cls_->addDeclarationMacro(
      "#define DOLLARP(type) reinterpret_cast<type*>(dollar)");
  cls_->addDeclarationMacro("#define DOLLAR(type) (*DOLLARP(type))");

  // create serializer constants struct
  KODE::Class consts("SerializerConstants");
  for (auto m : consts_) {
    consts.addMemberVariable(m);
  }
  cls_->addNestedClass(consts);
  KODE::MemberVariable consts_var("consts_", "SerializerConstants", false,
                                  true);
  cls_->addMemberVariable(consts_var);
  KODE::Function init_consts("initSerializerConstants", "void",
                             KODE::Function::Private, false);
  init_consts.setBody(init_consts_body);
  cls_->addFunction(init_consts);

  // TODO(ES): create block state entry struct(s)
  KODE::Class block_state("BlockState");
  KODE::MemberVariable bs_unit("unit", "char*", false, true);
  block_state.addMemberVariable(bs_unit);
  cls_->addNestedClass(block_state);

  // generate and add serialize function
  KODE::Function serialize_func("serialize",
                                "dr::serializing::SerializeResult");
  serialize_func.addArgument("char* unit");
  serialize_func.addArgument("char* out_buf_start");
  serialize_func.addArgument("char* out_buf_end");
  serialize_func.addArgument("dr::parsing::ParserState* state");
  serialize_func.addArgument("size_t* bytes_written");
  serialize_func.setBody(serialize_body);
  cls_->addFunction(serialize_func);

  // TODO(ES): add a reset function

  return !errors();
}

void SerializerGenerator::visit(
    node_ptr<ast::type::unit::item::field::AtomicType> node) {
  if (!node->application_accessible() && options_->input_pointers) {
    // serialize from input pointer
    use_input_pointer_ = true;
  }

  if (node->serialized_type() != node->type()) {
    // TODO(ES): support var_bytes/string serializing here
    // (where to allocate temp bytes/string storage?)

    // need transform between serialized and internal types.
    // transform into temporary variable first
    auto temp_name = addTemp(translator_.type(node->serialized_type()));

    if (node->attributes()->has("transform_to")) {
      // TODO(ES): we assume it's possible to cast here for now.
      code_->addLine(util::fmt("%s = (%s) %s->%s;", temp_name,
                               translator_.type(node->serialized_type()),
                               exprCurrentUnit(),
                               translator_.unitFieldName(node->id()->name())));
    } else if (node->attributes()->has("transform")) {
      // TODO(ES): support custom transforms
    } else {
      log(pantheios::error, node,
          "unknown case for serialized type != internal type");
    }

    code_->addLine(
        util::fmt("serialize_src = reinterpret_cast<char*>(&%s);", temp_name));

    // serialize value from location
    processOne(node->serialized_type());
  } else {
    // no transform required - serialize directly from field.
    // set up pointer into field.
    code_->addLine(util::fmt(
        "serialize_src = reinterpret_cast<char*>(&%s->%s);", exprCurrentUnit(),
        translator_.unitFieldName(node->id()->name())));

    // serialize value from location
    processOne(node->serialized_type());
  }

  // reset input pointer switch
  use_input_pointer_ = false;
}

void SerializerGenerator::visit(
    node_ptr<ast::type::unit::item::field::Constant> node) {
  // TODO(ES)
  // statically convert constant to byte array in serialized format
  // add constant field to unit's "SerializerConstants" struct
  // copy to output
}

void SerializerGenerator::visit(
    node_ptr<ast::type::unit::item::field::Unit> node) {
  // TODO(ES): recurse into embedded unit
}

void SerializerGenerator::visit(
    node_ptr<ast::type::unit::item::field::Ctor> node) {
  // TODO(ES): serialize ctor field value - regexp / bytes
  // TODO(ES): support "ignored" fields, whose value will be thrown away during
  // parsing and regenerated later - similar to a constant, but according to
  // ctor / regexp
}

void SerializerGenerator::visit(
    node_ptr<ast::type::unit::item::field::Unknown> node) {
  this->log(pantheios::error, node, "encountered field of unknown type");
}

void SerializerGenerator::visit(
    node_ptr<ast::type::unit::item::field::container::List> node) {
  // TODO(ES): support lists
}

void SerializerGenerator::visit(
    node_ptr<ast::type::unit::item::field::container::Vector> node) {
  // TODO(ES): support vectors
}

void SerializerGenerator::visit(
    node_ptr<spec::ast::type::unit::item::field::switch_::Switch> node) {
  // TODO(ES): support switch as union struct
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
        serialize(x);
      }
      code_->addLine("break;");
      code_->unindent();
    }
    code_->addLine("}");
  } else {
    // TODO(ES): support look-ahead switch
  }
}

void SerializerGenerator::visit(
    node_ptr<ast::type::unit::item::Variable> node) {
  if (node->attributes()->has("serialize")) {
    // evaluate value expression and store into variable field
    auto pattr = node->attributes()->lookup("serialize");
    code_->addLine(util::fmt("dollar = reinterpret_cast<char*>(&%s->%s);",
                             exprCurrentUnit(),
                             translator_.unitFieldName(node->id()->name())));
    code_->addLine(util::fmt("%s;", translator_.expression(pattr->value())));
    // TODO(ES): support strings, bytes constructors, ..
  }
}

void SerializerGenerator::visit(node_ptr<spec::ast::type::Any> node) {
  this->log(pantheios::error, node, "encountered field of type any");
}

void SerializerGenerator::visit(node_ptr<spec::ast::type::Bool> node) {
  // TODO(ES): support bool serializing
}

void SerializerGenerator::visit(node_ptr<spec::ast::type::Bytes> node) {
  // TODO(ES): support incremental copy of byte fields
  // only if "chunked" attr is given?

  if (!use_input_pointer_) {
    code_->addLine(
        "serialize_res = dr::serializing::util::copyBytes("
        "(*((dr::unit::var_bytes*) serialize_src)).data_, "
        "(*((dr::unit::var_bytes*) serialize_src)).len_, "
        "POS, out_buf_end);");
  } else {
    code_->addLine(
        "serialize_res = dr::serializing::util::copyBytes("
        "(*((dr::unit::var_stream_range*) serialize_src)).start_, "
        "(*((dr::unit::var_stream_range*) serialize_src)).len_, "
        "POS, out_buf_end);");
    // TODO(ES): decrement reference counting of original input buffer
  }
  emitCheckSerializeResult();

  // TODO(ES): support "chunked" byte fields? / embedded units
}

void SerializerGenerator::visit(node_ptr<spec::ast::type::CAddr> node) {
  // TODO(ES): support caddr serializing
}

void SerializerGenerator::visit(node_ptr<spec::ast::type::Double> node) {
  // TODO(ES): support double serializing
}

void SerializerGenerator::visit(node_ptr<spec::ast::type::Enum> node) {
  // TODO(ES): support enum serializing
}

void SerializerGenerator::visit(node_ptr<spec::ast::type::Integer> node) {
  auto byteorder = byteOrderLabel(node);
  code_->addLine(util::fmt(
      "serialize_res = dr::serializing::util::serializeInt%d_%s_%s("
      "serialize_src, POS, out_buf_end);",
      node->width(), node->_signed() ? "signed" : "unsigned", byteorder));
  emitCheckSerializeResult();
}

void SerializerGenerator::visit(node_ptr<spec::ast::type::List> node) {
  // TODO(ES): support list serializing
}

void SerializerGenerator::visit(node_ptr<spec::ast::type::Map> node) {
  // TODO(ES): support map serializing
}

void SerializerGenerator::visit(node_ptr<spec::ast::type::Set> node) {
  // TODO(ES): support set serializing
}

void SerializerGenerator::visit(node_ptr<spec::ast::type::Sink> node) {
  // TODO(ES): support sinks during serializing
}

void SerializerGenerator::visit(node_ptr<spec::ast::type::String> node) {
  // TODO(ES): support incremental copy of string fields
  // only if "chunked" attr is given?

  // TODO(ES): assuming ascii here, what about other encodings?

  code_->addLine(
      "serialize_res = dr::serializing::util::copyBytes("
      "(*((dr::unit::var_string*) serialize_src)).data_, "
      "(*((dr::unit::var_string*) serialize_src)).len_, "
      "POS, out_buf_end);");
  emitCheckSerializeResult();

  // TODO(ES): support "chunked" string fields?
}

void SerializerGenerator::visit(node_ptr<spec::ast::type::Tuple> node) {
  // TODO(ES): support tuple serializing
}

void SerializerGenerator::visit(node_ptr<spec::ast::type::TypeType> node) {
  // TODO(ES): support typetype serializing
}

void SerializerGenerator::visit(node_ptr<spec::ast::type::Unknown> node) {
  this->log(pantheios::error, node, "encountered field of unknown type");
}

void SerializerGenerator::visit(node_ptr<spec::ast::type::Vector> node) {
  // TODO(ES): support vector serializing
}

void SerializerGenerator::visit(node_ptr<spec::ast::type::Void> node) {
  this->log(pantheios::error, node, "encountered field of type void");
}

void SerializerGenerator::visit(node_ptr<spec::ast::type::Bitset> node) {
  // TODO(ES): support bitset serializing
}

void SerializerGenerator::visit(
    node_ptr<spec::ast::type::bitfield::Bitfield> node) {
  // TODO(ES): support bitfield serializing
}

void SerializerGenerator::serialize(
    node_ptr<ast::type::unit::item::Item> item) {
  // variables don't need extra instructions
  // TODO(ES): except if they allocate.
  if (ast::tryCast<ast::type::unit::item::Variable>(item)) {
    processOne(item);
    return;
  }

  std::string instr_label = newInstructionLabel(
      util::fmt("serialize_%s_%s", unit_->id()->name(), item->id()->name()));

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

void SerializerGenerator::updateLengthForField(
    node_ptr<spec::ast::type::unit::item::field::Field> field) {
  if (ast::tryCast<spec::ast::type::unit::item::field::container::List>(
          field)) {
    // TODO(ES) support lists.
  } else if (ast::tryCast<spec::ast::type::Bytes>(field->serialized_type()) ||
             ast::tryCast<spec::ast::type::String>(field->serialized_type())) {
    // TODO(ES) app-accessible check here should be a check ser_update
    // dependency, too? (what if inaccessible field is updated during
    // serializing?)
    if (field->application_accessible() && field->attributes()->has("length")) {
      auto length = field->attributes()->lookup("length")->value();
      if (auto op = ast::tryCast<spec::ast::expression::Operator>(length)) {
        if (op->kind() == spec::ast::expression::Operator::Kind::Attribute) {
          auto length_str = translator_.expression(length);
          code_->addLine(
              util::fmt("serialize_src = reinterpret_cast<char*>(&%s->%s);",
                        exprCurrentUnit(),
                        translator_.unitFieldName(field->id()->name())));
          code_->addLine(
              util::fmt("%s = (*((dr::unit::var_bytes*) serialize_src)).len_;",
                        length_str));
        }
      }
    }
  }
}

std::string SerializerGenerator::addTemp(std::string type) {
  auto name = newTempVarName();
  temp_vars_.push_back(std::make_pair(name, type));
  return name;
}

void SerializerGenerator::emitInitInstruction(const std::string& instr_label) {
  code_->addLine(instr_label + ":");
  code_->addLine(util::fmt("state->advanceToInstruction(&&%s);", instr_label));
}

void SerializerGenerator::emitCheckSerializeResult() {
  code_->addLine(
      "if (serialize_res != dr::serializing::SerializeResult::DONE)");
  code_->addLine("  return serialize_res;");
}

void SerializerGenerator::emitPushBlockState() {
  code_->addLine("state->push<BlockState>();");
  // TODO(ES): initialization of block state members?
  // currently, unit pointer is written into by subsequent allocate unit
  code_->newLine();
}

std::string SerializerGenerator::newInstructionLabel(std::string label_desc) {
  return util::fmt("lbl%i_%s", ++lastLabelId_, label_desc);
}

std::string SerializerGenerator::newTempVarName() {
  return util::fmt("tmp%i", ++lastTempId_);
}

std::string SerializerGenerator::byteOrderLabel(
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

std::string SerializerGenerator::exprCurrentUnit() {
  return util::fmt("UNIT(%s)", translator_.type(unit_));
}

std::string SerializerGenerator::exprCurrentUnitPP() {
  return util::fmt("UNITPP(%s)", translator_.type(unit_));
}

}  // namespace serializing
}  // namespace generation
}  // namespace diffingo
