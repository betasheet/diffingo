/*
 * type_translator.h
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

#ifndef SRC_GENERATION_OUTPUT_TYPE_TRANSLATOR_H_
#define SRC_GENERATION_OUTPUT_TYPE_TRANSLATOR_H_

#include <string>

#include "spec/ast/node.h"
#include "spec/ast/type/atomic_types.h"
#include "spec/ast/type/bitfield.h"
#include "spec/ast/type/bitset.h"
#include "spec/ast/type/container.h"
#include "spec/ast/type/enum.h"
#include "spec/ast/type/function.h"
#include "spec/ast/type/reg_exp.h"
#include "spec/ast/type/unit.h"
#include "spec/ast/visitor.h"

namespace diffingo {
namespace generation {
namespace output {

using spec::ast::node_ptr;

class TypeTranslator : public spec::ast::Visitor<std::string> {
 public:
  TypeTranslator();
  virtual ~TypeTranslator();

  bool translate(node_ptr<spec::ast::type::Type> node, std::string* result);

  using Visitor::visit;

  void visit(node_ptr<spec::ast::type::Bitset> node) override;
  void visit(node_ptr<spec::ast::type::Bool> node) override;
  void visit(node_ptr<spec::ast::type::Bytes> node) override;
  void visit(node_ptr<spec::ast::type::CAddr> node) override;
  void visit(node_ptr<spec::ast::type::Double> node) override;
  void visit(node_ptr<spec::ast::type::Enum> node) override;
  void visit(node_ptr<spec::ast::type::Integer> node) override;
  void visit(node_ptr<spec::ast::type::List> node) override;
  void visit(node_ptr<spec::ast::type::Map> node) override;
  void visit(node_ptr<spec::ast::type::RegExp> node) override;
  void visit(node_ptr<spec::ast::type::Set> node) override;
  void visit(node_ptr<spec::ast::type::Sink> node) override;
  void visit(node_ptr<spec::ast::type::String> node) override;
  void visit(node_ptr<spec::ast::type::Tuple> node) override;
  void visit(node_ptr<spec::ast::type::TypeType> node) override;
  void visit(node_ptr<spec::ast::type::Vector> node) override;
  void visit(node_ptr<spec::ast::type::Void> node) override;
  void visit(node_ptr<spec::ast::type::bitfield::Bitfield> node) override;
  void visit(node_ptr<spec::ast::type::unit::Unit> node) override;

 private:
  static const std::string kDefaultResult;
};

}  // namespace output
}  // namespace generation
}  // namespace diffingo

#endif  // SRC_GENERATION_OUTPUT_TYPE_TRANSLATOR_H_
