/*
 * bitfield.h
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

#ifndef SRC_SPEC_AST_TYPE_BITFIELD_H_
#define SRC_SPEC_AST_TYPE_BITFIELD_H_

#include <list>
#include <memory>
#include <string>

#include "spec/ast/location.h"
#include "spec/ast/node.h"
#include "spec/ast/type/type.h"
#include "spec/ast/visitor.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace type {
namespace bitfield {

class Bits : public Node {
 public:
  Bits(node_ptr<ID> id, int lower, int upper, int parent_width,
       const attribute_list& attributes = attribute_list(),
       const Location& l = Location::None);
  virtual ~Bits();

  node_ptr<ID> id() const { return id_; }

  int lower() const { return lower_; }

  int upper() const { return upper_; }

  int parent_width() const { return parent_width_; }

  node_ptr<AttributeMap> attributes() const { return attributes_; }

  std::string render() override;

  ACCEPT_VISITOR(Node)

 private:
  node_ptr<ID> id_;
  int lower_;
  int upper_;
  int parent_width_;
  node_ptr<AttributeMap> attributes_;
};

class Bitfield : public Type {
 public:
  typedef std::list<node_ptr<Bits>> bits_list;

  Bitfield(int width, const bits_list& bits,
           const Location& l = Location::None);

  /// Create a wildcard Bitfield type.
  explicit Bitfield(const Location& l = Location::None);

  virtual ~Bitfield();

  int width() const { return width_; }

  bits_list bits() const;

  ssize_t static_serialized_length() override;

  std::string render() override;

  ACCEPT_VISITOR(Type)

 private:
  int width_;
  std::list<node_ptr<Bits>> bits_;
};

}  // namespace bitfield
}  // namespace type
}  // namespace ast
}  // namespace spec
}  // namespace diffingo

#endif  // SRC_SPEC_AST_TYPE_BITFIELD_H_
