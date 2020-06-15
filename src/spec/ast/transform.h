/*
 * transform.h
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

#ifndef SRC_SPEC_AST_TRANSFORM_H_
#define SRC_SPEC_AST_TRANSFORM_H_

#include <string>

#include "spec/ast/function.h"
#include "spec/ast/id.h"
#include "spec/ast/location.h"
#include "spec/ast/node.h"
#include "spec/ast/type/type.h"

namespace diffingo {
namespace spec {
namespace ast {

class Transform : public Node {
 public:
  explicit Transform(node_ptr<ID> id, node_ptr<type::Type> wire_type,
                     node_ptr<type::Type> internal_type,
                     node_ptr<Function> decode, node_ptr<Function> encode,
                     const Location& l = Location::None);
  virtual ~Transform();

  node_ptr<Function> decode() const { return decode_; }
  node_ptr<Function> encode() const { return encode_; }
  node_ptr<ID> id() const { return id_; }
  node_ptr<type::Type> internal_type() const { return internal_type_; }
  node_ptr<type::Type> wire_type() const { return wire_type_; }

  std::string render() override;

 private:
  node_ptr<ID> id_;
  node_ptr<type::Type> wire_type_;
  node_ptr<type::Type> internal_type_;
  node_ptr<Function> decode_;
  node_ptr<Function> encode_;
};

}  // namespace ast
}  // namespace spec
}  // namespace diffingo

#endif  // SRC_SPEC_AST_TRANSFORM_H_
