/*
 * container.h
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

#ifndef SRC_SPEC_AST_TYPE_CONTAINER_H_
#define SRC_SPEC_AST_TYPE_CONTAINER_H_

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

class Container : public Type {
 public:
  explicit Container(const Location& l = Location::None);
  virtual ~Container();

  ACCEPT_VISITOR(Type)
};

class List : public Container {
 public:
  explicit List(node_ptr<Type> element_type,
                const Location& l = Location::None);

  /// Create a wildcard list type.
  explicit List(const Location& l = Location::None);

  virtual ~List();

  node_ptr<Type> element_type() const { return element_type_; }

  std::string render() override;

  ACCEPT_VISITOR(Container)

 private:
  node_ptr<Type> element_type_;
};

class Vector : public Container {
 public:
  explicit Vector(node_ptr<Type> element_type,
                  const Location& l = Location::None);

  /// Create a wildcard vector type.
  explicit Vector(const Location& l = Location::None);

  virtual ~Vector();

  node_ptr<Type> element_type() const { return element_type_; }

  std::string render() override;

  ACCEPT_VISITOR(Container)

 private:
  node_ptr<Type> element_type_;
};

class Map : public Container {
 public:
  Map(node_ptr<Type> key, node_ptr<Type> value,
      const Location& l = Location::None);

  /// Create a wildcard map type.
  explicit Map(const Location& l = Location::None);

  virtual ~Map();

  node_ptr<Type> key_type() const { return key_type_; }

  node_ptr<Type> value_type() const { return value_type_; }

  std::string render() override;

  ACCEPT_VISITOR(Container)

 private:
  node_ptr<Type> key_type_;
  node_ptr<Type> value_type_;
};

class Set : public Container {
 public:
  explicit Set(node_ptr<Type> element_type,
               const Location& l = Location::None);

  /// Create a wildcard set type.
  explicit Set(const Location& l = Location::None);

  virtual ~Set();

  node_ptr<Type> element_type() const { return element_type_; }

  std::string render() override;

  ACCEPT_VISITOR(Container)

 private:
  node_ptr<Type> element_type_;
};

}  // namespace type
}  // namespace ast
}  // namespace spec
}  // namespace diffingo

#endif  // SRC_SPEC_AST_TYPE_CONTAINER_H_
