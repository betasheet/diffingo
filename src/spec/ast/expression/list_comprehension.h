/*
 * list_comprehension.h
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

#ifndef SRC_SPEC_AST_EXPRESSION_LIST_COMPREHENSION_H_
#define SRC_SPEC_AST_EXPRESSION_LIST_COMPREHENSION_H_

#include <memory>
#include <string>

#include "spec/ast/expression/expression.h"
#include "spec/ast/expression/id.h"
#include "spec/ast/location.h"
#include "spec/ast/node.h"
#include "spec/ast/visitor.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace expression {

class ListComprehension : public Expression {
 public:
  ListComprehension(node_ptr<Expression> output,
                    node_ptr<ast::ID> variable,
                    node_ptr<Expression> input,
                    node_ptr<Expression> predicate = nullptr,
                    const Location& l = Location::None);
  virtual ~ListComprehension();

  node_ptr<type::Type> type() const override;

  node_ptr<Expression> output() const { return output_; }

  node_ptr<ast::ID> variable() const { return variable_; }

  node_ptr<Expression> input() const { return input_; }

  node_ptr<Expression> predicate() const { return predicate_; }

  node_ptr<Expression> clone() override;

  std::string render() override;

  ACCEPT_VISITOR(Expression)

 private:
  node_ptr<Expression> output_;
  node_ptr<ast::ID> variable_;
  node_ptr<Expression> input_;
  node_ptr<Expression> predicate_;
};

}  // namespace expression
}  // namespace ast
}  // namespace spec
}  // namespace diffingo

#endif  // SRC_SPEC_AST_EXPRESSION_LIST_COMPREHENSION_H_
