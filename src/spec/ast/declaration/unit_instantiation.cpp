/*
 * instantiation.cpp
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

#include <list>
#include <memory>
#include <string>

#include "spec/ast/attribute.h"
#include "spec/ast/declaration/declaration.h"
#include "spec/ast/declaration/unit_instantiation.h"
#include "spec/ast/expression/expression.h"
#include "spec/ast/id.h"
#include "spec/ast/node.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace declaration {
namespace unit_instantiation {

Instantiation::Instantiation(node_ptr<ID> id, node_ptr<ID> unit_id,
                             inst_item_list items, const Location& l)
    : Declaration(id, Linkage::EXPORTED, l), unit_id_(unit_id), items_(items) {
  addChild(unit_id_);
  for (auto i : items_) {
    addChild(i);
  }
}

Instantiation::~Instantiation() {}

std::string Instantiation::render() {
  return util::fmt("instantiate unit %s as %s (%d items)", unit_id_->render(),
                   id()->render(), items_.size());
}

Item::Item(node_ptr<ID> id, const attribute_list& attrs, bool dependency,
           const Location& l)
    : Node(l),
      id_(id),
      attributes_(
          node_ptr<AttributeMap>(std::make_shared<AttributeMap>(attrs))),
      dependency_(dependency) {
  addChild(id_);
  addChild(attributes_);
}

Item::~Item() {}

std::string Item::render() {
  std::string s = "";
  if (id_) s += id_->render();
  return s;
}

Property::Property(node_ptr<Attribute> prop, const Location& l)
    : Item(node_ptr<ID>(std::make_shared<ID>("%" + prop->key())),
           attribute_list(), false, l),
      property_(prop) {
  addChild(property_);
}

Property::~Property() {}

std::string Property::render() {
  auto s = Item::render();
  s += " = " + property_->value()->render();
  return property_->value()->render();
}

Field::Field(node_ptr<ID> id, const attribute_list& attrs, bool dependency,
             const Location& l)
    : Item(id, attrs, dependency, l) {}

Field::~Field() {}

std::string Field::render() { return Item::render(); }

}  // namespace unit_instantiation
}  // namespace declaration
}  // namespace ast
}  // namespace spec
}  // namespace diffingo
