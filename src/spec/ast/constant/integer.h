/*
 * integer.h
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

#ifndef SRC_SPEC_AST_CONSTANT_INTEGER_H_
#define SRC_SPEC_AST_CONSTANT_INTEGER_H_

#include <cstdint>
#include <string>

#include "spec/ast/constant/constant.h"
#include "spec/ast/location.h"
#include "spec/ast/visitor.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace constant {

class Integer : public SpecificConstant<int64_t> {
 public:
  Integer(int64_t value, int width, bool sign,
          const Location& l = Location::None);
  virtual ~Integer();

  std::string valueAsString() const override { return std::to_string(value()); }

  ACCEPT_VISITOR(Constant)
};

}  // namespace constant
}  // namespace ast
}  // namespace spec
}  // namespace diffingo

#endif  // SRC_SPEC_AST_CONSTANT_INTEGER_H_
