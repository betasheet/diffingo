/*
 * enum.h
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

#ifndef SRC_SPEC_AST_CONSTANT_ENUM_H_
#define SRC_SPEC_AST_CONSTANT_ENUM_H_

#include "spec/ast/type/enum.h"

#include "spec/ast/constant/constant.h"
#include "spec/ast/id.h"
#include "spec/ast/location.h"
#include "spec/ast/node.h"
#include "spec/ast/visitor.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace constant {

class Enum : public Constant {
 public:
  Enum(node_ptr<ID> label, node_ptr<type::Type> type,
       const Location& l = Location::None);
  virtual ~Enum();

  /// Returns the enum label.
  node_ptr<ID> label() const { return label_; }

  /// Returns the enum's type.
  node_ptr<type::Type> type() const override { return type_; }

  ACCEPT_VISITOR(Constant)

 private:
  node_ptr<ID> label_;
  node_ptr<type::Enum> type_;
};

}  // namespace constant
}  // namespace ast
}  // namespace spec
}  // namespace diffingo

#endif  // SRC_SPEC_AST_CONSTANT_ENUM_H_
