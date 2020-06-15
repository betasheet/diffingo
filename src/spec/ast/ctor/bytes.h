/*
 * bytes.h
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

#ifndef SRC_SPEC_AST_CTOR_BYTES_H_
#define SRC_SPEC_AST_CTOR_BYTES_H_

#include <string>

#include "spec/ast/ctor/ctor.h"
#include "spec/ast/location.h"
#include "spec/ast/visitor.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace ctor {

class Bytes : public ctor::Ctor {
 public:
  explicit Bytes(const std::string& value,
                 const Location& l = Location::None);
  virtual ~Bytes();

  virtual node_ptr<type::Type> type() const {
    return type_;
  }

  const std::string& value() const {
    return value_;
  }

  int length() const {
    return value_.length();
  }

  std::string render() override;

  ACCEPT_VISITOR(Ctor)

 private:
  std::string value_;
  node_ptr<type::Type> type_;
};

}  // namespace ctor
}  // namespace ast
}  // namespace spec
}  // namespace diffingo

#endif  // SRC_SPEC_AST_CTOR_BYTES_H_