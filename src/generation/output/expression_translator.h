/*
 * expression_translator.h
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

#ifndef SRC_GENERATION_OUTPUT_EXPRESSION_TRANSLATOR_H_
#define SRC_GENERATION_OUTPUT_EXPRESSION_TRANSLATOR_H_

#include <string>

#include "spec/ast/constant/bitset.h"
#include "spec/ast/constant/bool.h"
#include "spec/ast/constant/double.h"
#include "spec/ast/constant/enum.h"
#include "spec/ast/constant/integer.h"
#include "spec/ast/constant/none.h"
#include "spec/ast/constant/string.h"
#include "spec/ast/constant/tuple.h"
#include "spec/ast/expression/assign.h"
#include "spec/ast/expression/conditional.h"
#include "spec/ast/expression/constant.h"
#include "spec/ast/expression/ctor.h"
#include "spec/ast/expression/expression.h"
#include "spec/ast/expression/function.h"
#include "spec/ast/expression/id.h"
#include "spec/ast/expression/lambda.h"
#include "spec/ast/expression/list_comprehension.h"
#include "spec/ast/expression/member_attribute.h"
#include "spec/ast/expression/operator.h"
#include "spec/ast/expression/parser_state.h"
#include "spec/ast/expression/type.h"
#include "spec/ast/node.h"
#include "spec/ast/visitor.h"

namespace diffingo {
namespace generation {
namespace output {
class Translator;
}  // namespace output
}  // namespace generation
}  // namespace diffingo

namespace diffingo {
namespace generation {
namespace output {

using spec::ast::node_ptr;

class ExpressionTranslator : public spec::ast::Visitor<std::string> {
 public:
  explicit ExpressionTranslator(Translator* translator);
  virtual ~ExpressionTranslator();

  bool translate(node_ptr<spec::ast::expression::Expression> node,
                 std::string* result);

  using Visitor::visit;

  void visit(node_ptr<spec::ast::expression::Assign> node) override;
  void visit(node_ptr<spec::ast::expression::Conditional> node) override;
  void visit(node_ptr<spec::ast::expression::Constant> node) override;
  void visit(node_ptr<spec::ast::expression::Ctor> node) override;
  void visit(node_ptr<spec::ast::expression::Function> node) override;
  void visit(node_ptr<spec::ast::expression::ID> node) override;
  void visit(node_ptr<spec::ast::expression::Lambda> node) override;
  void visit(node_ptr<spec::ast::expression::ListComprehension> node) override;
  void visit(node_ptr<spec::ast::expression::MemberAttribute> node) override;
  void visit(node_ptr<spec::ast::expression::Operator> node) override;
  void visit(node_ptr<spec::ast::expression::ParserState> node) override;
  void visit(node_ptr<spec::ast::expression::Type> node) override;

  void visit(node_ptr<spec::ast::constant::Bitset> node) override;
  void visit(node_ptr<spec::ast::constant::Bool> node) override;
  void visit(node_ptr<spec::ast::constant::Double> node) override;
  void visit(node_ptr<spec::ast::constant::Enum> node) override;
  void visit(node_ptr<spec::ast::constant::Integer> node) override;
  void visit(node_ptr<spec::ast::constant::None> node) override;
  void visit(node_ptr<spec::ast::constant::String> node) override;
  void visit(node_ptr<spec::ast::constant::Tuple> node) override;

 private:
  static const std::string kDefaultResult;

  Translator* translator_;
};

}  // namespace output
}  // namespace generation
}  // namespace diffingo

#endif  // SRC_GENERATION_OUTPUT_EXPRESSION_TRANSLATOR_H_
