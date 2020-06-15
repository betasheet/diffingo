/*
 * parser_state.h
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

#ifndef SRC_SPEC_AST_EXPRESSION_PARSER_STATE_H_
#define SRC_SPEC_AST_EXPRESSION_PARSER_STATE_H_

#include <string>

#include "spec/ast/expression/expression.h"
#include "spec/ast/id.h"
#include "spec/ast/location.h"
#include "spec/ast/node.h"
#include "spec/ast/type/type.h"
#include "spec/ast/visitor.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace expression {

class ParserState : public Expression {
 public:
  enum Kind {
    SELF,          /// A \a self expression.
    DOLLARDOLLAR,  /// A \a $$ expression.
    PARAMETER      /// A type parameter, with \a id giving its name.
  };

  ParserState(Kind kind, node_ptr<ast::ID> id, node_ptr<type::Type> unit,
              node_ptr<type::Type> type = nullptr,
              const Location& l = Location::None);
  virtual ~ParserState();

  /// Return the kind of expression.
  Kind kind() const {return kind_;}

  /// Returns the associated ID, if set.
  node_ptr<ast::ID> id() const {return id_;}

  /// Returns the unit type which the referenced parser state belongs to.
  node_ptr<type::Type> unit() const {return unit_;}

  /// Sets the unit type which the referenced parser state belongs to.
  void set_unit(node_ptr<type::Type> unit);

  /// Sets the expression's type. This is intended to be used from the
  /// resolver only.
  void set_type(node_ptr<type::Type> type);

  node_ptr<Expression> clone() override;

  std::string render() override;

  node_ptr<type::Type> type() const override { return type_; }

  ACCEPT_VISITOR(Expression);

 private:
  Kind kind_;
  node_ptr<ast::ID> id_;
  node_ptr<type::Type> unit_;
  node_ptr<type::Type> type_;
};

}  // namespace expression
}  // namespace ast
}  // namespace spec
}  // namespace diffingo

#endif  // SRC_SPEC_AST_EXPRESSION_PARSER_STATE_H_
