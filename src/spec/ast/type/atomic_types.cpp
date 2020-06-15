/*
 * atomic_types.cpp
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

#include "spec/ast/type/atomic_types.h"

#include <list>
#include <memory>
#include <string>

#include "spec/ast/expression/expression.h"
#include "spec/ast/id.h"
#include "spec/ast/node.h"
#include "spec/ast/type/type.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace type {

Any::Any(const Location& l) : Type(l) {}

Any::~Any() {}

std::string Any::render() {
  std::string s = "ANY ";
  s += Type::render();
  return s;
}

Unknown::Unknown(node_ptr<ID> id, const Location& l) : Type(l), id_(id) {
  addChild(id_);
}

Unknown::Unknown(const Location& l) : Type(l) { setWildcard(true); }

Unknown::~Unknown() {}

std::string Unknown::render() {
  std::string s = "UNKNOWN ";
  if (id_) s += id_->render() + " ";
  s += Type::render();
  return s;
}

UnknownElementType::UnknownElementType(node_ptr<expression::Expression> expr,
                                       const Location& l)
    : Type(l), expr_(expr) {
  addChild(expr_);
}

UnknownElementType::~UnknownElementType() {}

std::string UnknownElementType::render() {
  std::string s = "UNKNOWN ELEMENT ";
  s += expr_->render() + " ";
  s += Type::render();
  return s;
}

Bool::Bool(const Location& l) : Type(l) {}

Bool::~Bool() {}

ssize_t Bool::static_serialized_length() {
  if (wildcard()) return -1;

  // TODO(ES): support serialized representation of bools
  return 1;
}

std::string Bool::render() {
  std::string s = "BOOL ";
  s += Type::render();
  return s;
}

Bytes::Bytes(const Location& l) : Type(l) {}

Bytes::~Bytes() {}

std::string Bytes::render() {
  std::string s = "BYTES ";
  s += Type::render();
  return s;
}

CAddr::CAddr(const Location& l) : Type(l) {}

CAddr::~CAddr() {}

ssize_t CAddr::static_serialized_length() {
  if (wildcard()) return -1;
  // TODO(ES): support serialized representation of caddrs?
  return sizeof(char*);
}

std::string CAddr::render() {
  std::string s = "CADDR ";
  s += Type::render();
  return s;
}

Double::Double(const Location& l) : Type(l) {}

Double::~Double() {}

ssize_t Double::static_serialized_length() {
  if (wildcard()) return -1;
  // TODO(ES): support serialized representation of doubles
  return sizeof(double);
}

std::string Double::render() {
  std::string s = "DOUBLE ";
  s += Type::render();
  return s;
}

Sink::Sink(const Location& l) : Type(l) {}

Sink::~Sink() {}

std::string Sink::render() {
  std::string s = "SINK ";
  s += Type::render();
  return s;
}

String::String(const Location& l) : Type(l) {}

String::~String() {}

std::string String::render() {
  std::string s = "STRING ";
  s += Type::render();
  return s;
}

Void::Void(const Location& l) : Type(l) {}

Void::~Void() {}

ssize_t Void::static_serialized_length() { return 0; }

std::string Void::render() {
  std::string s = "VOID ";
  s += Type::render();
  return s;
}

Integer::Integer(int width, bool signed__, const Location& l)
    : Type(l), width_(width), signed_(signed__) {}

Integer::Integer(const Location& l) : Integer(0, true, l) { setWildcard(true); }

Integer::~Integer() {}

ssize_t Integer::static_serialized_length() {
  if (wildcard()) return -1;
  // TODO(ES): support bit-size widths
  return width_ / 8;
}

std::string Integer::render() {
  std::string s = signed_ ? "SIGNED INT" : "UNSIGNED INT";
  s += std::to_string(width_) + " " + Type::render();
  return s;
}

Tuple::Tuple(const type_list& types, const Location& l) : Type(l) {
  for (auto t : types) types_.push_back(t);

  for (auto t : types_) addChild(t);
}

Tuple::Tuple(const Location& l) : Type(l) { setWildcard(true); }

Tuple::~Tuple() {}

ssize_t Tuple::static_serialized_length() {
  if (wildcard()) return -1;

  ssize_t len = 0;
  for (auto t : types_) {
    auto t_len = t->static_serialized_length();
    if (t_len < 0) return -1;
    len += t_len;
  }
  return len;
}

std::string Tuple::render() {
  std::string s = "TUPLE (";
  bool first = true;
  for (auto t : types_) {
    if (first) {
      first = false;
    } else {
      s += ", ";
    }
    s += t->render();
  }
  s += ") " + Type::render();
  return s;
}

type_list Tuple::types() const {
  type_list types;

  for (auto t : types_) types.push_back(t);

  return types;
}

MemberAttribute::MemberAttribute(node_ptr<ID> attr, const Location& l)
    : Type(l), attr_(attr) {
  addChild(attr_);
}

MemberAttribute::MemberAttribute(const Location& l) : Type(l) {
  setWildcard(true);
}

MemberAttribute::~MemberAttribute() {}

std::string MemberAttribute::render() {
  std::string s = "MEMBER_ATTRIBUTE ";
  if (attr_) s += attr_->render() + " ";
  s += Type::render();
  return s;
}

TypeType::TypeType(node_ptr<type::Type> type, const Location& l)
    : Type(l), type_(type) {
  addChild(type_);
}

TypeType::TypeType(const Location& l) : Type(l) { setWildcard(true); }

TypeType::~TypeType() {}

std::string TypeType::render() {
  std::string s = "TYPE ";
  if (type_) s += type_->render() + " ";
  s += Type::render();
  return s;
}

}  // namespace type
}  // namespace ast
}  // namespace spec
}  // namespace diffingo
