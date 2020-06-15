/*
 * operator.cpp
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

#include "spec/ast/expression/operator.h"

#include <cassert>
#include <list>
#include <memory>
#include <string>

#include "spec/ast/exception.h"
#include "spec/ast/node.h"
#include "spec/ast/type/atomic_types.h"
#include "util/util.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace expression {

Operator::Operator(Kind kind, const expression_list& operands,
                   const Location& l)
    : Expression(l), kind_(kind), type_(std::make_shared<type::Unknown>(l)) {
  for (auto op : operands) operands_.push_back(op);

  for (auto op : operands_) addChild(op);
}

Operator::~Operator() {}

expression_list Operator::operands() const {
  expression_list ops;
  for (auto o : operands_) ops.push_back(o);
  return ops;
}

node_ptr<Expression> Operator::clone() {
  expression_list ops;
  for (auto o : operands_) ops.push_back(o->clone());
  return ast::newNodePtr(std::make_shared<Operator>(kind(), ops, location()));
}

std::string Operator::render() {
  std::string s = "(";

  auto it = operands_.begin();
  switch (kind_) {
    case None:
      s += internalRenderPrefix("NONE");
      break;
    case Attribute:
      s += internalRenderInfix(".", false);
      break;
    case AttributeAssign:
      assert(operands_.size() == 3);
      s += (*it)->render() + ".";
      s += (*++it)->render() + " = ";
      s += (*++it)->render();
      break;
    case BitAnd:
      s += internalRenderInfix("&");
      break;
    case BitOr:
      s += internalRenderInfix("|");
      break;
    case BitXor:
      s += internalRenderInfix("^");
      break;
    case Call:
      s += internalRenderInfix("", false);
      break;
    case Coerce:
      s += internalRenderPrefix("COERCE");
      break;
    case Cast:
      assert(operands_.size() == 2);
      s += "CAST<";
      s += (*it)->render() + ">(";
      s += (*++it)->render() + ")";
      break;
    case DecrPostfix:
      s += internalRenderPostfix("--", false);
      break;
    case DecrPrefix:
      s += internalRenderPrefix("--", false);
      break;
    case Deref:
      s += internalRenderPrefix("*", false);
      break;
    case Div:
      s += internalRenderInfix("/");
      break;
    case Equal:
      s += internalRenderInfix("==");
      break;
    case Greater:
      s += internalRenderInfix(">");
      break;
    case HasAttribute:
      s += internalRenderInfix("?.");
      break;
    case IncrPostfix:
      s += internalRenderPostfix("++", false);
      break;
    case IncrPrefix:
      s += internalRenderPrefix("++", false);
      break;
    case In:
      s += internalRenderInfix("IN");
      break;
    case Index:
      assert(operands_.size() == 2);
      s += (*it)->render() + "[";
      s += (*++it)->render() + "]";
      break;
    case IndexAssign:
      assert(operands_.size() == 3);
      s += (*it)->render() + "[";
      s += (*++it)->render() + "] = ";
      s += (*++it)->render();
      break;
    case LogicalAnd:
      s += internalRenderInfix("&&");
      break;
    case LogicalOr:
      s += internalRenderInfix("||");
      break;
    case Less:
      s += internalRenderInfix("<");
      break;
    case MethodCall:
      assert(operands_.size() == 3);
      s += (*it)->render() + ".";
      s += (*++it)->render();
      s += (*++it)->render();
      break;
    case Minus:
      s += internalRenderInfix("-");
      break;
    case MinusAssign:
      s += internalRenderInfix("-=");
      break;
    case Mod:
      s += internalRenderInfix("mod");
      break;
    case Mult:
      s += internalRenderInfix("*");
      break;
    case Not:
      s += internalRenderPrefix("!");
      break;
    case Plus:
      s += internalRenderInfix("+");
      break;
    case PlusAssign:
      s += internalRenderInfix("+=");
      break;
    case Power:
      s += internalRenderInfix("**");
      break;
    case ShiftLeft:
      s += internalRenderInfix("<<");
      break;
    case ShiftRight:
      s += internalRenderInfix(">>");
      break;
    case SignNeg:
      s += internalRenderPrefix("-", false);
      break;
    case SignPos:
      s += internalRenderPrefix("+", false);
      break;
    case Size:
      assert(operands_.size() == 1);
      s += "|" + (*it)->render() + "|";
      break;
    default:
      throw InternalError(util::fmt("unknown operator kind: %d", kind_), this);
  }

  s += " " + Expression::render() + ")";
  return s;
}

std::string Operator::internalRenderPrefix(std::string prefix,
                                           bool whitespace) {
  auto s = prefix;
  for (auto o : operands_) s += (whitespace ? " " : "") + o->render();
  return s;
}

std::string Operator::internalRenderInfix(std::string infix, bool whitespace) {
  if (operands_.size() < 2) {
    return internalRenderPrefix(infix);
  }

  std::string s = "";

  bool first = true;
  for (auto o : operands_) {
    if (first) {
      first = false;
    } else {
      if (whitespace) s += " ";
      s += infix;
      if (whitespace) s += " ";
    }
    s += o->render();
  }

  return s;
}

std::string Operator::internalRenderPostfix(std::string postfix,
                                            bool whitespace) {
  std::string s = "";
  for (auto o : operands_) s += o->render() + (whitespace ? " " : "");
  s += postfix;
  return s;
}

}  // namespace expression
}  // namespace ast
}  // namespace spec
}  // namespace diffingo
