/*
 * unit.cpp
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

#include "spec/ast/type/unit.h"

#include <sys/types.h>
#include <cassert>
#include <list>
#include <memory>
#include <string>
#include <utility>

#include "spec/ast/attribute.h"
#include "spec/ast/constant/constant.h"
#include "spec/ast/constant/integer.h"
#include "spec/ast/ctor/ctor.h"
#include "spec/ast/expression/constant.h"
#include "spec/ast/expression/expression.h"
#include "spec/ast/expression/operator.h"
#include "spec/ast/id.h"
#include "spec/ast/module.h"
#include "spec/ast/node.h"
#include "spec/ast/scope.h"
#include "spec/ast/type/atomic_types.h"
#include "spec/ast/type/container.h"
#include "spec/ast/type/type.h"
#include "util/util.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace type {
namespace unit {

Unit::Unit(const parameter_list& params, const unit_item_list& items,
           const Location& l)
    : Type(l), scope_(std::make_shared<Scope>()) {
  for (auto p : params) parameters_.push_back(p);
  for (auto p : parameters_) addChild(p);

  for (auto i : items) items_.push_back(i);
  for (auto i : items_) {
    i->set_unit(this);
    addChild(i);
  }
}

Unit::Unit(const Location& l) : Type(l) { setWildcard(true); }

Unit::~Unit() {}

static void _flatten(node_ptr<item::Item> i, unit_item_list* dst) {
  // If it's a switch, descend.
  auto switch_ = ast::tryCast<item::field::switch_::Switch>(i);

  if (switch_) {
    for (auto c : switch_->cases())
      for (auto f : c->items()) _flatten(f, dst);
  }

  dst->push_back(i);
}

unit_item_list Unit::flattenedItems() const {
  unit_item_list items;

  for (auto i : items_) _flatten(i, &items);

  return items;
}

std::list<node_ptr<item::field::Field>> Unit::fields() const {
  std::list<node_ptr<item::field::Field>> m;

  for (auto i : items_) {
    auto f = ast::tryCast<item::field::Field>(i);

    if (f) m.push_back(f);
  }

  return m;
}

std::list<node_ptr<item::field::Field>> Unit::flattenedFields() const {
  std::list<node_ptr<item::field::Field>> m;

  for (auto i : flattenedItems()) {
    auto f = ast::tryCast<item::field::Field>(i);

    if (f) m.push_back(f);
  }

  return m;
}

std::list<node_ptr<item::Variable>> Unit::variables() const {
  std::list<node_ptr<item::Variable>> m;

  for (auto i : items_) {
    auto f = ast::tryCast<item::Variable>(i);

    if (f) m.push_back(f);
  }

  return m;
}

std::list<node_ptr<item::Property>> Unit::properties() const {
  std::list<node_ptr<item::Property>> m;

  for (auto i : items_) {
    auto f = ast::tryCast<item::Property>(i);

    if (f) m.push_back(f);
  }

  return m;
}

std::list<node_ptr<item::Property>> Unit::properties(
    const std::string& name) const {
  Attribute pp(name);

  std::list<node_ptr<item::Property>> m;

  for (auto i : items_) {
    auto f = ast::tryCast<item::Property>(i);

    if (!f) continue;

    assert(f->property());

    if (pp == *f->property()) m.push_back(f);
  }

  return m;
}

node_ptr<item::Property> Unit::property(const std::string& prop) const {
  auto all = properties(prop);
  return all.size() > 0 ? all.front() : nullptr;
}

node_ptr<item::Item> Unit::item(node_ptr<ID> id) const {
  return item(id->name());
}

node_ptr<item::Item> Unit::item(const std::string& name) const {
  for (auto i : flattenedItems()) {
    if (i->id() && i->id()->name() == name) return i;
  }

  return nullptr;
}

std::pair<node_ptr<item::Item>, std::string> Unit::path(
    const std::string& _path) {
  return path(util::strsplit(_path, "."), this->nodePtr<Unit>());
}

std::pair<node_ptr<item::Item>, std::string> Unit::path(
    string_list _path, node_ptr<Unit> current) {
  auto next = current->item(_path.front());

  if (!next) return std::make_pair(nullptr, ::util::strjoin(_path, "."));

  _path.pop_front();

  if (!_path.size()) return std::make_pair(next, "");

  auto child = ast::tryCast<item::field::Field>(next);

  if (!child) return std::make_pair(next, ::util::strjoin(_path, "."));

  auto unit = ast::tryCast<unit::Unit>(child->type());

  if (!unit) return std::make_pair(next, ::util::strjoin(_path, "."));

  return path(_path, unit);
}

node_ptr<expression::Expression> Unit::inheritedProperty(
    const std::string& pname, node_ptr<item::Item> item) {
  node_ptr<expression::Expression> expr = nullptr;

  node_ptr<Attribute> item_property;
  node_ptr<Attribute> unit_property;
  node_ptr<Attribute> module_property;

  auto unit_prop_item = property(pname);
  unit_property = unit_prop_item ? unit_prop_item->property() : nullptr;

  if (module_) module_property = module_->property(pname);

  if (item && item->attributes()->has(pname))
    item_property = item->attributes()->lookup(pname);

  // Take most specific one.

  if (item_property) return item_property->value();

  if (unit_property) return unit_property->value();

  if (module_property) return module_property->value();

  return nullptr;
}

ssize_t Unit::static_serialized_length() {
  if (wildcard()) return -1;

  ssize_t len = 0;
  for (auto f : fields()) {
    auto f_len = f->static_serialized_length();
    if (f_len < 0) return -1;
    len += f_len;
  }
  return len;
}

node_ptr<expression::Expression> Unit::serialized_length() {
  if (wildcard()) return nullptr;

  node_ptr<expression::Expression> len;
  for (auto f : fields()) {
    auto f_len = f->serialized_length();
    if (!f_len) return nullptr;

    if (len) {
      expression_list ops;
      ops.push_back(len);
      ops.push_back(f_len);
      len = ast::newNodePtr(std::make_shared<expression::Operator>(
          expression::Operator::Kind::Plus, ops));
    } else {
      len = f_len;
    }
  }
  return len;
}

std::string Unit::render() {
  std::string s = "UNIT ";
  s += "(" + std::to_string(parameters_.size()) + " params, " +
       std::to_string(items_.size()) + " items) ";
  s += Type::render();
  return s;
}

namespace item {

int Item::id_counter_ = 0;

Item::Item(node_ptr<ID> id, const node_ptr<Type> type,
           const attribute_list& attrs, const Location& l)
    : Node(l),
      id_(id),
      type_(type),
      attributes_(
          node_ptr<AttributeMap>(std::make_shared<AttributeMap>(attrs))),
      scope_(std::make_shared<Scope>()) {
  if (!id_) {
    id_ = node_ptr<ID>(
        std::make_shared<ID>(util::fmt("__anon%d", ++id_counter_), l));
    anonymous_ = true;
  }

  addChild(id_);
  addChild(type_);
  addChild(attributes_);
}

Item::~Item() {}

node_ptr<expression::Expression> Item::inheritedProperty(
    const std::string& property) {
  if (!unit_) return nullptr;

  return unit_->inheritedProperty(property, this->nodePtr<Item>());
}

std::string Item::render() {
  std::string s = "";
  if (id_) s += id_->render();
  if (type_) s += " : " + type_->render();
  return s;
}

Variable::Variable(node_ptr<ID> id, node_ptr<Type> type,
                   node_ptr<expression::Expression> default_,
                   const attribute_list& attrs, const Location& l)
    : Item(id, type, attrs, l), default_value_(default_) {
  addChild(default_value_);
}

Variable::~Variable() {}

node_ptr<Item> Variable::clone() {
  auto def = default_value() ? default_value()->clone() : nullptr;
  auto v = newNodePtr(std::make_shared<Variable>(
      id(), type(), def, attributes()->clonedAttributes(), location()));
  v->set_parsing_only(parsing_only());
  return v;
}

std::string Variable::render() {
  std::string s = Item::render();
  if (default_value_) s += " = " + default_value_->render();
  return s;
}

Property::Property(node_ptr<Attribute> prop, const Location& l)
    : Item(node_ptr<ID>(std::make_shared<ID>("%" + prop->key())), nullptr,
           attribute_list(), l),
      property_(prop) {
  addChild(property_);
}

Property::~Property() {}

node_ptr<Item> Property::clone() {
  return newNodePtr(
      std::make_shared<Property>(property()->clone(), location()));
}

std::string Property::render() {
  auto s = Item::render();
  s += " = " + property_->value()->render();
  return s;
}

namespace field {

Field::Field(node_ptr<ID> id, node_ptr<type::Type> type,
             node_ptr<expression::Expression> cond, const attribute_list& attrs,
             const expression_list& params, const expression_list& sinks,
             const Location& l)
    : Item(id, type, attrs, l), condition_(cond) {
  for (auto s : params) parameters_.push_back(s);
  for (auto s : parameters_) addChild(s);

  for (auto s : sinks) sinks_.push_back(s);
  for (auto s : sinks_) addChild(s);
}

Field::~Field() {}

ssize_t Field::static_serialized_length() { return -1; }

node_ptr<expression::Expression> Field::serialized_length() {
  auto static_len = static_serialized_length();
  if (static_len >= 0) {
    auto c = ast::newNodePtr(std::make_shared<constant::Integer>(
        static_len, sizeof(static_len) * 4, true));
    return ast::newNodePtr(std::make_shared<expression::Constant>(c));
  }
  return nullptr;
}

std::string Field::render() {
  auto s = Item::render();
  auto s_value = renderValue();
  if (s_value.length() > 0) s += " = " + s_value;

  if (!sinks_.empty()) {
    s += " -> ";
    bool first = true;
    for (auto sink : sinks_) {
      if (first)
        first = false;
      else
        s += ", ";
      s += sink->render();
    }
  }
  return s;
}

node_ptr<Field> Field::createByType(node_ptr<Type> type, node_ptr<ID> id,
                                    node_ptr<expression::Expression> cond,
                                    const attribute_list& attrs,
                                    const expression_list& params,
                                    const expression_list& sinks,
                                    const Location& l) {
  if (auto unit = tryCast<unit::Unit>(type))
    return node_ptr<Field>(
        std::make_shared<Unit>(id, unit, cond, attrs, params, sinks, l));

  if (auto list = ast::tryCast<type::List>(type)) {
    auto field =
        createByType(list->element_type(), nullptr, nullptr, attribute_list(),
                     expression_list(), expression_list(), l);
    return node_ptr<Field>(
        std::make_shared<container::List>(id, field, cond, attrs, sinks, l));
  }

  return node_ptr<Field>(
      std::make_shared<AtomicType>(id, type, cond, attrs, sinks, l));
}

AtomicType::AtomicType(node_ptr<ID> id, node_ptr<type::Type> type,
                       node_ptr<expression::Expression> cond,
                       const attribute_list& attrs,
                       const expression_list& sinks, const Location& l)
    : Field(id, type, cond, attrs, expression_list(), sinks, l) {}

AtomicType::~AtomicType() {}

node_ptr<Item> AtomicType::clone() {
  expression_list _sinks;
  for (auto s : sinks()) {
    _sinks.push_back(s->clone());
  }
  auto cond = condition() ? condition()->clone() : nullptr;
  auto v = newNodePtr(std::make_shared<AtomicType>(
      id(), type(), cond, attributes()->clonedAttributes(), _sinks,
      location()));
  v->set_anonymous(anonymous());
  v->set_parsing_only(parsing_only());
  if (has_serialized_type()) v->set_serialized_type(serialized_type());
  return v;
}

ssize_t AtomicType::static_serialized_length() {
  auto type_len = serialized_type()->static_serialized_length();
  if (type_len >= 0) return type_len;

  auto len_attr = attributes()->lookup("length");
  if (len_attr) {
    if (auto c_expr = ast::tryCast<expression::Constant>(len_attr->value())) {
      auto c = c_expr->constant();
      if (auto int_c = ast::tryCast<constant::Integer>(c)) {
        return int_c->value();
      }
    }
  }
  return -1;
}

node_ptr<expression::Expression> AtomicType::serialized_length() {
  auto len = Field::serialized_length();
  if (len) return len;

  auto len_attr = attributes()->lookup("length");
  if (len_attr)
    return len_attr->value();
  else
    return nullptr;
}

std::string AtomicType::renderValue() { return ""; }

Constant::Constant(node_ptr<ID> id, node_ptr<constant::Constant> constant,
                   node_ptr<expression::Expression> cond,
                   const attribute_list& attrs, const expression_list& sinks,
                   const Location& l)
    : Field(id, constant->type(), cond, attrs, expression_list(), sinks, l),
      constant_(constant) {
  addChild(constant_);
}

Constant::~Constant() {}

node_ptr<Item> Constant::clone() {
  expression_list _sinks;
  for (auto s : sinks()) {
    _sinks.push_back(s->clone());
  }
  auto cond = condition() ? condition()->clone() : nullptr;
  auto v = newNodePtr(std::make_shared<Constant>(
      id(), constant(), cond, attributes()->clonedAttributes(), _sinks,
      location()));
  v->set_anonymous(anonymous());
  v->set_parsing_only(parsing_only());
  if (has_serialized_type()) v->set_serialized_type(serialized_type());
  return v;
}

ssize_t Constant::static_serialized_length() {
  return constant()->static_serialized_length();
}

std::string Constant::renderValue() { return constant_->render(); }

Ctor::Ctor(node_ptr<ID> id, node_ptr<ctor::Ctor> ctor,
           node_ptr<expression::Expression> cond, const attribute_list& attrs,
           const expression_list& sinks, const Location& l)
    : Field(id, newNodePtr(std::make_shared<type::Bytes>()), cond, attrs,
            expression_list(), sinks, l),
      ctor_(ctor) {
  addChild(ctor_);
}

Ctor::~Ctor() {}

node_ptr<Item> Ctor::clone() {
  expression_list _sinks;
  for (auto s : sinks()) {
    _sinks.push_back(s->clone());
  }
  auto cond = condition() ? condition()->clone() : nullptr;
  auto v = newNodePtr(std::make_shared<Ctor>(id(), ctor(), cond,
                                             attributes()->clonedAttributes(),
                                             _sinks, location()));
  v->set_anonymous(anonymous());
  v->set_parsing_only(parsing_only());
  if (has_serialized_type()) v->set_serialized_type(serialized_type());
  return v;
}

ssize_t Ctor::static_serialized_length() {
  // TODO(ES): support static_serialized_length calculation for ctors
  return -1;
}

node_ptr<expression::Expression> Ctor::serialized_length() {
  // TODO(ES): support serialized_length calculation for ctors
  return Field::serialized_length();
}

std::string Ctor::renderValue() { return ctor_->render(); }

Unknown::Unknown(node_ptr<ID> id, node_ptr<ID> referenced_id,
                 node_ptr<expression::Expression> cond,
                 const attribute_list& attrs, const expression_list& params,
                 const expression_list& sinks, const Location& l)
    : Field(id, nullptr, cond, attrs, params, sinks, l),
      referenced_id_(referenced_id) {
  addChild(referenced_id_);
}

Unknown::~Unknown() {}

node_ptr<Item> Unknown::clone() {
  expression_list _sinks;
  for (auto s : sinks()) {
    _sinks.push_back(s->clone());
  }
  expression_list params;
  for (auto p : parameters()) {
    params.push_back(p->clone());
  }
  auto cond = condition() ? condition()->clone() : nullptr;
  auto v = newNodePtr(std::make_shared<Unknown>(
      id(), referenced_id(), cond, attributes()->clonedAttributes(), params,
      _sinks, location()));
  v->set_anonymous(anonymous());
  v->set_parsing_only(parsing_only());
  if (has_serialized_type()) v->set_serialized_type(serialized_type());
  return v;
}

std::string Unknown::renderValue() { return referenced_id_->render(); }

Unit::Unit(node_ptr<ID> id, node_ptr<type::Type> type,
           node_ptr<expression::Expression> cond, const attribute_list& attrs,
           const expression_list& params, const expression_list& sinks,
           const Location& l)
    : Field(id, type, cond, attrs, params, sinks, l) {}

Unit::~Unit() {}

node_ptr<Item> Unit::clone() {
  expression_list _sinks;
  for (auto s : sinks()) {
    _sinks.push_back(s->clone());
  }
  expression_list params;
  for (auto p : parameters()) {
    params.push_back(p->clone());
  }
  auto cond = condition() ? condition()->clone() : nullptr;
  auto v = newNodePtr(std::make_shared<Unit>(id(), type(), cond,
                                             attributes()->clonedAttributes(),
                                             params, _sinks, location()));
  v->set_anonymous(anonymous());
  v->set_parsing_only(parsing_only());
  if (has_serialized_type()) v->set_serialized_type(serialized_type());
  return v;
}

ssize_t Unit::static_serialized_length() {
  return ast::checkedCast<type::unit::Unit>(type())->static_serialized_length();
}

node_ptr<expression::Expression> Unit::serialized_length() {
  // TODO(ES): evaluate parameter values?
  return ast::checkedCast<type::unit::Unit>(type())->serialized_length();
}

std::string Unit::renderValue() {
  auto params = parameters();
  std::string s = "";
  if (params.size() > 0) {
    s += "(";
    bool first = true;
    for (auto p : params) {
      if (first)
        first = false;
      else
        s += ", ";
      s += p->render();
    }
    s += ")";
  }
  return s;
}

namespace switch_ {

Switch::Switch(node_ptr<expression::Expression> expr, const case_list& cases,
               node_ptr<expression::Expression> cond, const Location& l)
    : Field(nullptr, nullptr, cond, attribute_list(), expression_list(),
            expression_list(), l),
      expression_(expr) {
  addChild(expression_);

  for (auto c : cases) cases_.push_back(c);
  for (auto c : cases_) addChild(c);

  for (auto c : cases_) {
    for (auto f : c->items()) {
      f->scope()->set_parent(scope());
    }
  }
}

Switch::~Switch() {}

void Switch::set_unit(unit::Unit* unit_) {
  Field::set_unit(unit_);
  for (auto c : cases_) {
    for (auto f : c->items()) {
      f->set_unit(unit_ptr());
    }
  }
}

Switch::case_list Switch::cases() const {
  case_list cases;
  for (auto c : cases_) cases.push_back(c);
  return cases;
}

bool Switch::noFields() const {
  for (auto c : cases_) {
    for (auto f : c->items()) {
      if (f->type() && !ast::isA<type::Void>(f->type())) return false;
    }
  }

  return true;
}

ssize_t Switch::static_serialized_length() {
  int last_len = 0;
  bool first = true;
  for (auto c : cases_) {
    ssize_t case_len = 0;
    for (auto i : c->items()) {
      if (auto f = tryCast<Field>(i)) {
        auto f_len = f->static_serialized_length();
        if (f_len < 0) return -1;
        case_len += f_len;
      }
    }

    if (!first && last_len != case_len) return -1;

    last_len = case_len;
    first = false;
  }
  return last_len;
}

node_ptr<expression::Expression> Switch::serialized_length() {
  // TODO(ES): support serialized_length calculation for switches
  // (if len exprs of all cases are the same, then return that len expr)
  return Field::serialized_length();
}

node_ptr<Item> Switch::clone() {
  // TODO(ES): clone cases?
  expression_list _sinks;
  for (auto s : sinks()) {
    _sinks.push_back(s->clone());
  }
  case_list _cases;
  for (auto c : cases()) {
    _cases.push_back(c->clone());
  }
  auto cond = condition() ? condition()->clone() : nullptr;
  auto expr = expression() ? expression()->clone() : nullptr;
  auto v = newNodePtr(std::make_shared<Switch>(expr, _cases, cond, location()));
  v->set_id(id());
  v->set_anonymous(anonymous());
  v->set_parsing_only(parsing_only());
  return v;
}

std::string Switch::renderValue() {
  return "switch(" + expression_->render() + ") [" +
         std::to_string(cases_.size()) + " cases]";
}

Case::Case(const expression_list& exprs, node_ptr<Field> item,
           const Location& l)
    : Case(exprs, (std::list<node_ptr<Field>>){item}, l) {}

Case::Case(const expression_list& exprs, const unit_field_list& items,
           const Location& l)
    : Node(l) {
  for (auto e : exprs) expressions_.push_back(e);
  for (auto e : expressions_) addChild(e);

  for (auto i : items) items_.push_back(i);
  for (auto i : items_) addChild(i);
}

Case::Case(node_ptr<Field> item, const Location& l)
    : Case({}, (std::list<node_ptr<Field>>){item}, l) {
  default_ = true;
}

Case::Case(const unit_field_list& items, const Location& l)
    : Case({}, items, l) {
  default_ = true;
}

Case::~Case() {}

expression_list Case::expressions() const {
  expression_list exprs;
  for (auto e : expressions_) exprs.push_back(e);
  return exprs;
}

unit_field_list Case::items() const {
  unit_field_list items;
  for (auto i : items_) items.push_back(i);
  return items;
}

node_ptr<Case> Case::clone() const {
  expression_list _exprs;
  for (auto e : expressions()) {
    _exprs.push_back(e->clone());
  }
  unit_field_list _items;
  for (auto i : items()) {
    _items.push_back(checkedCast<Field>(i->clone()));
  }
  return ast::newNodePtr(std::make_shared<Case>(_exprs, _items, location()));
}

std::string Case::render() {
  std::string s = "";

  if (default_) {
    s += "*";
  } else {
    s += "(";
    bool first = true;
    for (auto e : expressions_) {
      if (first)
        first = false;
      else
        s += ", ";
      s += e->render();
    }
    s += ")";
  }

  s += " => [";

  bool first = true;
  for (auto i : items_) {
    if (first)
      first = false;
    else
      s += ", ";
    s += i->render();
  }

  s += "]";
  return s;
}

}  // namespace switch_

namespace container {

Container::Container(node_ptr<ID> id, node_ptr<Field> contained_field,
                     node_ptr<expression::Expression> cond,
                     const attribute_list& attrs, const expression_list& sinks,
                     const Location& l)
    : Field(id, node_ptr<type::Type>(std::make_shared<type::Bytes>()), cond,
            attrs, expression_list(), sinks, l),
      contained_field_(contained_field) {
  contained_field_->scope()->set_parent(scope());
  addChild(contained_field_);
}

Container::~Container() {}

void Container::set_unit(unit::Unit* _unit) {
  Field::set_unit(_unit);
  contained_field_->set_unit(unit_ptr());
}

Vector::Vector(node_ptr<ID> id, node_ptr<Field> contained_field,
               node_ptr<expression::Expression> length,
               node_ptr<expression::Expression> cond,
               const attribute_list& attrs, const expression_list& sinks,
               const Location& l)
    : Container(id, contained_field, cond, attrs, sinks, l), length_(length) {
  addChild(length_);
}

Vector::~Vector() {}

node_ptr<Item> Vector::clone() {
  // TODO(ES): clone contained field?
  expression_list _sinks;
  for (auto s : sinks()) {
    _sinks.push_back(s->clone());
  }
  auto cond = condition() ? condition()->clone() : nullptr;
  auto v = newNodePtr(std::make_shared<Vector>(
      id(), checkedCast<Field>(contained_field()->clone()), length()->clone(),
      cond, attributes()->clonedAttributes(), _sinks, location()));
  v->set_anonymous(anonymous());
  v->set_parsing_only(parsing_only());
  if (has_serialized_type()) v->set_serialized_type(serialized_type());
  return v;
}

ssize_t Vector::static_serialized_length() {
  if (auto expr = ast::tryCast<expression::Constant>(length())) {
    if (auto int_const = ast::tryCast<constant::Integer>(expr->constant())) {
      auto f_len = contained_field()->static_serialized_length();
      if (f_len >= 0) {
        return int_const->value() * f_len;
      }
    }
  }
  return -1;
}

node_ptr<expression::Expression> Vector::serialized_length() {
  auto len = Field::serialized_length();
  if (len) return len;

  if (auto f_len = contained_field()->serialized_length()) {
    expression_list ops;
    ops.push_back(length());
    ops.push_back(f_len);
    return ast::newNodePtr(std::make_shared<expression::Operator>(
        expression::Operator::Kind::Mult, ops));
  }
  return nullptr;
}

std::string Vector::renderValue() {
  auto s = contained_field()->id()->render();
  s += "[" + length_->render() + "]";
  return s;
}

List::List(node_ptr<ID> id, node_ptr<Field> contained_field,
           node_ptr<expression::Expression> cond, const attribute_list& attrs,
           const expression_list& sinks, const Location& l)
    : Container(id, contained_field, cond, attrs, sinks, l) {}

List::~List() {}

node_ptr<Item> List::clone() {
  // TODO(ES): clone contained field?
  expression_list _sinks;
  for (auto s : sinks()) {
    _sinks.push_back(s->clone());
  }
  auto cond = condition() ? condition()->clone() : nullptr;
  auto v = newNodePtr(std::make_shared<List>(
      id(), checkedCast<Field>(contained_field()->clone()), cond,
      attributes()->clonedAttributes(), _sinks, location()));
  v->set_anonymous(anonymous());
  v->set_parsing_only(parsing_only());
  if (has_serialized_type()) v->set_serialized_type(serialized_type());
  return v;
}

std::string List::renderValue() {
  return "LIST<" + contained_field()->id()->render() + ">";
}

}  // namespace container

}  // namespace field

}  // namespace item

}  // namespace unit
}  // namespace type
}  // namespace ast
}  // namespace spec
}  // namespace diffingo
