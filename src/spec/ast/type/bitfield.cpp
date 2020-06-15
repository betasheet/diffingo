/*
 * bitfield.cpp
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

#include "spec/ast/type/bitfield.h"

#include <list>
#include <memory>
#include <string>

#include "spec/ast/attribute.h"
#include "spec/ast/id.h"
#include "spec/ast/node.h"
#include "spec/ast/type/type.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace type {
namespace bitfield {

Bits::Bits(node_ptr<ID> id, int lower, int upper, int parent_width,
           const attribute_list& attributes, const Location& l)
    : Node(l),
      id_(id),
      lower_(lower),
      upper_(upper),
      parent_width_(parent_width),
      attributes_(std::make_shared<AttributeMap>(attributes)) {
  addChild(id_);
  addChild(attributes_);
}

Bits::~Bits() {}

std::string Bits::render() {
  return id_->render() + " : " + std::to_string(lower_) + ".." +
         std::to_string(upper_);
}

Bitfield::Bitfield(int width, const bits_list& bits, const Location& l)
    : Type(l), width_(width) {
  for (auto b : bits) bits_.push_back(b);
  for (auto b : bits_) addChild(b);
}

Bitfield::Bitfield(const Location& l) : Type(l), width_(0) {
  setWildcard(true);
}

Bitfield::~Bitfield() {}

Bitfield::bits_list Bitfield::bits() const {
  bits_list bits;
  for (auto b : bits_) bits.push_back(b);
  return bits;
}

ssize_t Bitfield::static_serialized_length() { return width_; }

std::string Bitfield::render() {
  std::string s = "BITFIELD(" + std::to_string(width_) + ") ";
  s += Type::render();
  return s;
}

}  // namespace bitfield
}  // namespace type
}  // namespace ast
}  // namespace spec
}  // namespace diffingo
