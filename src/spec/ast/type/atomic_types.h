/*
 * atomic_types.h
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

#ifndef SRC_SPEC_AST_TYPE_ATOMIC_TYPES_H_
#define SRC_SPEC_AST_TYPE_ATOMIC_TYPES_H_

#include <list>
#include <memory>
#include <string>

#include "spec/ast/id.h"
#include "spec/ast/location.h"
#include "spec/ast/node.h"
#include "spec/ast/type/type.h"
#include "spec/ast/visitor.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace type {

class Any : public type::Type {
 public:
  explicit Any(const Location& l = Location::None);
  virtual ~Any();

  std::string render() override;

  ACCEPT_VISITOR(Type)
};

class Unknown : public type::Type {
 public:
  explicit Unknown(const Location& l = Location::None);
  explicit Unknown(node_ptr<ID> id, const Location& l = Location::None);
  virtual ~Unknown();

  node_ptr<ID> id() const { return id_; }

  std::string render() override;

  ACCEPT_VISITOR(Type)

 private:
  node_ptr<ID> id_ = nullptr;
};

class UnknownElementType : public type::Type {
 public:
  explicit UnknownElementType(node_ptr<expression::Expression> expr,
                              const Location& l = Location::None);
  virtual ~UnknownElementType();

  node_ptr<expression::Expression> expression() const { return expr_; }

  std::string render() override;

  ACCEPT_VISITOR(Type)

 private:
  node_ptr<expression::Expression> expr_ = nullptr;
};

class Bool : public type::Type {
 public:
  explicit Bool(const Location& l = Location::None);
  virtual ~Bool();

  ssize_t static_serialized_length() override;

  std::string render() override;

  ACCEPT_VISITOR(Type)
};

class Bytes : public type::Type {
 public:
  explicit Bytes(const Location& l = Location::None);
  virtual ~Bytes();

  std::string render() override;

  ACCEPT_VISITOR(Type)
};

class CAddr : public type::Type {
 public:
  explicit CAddr(const Location& l = Location::None);
  virtual ~CAddr();

  ssize_t static_serialized_length() override;

  std::string render() override;

  ACCEPT_VISITOR(Type)
};

class Double : public type::Type {
 public:
  explicit Double(const Location& l = Location::None);
  virtual ~Double();

  ssize_t static_serialized_length() override;

  std::string render() override;

  ACCEPT_VISITOR(Type)
};

class Sink : public type::Type {
 public:
  explicit Sink(const Location& l = Location::None);
  virtual ~Sink();

  std::string render() override;

  ACCEPT_VISITOR(Type)
};

class String : public type::Type {
 public:
  explicit String(const Location& l = Location::None);
  virtual ~String();

  std::string render() override;

  ACCEPT_VISITOR(Type)
};

class Void : public type::Type {
 public:
  explicit Void(const Location& l = Location::None);
  virtual ~Void();

  ssize_t static_serialized_length() override;

  std::string render() override;

  ACCEPT_VISITOR(Type)
};

class Integer : public type::Type {
 public:
  Integer(int width, bool signed_, const Location& l = Location::None);

  /// Create an integer type matching any other integer type.
  explicit Integer(const Location& l = Location::None);

  virtual ~Integer();

  bool _signed() const { return signed_; }

  int width() const { return width_; }

  ssize_t static_serialized_length() override;

  std::string render() override;

  ACCEPT_VISITOR(Type)

 private:
  int width_;
  bool signed_;
};

class Tuple : public type::Type {
 public:
  explicit Tuple(const type_list& types, const Location& l = Location::None);

  /// Create a wildcard tuple type.
  explicit Tuple(const Location& l = Location::None);

  virtual ~Tuple();

  type_list types() const;

  std::string render() override;

  ssize_t static_serialized_length() override;

  ACCEPT_VISITOR(Type)

 private:
  std::list<node_ptr<type::Type>> types_;
};

class MemberAttribute : public type::Type {
 public:
  explicit MemberAttribute(node_ptr<ID> attr,
                           const Location& l = Location::None);

  /// Create a wildcard memberattr type.
  explicit MemberAttribute(const Location& l = Location::None);

  virtual ~MemberAttribute();

  node_ptr<ID> attr() const { return attr_; }

  std::string render() override;

  ACCEPT_VISITOR(Type)

 private:
  node_ptr<ID> attr_;
};

class TypeType : public type::Type {
 public:
  explicit TypeType(node_ptr<type::Type> type,
                    const Location& l = Location::None);

  /// Create a wildcard type type.
  explicit TypeType(const Location& l = Location::None);

  virtual ~TypeType();

  node_ptr<type::Type> type() const { return type_; }

  std::string render() override;

  ACCEPT_VISITOR(Type)

 private:
  node_ptr<type::Type> type_;
};

}  // namespace type
}  // namespace ast
}  // namespace spec
}  // namespace diffingo

#endif  // SRC_SPEC_AST_TYPE_ATOMIC_TYPES_H_
