/*
 * expression_translator.cpp
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

#include "generation/output/expression_translator.h"

#include <pantheios/pantheios.hpp>
#include <string>

#include "generation/output/translator.h"
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
#include "spec/ast/expression/function.h"
#include "spec/ast/expression/id.h"
#include "spec/ast/expression/lambda.h"
#include "spec/ast/expression/list_comprehension.h"
#include "spec/ast/expression/member_attribute.h"
#include "spec/ast/expression/operator.h"
#include "spec/ast/expression/parser_state.h"
#include "spec/ast/expression/type.h"
#include "spec/ast/id.h"
#include "spec/ast/node.h"
#include "spec/ast/visitor.h"
#include "util/util.h"

namespace ast = diffingo::spec::ast;

namespace diffingo {
namespace generation {
namespace output {

using ast::node_ptr;

const std::string ExpressionTranslator::kDefaultResult =
    "[[EXPRESSION UNSUPPORTED]]";

ExpressionTranslator::ExpressionTranslator(Translator* translator)
    : translator_(translator) {
  setDefaultResult(kDefaultResult);
}

ExpressionTranslator::~ExpressionTranslator() {}

bool ExpressionTranslator::translate(node_ptr<ast::expression::Expression> node,
                                     std::string* result) {
  return processOne(node, result);
}

void ExpressionTranslator::visit(node_ptr<ast::expression::Assign> node) {
  std::string dst;
  std::string src;
  processOne(node->dst(), &dst);
  processOne(node->src(), &src);
  // TODO(ES): this may need some deref-magic to work correctly?
  setResult(util::fmt("(%s = %s)", dst, src));
  // TODO(ES): support assignment for strings, bytes, lists, ..
}

void ExpressionTranslator::visit(node_ptr<ast::expression::Conditional> node) {
  std::string cond;
  std::string _true;
  processOne(node->cond(), &cond);
  processOne(node->_true(), &_true);
  if (node->_false()) {
    std::string _false;
    processOne(node->_false(), &_false);
    setResult(util::fmt("(%s ? %s : %s)", cond, _true, _false));
  } else {
    setResult(util::fmt("(%s ? %s : 0)", cond, _true));
  }
}

void ExpressionTranslator::visit(node_ptr<ast::expression::Constant> node) {
  std::string res;
  processOne(node->constant(), &res);
  setResult(res);
}

void ExpressionTranslator::visit(node_ptr<ast::expression::Ctor> node) {
  // TODO(ES): support regex/byte ctor expressions
  // may want to init these as constants and reference them later only
}

void ExpressionTranslator::visit(node_ptr<ast::expression::Function> node) {
  // TODO(ES): support calling functions
}

void ExpressionTranslator::visit(node_ptr<ast::expression::ID> node) {
  // TODO(ES): does this need to be cleverer than this?
  setResult(node->id()->pathAsString());
}

void ExpressionTranslator::visit(node_ptr<ast::expression::Lambda> node) {
  // TODO(ES): support lambda expressions
}

void ExpressionTranslator::visit(
    node_ptr<ast::expression::ListComprehension> node) {
  // TODO(ES): support list comprehensions
}

void ExpressionTranslator::visit(
    node_ptr<ast::expression::MemberAttribute> node) {
  setResult(node->attribute()->name());
}

void ExpressionTranslator::visit(node_ptr<ast::expression::Operator> node) {
  auto ops = node->operands();
  auto it = ops.begin();
  std::string a;
  std::string b;
  std::string c;

  switch (node->kind()) {
    case ast::expression::Operator::Kind::Attribute:
      processOne(*it, &a);
      processOne(*++it, &b);
      setResult(util::fmt("(%s.%s)", a, b));
      break;
    case ast::expression::Operator::Kind::AttributeAssign:
      processOne(*it, &a);
      processOne(*++it, &b);
      processOne(*++it, &c);
      setResult(util::fmt("(%s.%s = %s)", a, b, c));
      break;
    case ast::expression::Operator::Kind::Call:
      processOne(*it, &a);
      processOne(*++it, &b);  // tuple expr
      setResult(util::fmt("(%s%s)", a, b));
      break;
    case ast::expression::Operator::Kind::Cast:
      processOne(*it, &a);
      processOne(*++it, &b);
      setResult(util::fmt("((%s) %s)", b, a));
      break;
    case ast::expression::Operator::Kind::DecrPostfix:
      processOne(*it, &a);
      setResult(util::fmt("(%s--)", a));
      break;
    case ast::expression::Operator::Kind::DecrPrefix:
      processOne(*it, &a);
      setResult(util::fmt("(--%s)", a));
      break;
    case ast::expression::Operator::Kind::Equal:
      processOne(*it, &a);
      processOne(*++it, &b);
      setResult(util::fmt("(%s == %s)", a, b));
      break;
    case ast::expression::Operator::Kind::Greater:
      processOne(*it, &a);
      processOne(*++it, &b);
      setResult(util::fmt("(%s > %s)", a, b));
      break;
    case ast::expression::Operator::Kind::IncrPostfix:
      processOne(*it, &a);
      setResult(util::fmt("(%s++)", a));
      break;
    case ast::expression::Operator::Kind::IncrPrefix:
      processOne(*it, &a);
      processOne(*++it, &b);
      setResult(util::fmt("(++%s)", a));
      break;
    case ast::expression::Operator::Kind::Less:
      processOne(*it, &a);
      processOne(*++it, &b);
      setResult(util::fmt("(%s < %s)", a, b));
      break;
    case ast::expression::Operator::Kind::LogicalAnd:
      processOne(*it, &a);
      processOne(*++it, &b);
      setResult(util::fmt("(%s && %s)", a, b));
      break;
    case ast::expression::Operator::Kind::LogicalOr:
      processOne(*it, &a);
      processOne(*++it, &b);
      setResult(util::fmt("(%s || %s)", a, b));
      break;
    case ast::expression::Operator::Kind::MethodCall:
      processOne(*it, &a);
      processOne(*++it, &b);
      processOne(*++it, &c);
      setResult(util::fmt("(%s.%s%s)", a, b, c));
      break;
    case ast::expression::Operator::Kind::Minus:
      processOne(*it, &a);
      processOne(*++it, &b);
      setResult(util::fmt("(%s - %s)", a, b));
      break;
    case ast::expression::Operator::Kind::Not:
      processOne(*it, &a);
      setResult(util::fmt("(!%s)", a));
      break;
    case ast::expression::Operator::Kind::Plus:
      processOne(*it, &a);
      processOne(*++it, &b);
      setResult(util::fmt("(%s + %s)", a, b));
      break;
    case ast::expression::Operator::Kind::BitAnd:
    case ast::expression::Operator::Kind::BitOr:
    case ast::expression::Operator::Kind::BitXor:
    case ast::expression::Operator::Kind::Coerce:
    case ast::expression::Operator::Kind::Deref:
    case ast::expression::Operator::Kind::Div:
    case ast::expression::Operator::Kind::HasAttribute:
    case ast::expression::Operator::Kind::In:
    case ast::expression::Operator::Kind::Index:
    case ast::expression::Operator::Kind::IndexAssign:
    case ast::expression::Operator::Kind::MinusAssign:
    case ast::expression::Operator::Kind::Mod:
    case ast::expression::Operator::Kind::Mult:
    case ast::expression::Operator::Kind::None:
    case ast::expression::Operator::Kind::PlusAssign:
    case ast::expression::Operator::Kind::Power:
    case ast::expression::Operator::Kind::ShiftLeft:
    case ast::expression::Operator::Kind::ShiftRight:
    case ast::expression::Operator::Kind::SignNeg:
    case ast::expression::Operator::Kind::SignPos:
    case ast::expression::Operator::Kind::Size:
    default:
      log(pantheios::error, node, "unsupported Operator expression");
      break;
  }
}

void ExpressionTranslator::visit(node_ptr<ast::expression::ParserState> node) {
  switch (node->kind()) {
    case ast::expression::ParserState::SELF:
      setResult(util::fmt("SELF(%s)", translator_->type(node->type())));
      break;
    case ast::expression::ParserState::DOLLARDOLLAR:
      setResult(util::fmt("DOLLAR(%s)", translator_->type(node->type())));
      break;
    case ast::expression::ParserState::PARAMETER:
      setResult(util::fmt("PARAM(%s, %s)", node->id()->name(),
                          translator_->type(node->type())));
      break;
    default:
      log(pantheios::error, node, "unsupported ParserState expression");
      break;
  }
}

void ExpressionTranslator::visit(node_ptr<ast::expression::Type> node) {
  // TODO(ES): support type values
  setResult(translator_->type(node->contained_type()));
}

void ExpressionTranslator::visit(node_ptr<ast::constant::Bitset> node) {
  // TODO(ES): support bitsets
}

void ExpressionTranslator::visit(node_ptr<ast::constant::Bool> node) {
  setResult(node->valueAsString());
}

void ExpressionTranslator::visit(node_ptr<ast::constant::Double> node) {
  setResult(node->valueAsString());
}

void ExpressionTranslator::visit(node_ptr<ast::constant::Enum> node) {
  auto e_type = ast::tryCast<ast::type::Enum>(node->type());
  setResult(
      util::fmt("%s::%s", translator_->type(e_type), node->label()->name()));
}

void ExpressionTranslator::visit(node_ptr<ast::constant::Integer> node) {
  setResult(node->valueAsString());
}

void ExpressionTranslator::visit(node_ptr<ast::constant::None> node) {
  setResult("nullptr");
}

void ExpressionTranslator::visit(node_ptr<ast::constant::String> node) {
  setResult(node->valueAsString());
}

void ExpressionTranslator::visit(node_ptr<ast::constant::Tuple> node) {
  // TODO(ES): currently only supporting tuples in calls
  std::string tmp;
  std::string e_str;
  for (auto e : node->value()) {
    processOne(e, &e_str);
    if (!tmp.empty()) tmp += ", ";
    tmp += e_str;
  }
  setResult(util::fmt("(%s)", tmp));
}

}  // namespace output
}  // namespace generation
}  // namespace diffingo
