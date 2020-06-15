/*
 * container.cpp
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

#include "spec/ast/type/container.h"

#include <memory>
#include <string>

#include "spec/ast/type/type.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace type {

Container::Container(const Location& l) : Type(l) {}

Container::~Container() {}

List::List(node_ptr<Type> element_type, const Location& l)
    : Container(l), element_type_(element_type) {
  addChild(element_type_);
}

List::List(const Location& l) : Container(l) { setWildcard(true); }

List::~List() {}

std::string List::render() {
  std::string s = "LIST ";
  if (element_type_) s += "(elements: " + element_type_->render() + ") ";
  s += Type::render();
  return s;
}

Vector::Vector(node_ptr<Type> element_type, const Location& l)
    : Container(l), element_type_(element_type) {
  addChild(element_type_);
}

Vector::Vector(const Location& l) : Container(l) { setWildcard(true); }

Vector::~Vector() {}

std::string Vector::render() {
  std::string s = "VECTOR ";
  if (element_type_) s += "(elements: " + element_type_->render() + ") ";
  s += Type::render();
  return s;
}

Map::Map(node_ptr<Type> key, node_ptr<Type> value,
         const Location& l)
    : Container(l), key_type_(key), value_type_(value) {
  addChild(key_type_);
  addChild(value_type_);
}

Map::Map(const Location& l) : Container(l) { setWildcard(true); }

Map::~Map() {}

std::string Map::render() {
  std::string s = "MAP ";
  if (key_type_ && value_type_)
    s += "(keys: " + key_type_->render() + ", values: " +
         value_type_->render() + ") ";
  s += Type::render();
  return s;
}

Set::Set(node_ptr<Type> element_type, const Location& l)
    : Container(l), element_type_(element_type) {
  addChild(element_type_);
}

Set::Set(const Location& l) : Container(l) { setWildcard(true); }

Set::~Set() {}

std::string Set::render() {
  std::string s = "SET ";
  if (element_type_) s += "(elements: " + element_type_->render() + ") ";
  s += Type::render();
  return s;
}

}  // namespace type
}  // namespace ast
}  // namespace spec
}  // namespace diffingo
