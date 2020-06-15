/*
 * parser_generator.h
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

#ifndef SRC_GENERATION_PARSING_PARSER_GENERATOR_H_
#define SRC_GENERATION_PARSING_PARSER_GENERATOR_H_

#include <kode/class.h>
#include <kode/code.h>
#include <kode/membervariable.h>
#include <list>
#include <string>
#include <utility>

#include "generation/output/translator.h"
#include "spec/ast/node.h"
#include "spec/ast/type/atomic_types.h"
#include "spec/ast/type/bitfield.h"
#include "spec/ast/type/bitset.h"
#include "spec/ast/type/container.h"
#include "spec/ast/type/enum.h"
#include "spec/ast/type/unit.h"
#include "spec/ast/visitor.h"

namespace diffingo {
namespace generation {

struct Options;

namespace parsing {

using spec::ast::node_ptr;

class ParserGenerator : public spec::ast::Visitor<> {
 public:
  ParserGenerator();
  virtual ~ParserGenerator();

  bool run(node_ptr<spec::ast::type::unit::Unit> node, KODE::Class* parser_cls,
           const Options& options);

  using Visitor::visit;

  void visit(
      node_ptr<spec::ast::type::unit::item::field::AtomicType> node) override;
  void visit(
      node_ptr<spec::ast::type::unit::item::field::Constant> node) override;
  void visit(node_ptr<spec::ast::type::unit::item::field::Unit> node) override;
  void visit(node_ptr<spec::ast::type::unit::item::field::Ctor> node) override;
  void visit(
      node_ptr<spec::ast::type::unit::item::field::Unknown> node) override;
  void visit(node_ptr<spec::ast::type::unit::item::field::container::List> node)
      override;
  void visit(node_ptr<spec::ast::type::unit::item::field::container::Vector>
                 node) override;
  void visit(node_ptr<spec::ast::type::unit::item::field::switch_::Switch> node)
      override;

  void visit(node_ptr<spec::ast::type::unit::item::Variable> node) override;

  void visit(node_ptr<spec::ast::type::Any> node) override;
  void visit(node_ptr<spec::ast::type::Bool> node) override;
  void visit(node_ptr<spec::ast::type::Bytes> node) override;
  void visit(node_ptr<spec::ast::type::CAddr> node) override;
  void visit(node_ptr<spec::ast::type::Double> node) override;
  void visit(node_ptr<spec::ast::type::Enum> node) override;
  void visit(node_ptr<spec::ast::type::Integer> node) override;
  void visit(node_ptr<spec::ast::type::List> node) override;
  void visit(node_ptr<spec::ast::type::Map> node) override;
  void visit(node_ptr<spec::ast::type::Set> node) override;
  void visit(node_ptr<spec::ast::type::Sink> node) override;
  void visit(node_ptr<spec::ast::type::String> node) override;
  void visit(node_ptr<spec::ast::type::Tuple> node) override;
  void visit(node_ptr<spec::ast::type::TypeType> node) override;
  void visit(node_ptr<spec::ast::type::Unknown> node) override;
  void visit(node_ptr<spec::ast::type::Vector> node) override;
  void visit(node_ptr<spec::ast::type::Void> node) override;
  void visit(node_ptr<spec::ast::type::Bitset> node) override;
  void visit(node_ptr<spec::ast::type::bitfield::Bitfield> node) override;

 private:
  KODE::Class* cls_ = nullptr;
  KODE::Code* code_ = nullptr;
  KODE::Code* init_consts_code_ = nullptr;
  std::list<KODE::MemberVariable> consts_;
  std::list<std::pair<std::string, std::string>> temp_vars_;
  std::string root_instr_;

  node_ptr<spec::ast::type::unit::Unit> unit_ = nullptr;
  node_ptr<spec::ast::type::unit::item::Item> item_ = nullptr;

  int lastLabelId_ = 0;
  int lastTempId_ = 0;
  output::Translator translator_;

  bool use_input_pointer_ = false;
  const Options* options_ = nullptr;

  void parse(node_ptr<spec::ast::type::unit::item::Item> item);

  std::string addTemp(std::string type);

  void emitInitInstruction(const std::string& instr_label);
  void emitAllocateIntoPointer(const std::string& pointer);
  void emitAllocateIntoPointerPointer(const std::string& pointer);
  void emitCheckParseResult();
  void emitPushBlockState();

  std::string newInstructionLabel(std::string label_desc = std::string());
  std::string newTempVarName();
  std::string byteOrderLabel(const node_ptr<spec::ast::type::Integer>& node);
  std::string exprCurrentUnit();
  std::string exprCurrentUnitPP();
};

}  // namespace parsing
}  // namespace generation
}  // namespace diffingo

#endif  // SRC_GENERATION_PARSING_PARSER_GENERATOR_H_
