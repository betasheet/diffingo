/*
 * attribute.h
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

#ifndef SRC_SPEC_AST_ATTRIBUTE_H_
#define SRC_SPEC_AST_ATTRIBUTE_H_

#include <stddef.h>
#include <list>
#include <map>
#include <memory>
#include <string>

#include "spec/ast/expression/expression.h"
#include "spec/ast/location.h"
#include "spec/ast/node.h"
#include "spec/ast/visitor.h"

namespace diffingo {
namespace spec {
namespace ast {

class Attribute : public Node {
 public:
  Attribute(const std::string& key,
            node_ptr<expression::Expression> value = nullptr,
            bool internal = false, const Location& l = Location::None);
  virtual ~Attribute();

  const std::string& key() const { return key_; }

  node_ptr<expression::Expression> value() const { return value_; }

  int internal() const { return internal_; }

  /// Returns true if the property names match.
  bool operator==(const Attribute& other) const;

  node_ptr<Attribute> clone() const;

  std::string render() override;

  ACCEPT_VISITOR(Node)

 private:
  std::string key_;
  node_ptr<expression::Expression> value_;
  bool internal_;
};

/// A collection of attributes.
class AttributeMap : public Node {
 public:
  explicit AttributeMap(const Location& l = Location::None);

  explicit AttributeMap(const attribute_list& attrs,
                        const Location& l = Location::None);

  virtual ~AttributeMap();

  /// Inserts an attribute into the map. If the same key already exists,
  /// it's overwritten.
  ///
  /// Returns: The previous attribute with the same key, if any.
  node_ptr<Attribute> add(node_ptr<Attribute> attr);

  /// Removes an attribute from the map. Returns the old attribute, or null
  /// if none.
  node_ptr<Attribute> remove(const std::string& key);

  /// Returns true if the map contains an attribute of the given key.
  bool has(const std::string& key) const;

  /// Returns an attribute of a given key, or null if that key doesn't
  /// exist.
  node_ptr<Attribute> lookup(const std::string& key) const;

  /// Returns a list of all attributes.
  std::list<node_ptr<Attribute>> attributes() const;

  std::list<node_ptr<Attribute>> clonedAttributes() const;

  /// Returns the number of attributes defined.
  size_t size() const;

  AttributeMap& operator=(const attribute_list& attrs);

  std::string render() override;

  ACCEPT_VISITOR(Node);

 private:
  std::map<std::string, node_ptr<Attribute>> attrs_;
};

}  // namespace ast
}  // namespace spec
}  // namespace diffingo

#endif  // SRC_SPEC_AST_ATTRIBUTE_H_
