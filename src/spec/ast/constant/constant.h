/*
 * constant.h
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

#ifndef SRC_SPEC_AST_CONSTANT_CONSTANT_H_
#define SRC_SPEC_AST_CONSTANT_CONSTANT_H_

#include <memory>
#include <string>

#include "spec/ast/exception.h"
#include "spec/ast/location.h"
#include "spec/ast/node.h"
#include "spec/ast/type/type.h"
#include "spec/ast/visitor.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace constant {

class Constant : public Node {
 public:
  explicit Constant(const Location& l = Location::None);
  virtual ~Constant();

  virtual node_ptr<type::Type> type() const = 0;

  virtual ssize_t static_serialized_length();

  std::string render() override;

  ACCEPT_VISITOR(Node)
};

/// Exception thrown when a constant cannot be parsed into the internal
/// representation.
class ConstantParseError : public RuntimeError {
 public:
  ConstantParseError(Constant* c, const std::string& msg)
      : RuntimeError(msg, c) {}
};

template <typename Value>
class SpecificConstant : public Constant {
 public:
  SpecificConstant(Value value, node_ptr<type::Type> type,
                   const Location& l = Location::None)
      : Constant(l), type_(type), value_(value) {
    this->addChild(type_);
  }

  virtual ~SpecificConstant() {}

  /// Returns the constant's value.
  Value value() const { return value_; }

  /// Returns the constant's type.
  node_ptr<type::Type> type() const override { return type_; }

  std::string render() override {
    return valueAsString() + " " + Constant::render();
  }

  virtual std::string valueAsString() const = 0;

 private:
  node_ptr<type::Type> type_;
  Value value_;
};

}  // namespace constant
}  // namespace ast
}  // namespace spec
}  // namespace diffingo

#endif  // SRC_SPEC_AST_CONSTANT_CONSTANT_H_
