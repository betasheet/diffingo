/*
 * function.h
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

#ifndef SRC_SPEC_AST_TYPE_FUNCTION_H_
#define SRC_SPEC_AST_TYPE_FUNCTION_H_

#include <list>
#include <memory>
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
namespace type {
namespace function {

class Parameter : public Node {
 public:
  Parameter(node_ptr<ID> id, node_ptr<Type> type, bool constant,
            node_ptr<expression::Expression> default_value,
            Location l = Location::None);

  Parameter(node_ptr<Type> type, bool constant,
            Location l = Location::None);

  virtual ~Parameter();

  node_ptr<ID> id() const { return id_; }

  node_ptr<Type> type() const { return type_; }

  bool constant() const { return constant_; }

  node_ptr<expression::Expression> default_value() const {
    return default_value_;
  }

  std::string render() override;

  ACCEPT_VISITOR(Node)

 private:
  node_ptr<ID> id_;
  node_ptr<Type> type_;
  bool constant_;
  node_ptr<expression::Expression> default_value_;
};

class Result : public Node {
 public:
  Result(node_ptr<Type> type, bool constant,
         Location l = Location::None);
  virtual ~Result();

  node_ptr<Type> type() const { return type_; }

  bool constant() const { return constant_; }

  std::string render() override;

  ACCEPT_VISITOR(Node)

 private:
  node_ptr<Type> type_;
  bool constant_;
};

class Function : public Type {
 public:
  Function(node_ptr<Result> result, const parameter_list& args,
           const Location& l = Location::None);

  /// Create a wildcard function type.
  explicit Function(const Location& l = Location::None);

  virtual ~Function();

  node_ptr<Result> result() { return result_; }

  parameter_list args() const;

  std::string render() override;

  ACCEPT_VISITOR(Type)

 private:
  node_ptr<Result> result_;
  std::list<node_ptr<Parameter>> args_;
};

}  // namespace function
}  // namespace type
}  // namespace ast
}  // namespace spec
}  // namespace diffingo

#endif  // SRC_SPEC_AST_TYPE_FUNCTION_H_
