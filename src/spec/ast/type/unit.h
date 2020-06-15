/*
 * unit.h
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

#ifndef SRC_SPEC_AST_TYPE_UNIT_H_
#define SRC_SPEC_AST_TYPE_UNIT_H_

#include <list>
#include <memory>
#include <string>
#include <utility>

#include "spec/ast/attribute.h"
#include "spec/ast/constant/constant.h"
#include "spec/ast/ctor/ctor.h"
#include "spec/ast/expression/expression.h"
#include "spec/ast/id.h"
#include "spec/ast/location.h"
#include "spec/ast/module.h"
#include "spec/ast/node.h"
#include "spec/ast/type/function.h"
#include "spec/ast/type/type.h"
#include "spec/ast/visitor.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace type {
namespace unit {

class Unit : public Type {
 public:
  Unit(const parameter_list& params, const unit_item_list& items,
       const Location& l = Location::None);

  /// Create a wildcard unit type.
  explicit Unit(const Location& l = Location::None);

  virtual ~Unit();

  parameter_list parameters() const { return parameters_; }

  unit_item_list items() const { return items_; }

  /// Returns a "flattened" list of all of the units items. This descends
  /// into items that recursively contain further items and adds them all
  /// to the list as well.
  unit_item_list flattenedItems() const;

  /// Returns a list of all fields. This is a convenience method that
  /// pre-filters all items for this type,
  std::list<node_ptr<item::field::Field>> fields() const;

  /// Returns a "flattened" list of all of the unit's fields. This descends
  /// into fields that recursively contain further items and adds them all
  /// to the list as well.
  std::list<node_ptr<item::field::Field>> flattenedFields() const;

  /// Returns a list of all variables. This is a convenience method that
  /// pre-filters all items for this type,
  std::list<node_ptr<item::Variable>> variables() const;

  /// Returns a list of all properties. This is a convenience method that
  /// pre-filters all items for this type,
  std::list<node_ptr<item::Property>> properties() const;

  /// Returns a list of all properties with a given name.
  std::list<node_ptr<item::Property>> properties(const std::string& name) const;

  /// Returns the property of a given name, or null if none. If there's
  /// more than one property with the given name, one of them is returned.
  node_ptr<item::Property> property(const std::string& prop) const;

  /// Returns the item of a given name, or null if there's no such item.
  ///
  /// id: The item to look up.
  node_ptr<item::Item> item(node_ptr<ID> id) const;

  /// Returns the item of a given name, or null if there's no such item.
  ///
  /// name: The item to look up.
  node_ptr<item::Item> item(const std::string& name) const;

  /// Traverses an ID path to a sub-item and returns it. For example, \c
  /// a.b.c descends down field \c a, then \c b, and finally returns \c c
  /// there. Returns null for the item if the path is not valid. The second
  /// element of the returned tuple is a remaining string if we arrive at
  /// at leaf item before the path has been fully used (e.g., if with \c
  /// a.b.c, \c b is an atomic field, it would be set to \c c.), while the
  /// returned item is \c b.
  std::pair<node_ptr<item::Item>, std::string> path(const std::string& path);

  /// Returns true if this unit type has been exported. This is initially
  /// always false, but the normalizer pass will set it as necessary based
  /// on the operations that are performed with the type.
  bool exported() const { return exported_; }

  /// Marks the type as being exported. Mainly for internal use.
  void setExported() { exported_ = true; }

  /// Returns a BinPAC expression associated with a property defined by the
  /// unit, or the module it's in. If a item is specified and has a
  /// corresponding attribute, that's checked as well. The returned
  /// expression is the most-specific one, i.e., the item is checked first,
  /// then the unit, then the module. Returns null if none of them defines
  /// the property/attribute.
  node_ptr<expression::Expression> inheritedProperty(
      const std::string& property, node_ptr<item::Item> item = nullptr);

  std::shared_ptr<Scope> typeScope() override { return scope_; }

  node_ptr<Module> module() const {
    return module_ ? module_->nodePtr<Module>() : nullptr;
  }
  virtual void set_module(Module* module) { module_ = module; }

  ssize_t static_serialized_length() override;

  node_ptr<expression::Expression> serialized_length();

  std::string render() override;

  ACCEPT_VISITOR(Type)

 protected:
  // Internal helper for travesePath().
  std::pair<node_ptr<item::Item>, std::string> path(string_list path,
                                                    node_ptr<Unit> current);

 private:
  std::list<node_ptr<type::function::Parameter>> parameters_;
  std::list<node_ptr<item::Item>> items_;

  std::shared_ptr<Scope> scope_;

  // keeping pointer rather than node_ptr to avoid circular refs
  Module* module_ = nullptr;

  bool exported_ = false;
};

namespace item {

class Item : public Node {
 public:
  Item(node_ptr<ID> id, const node_ptr<Type> type = nullptr,
       const attribute_list& attrs = attribute_list(),
       const Location& l = Location::None);
  virtual ~Item();

  node_ptr<ID> id() const { return id_; }

  void set_id(node_ptr<ID> id) {
    if (id_) removeChild(id_);
    id_ = id;
    addChild(id_);
  }

  node_ptr<Type> type() const { return type_; }

  void set_type(node_ptr<Type> type) {
    if (type_) removeChild(type_);
    type_ = type;
    addChild(type_);
  }

  node_ptr<AttributeMap> attributes() const { return attributes_; }
  std::shared_ptr<Scope> scope() const { return scope_; }

  node_ptr<Type> serialized_type() const {
    if (serialized_type_) {
      return serialized_type_;
    } else {
      return type();
    }
  }

  bool has_serialized_type() const {
    return static_cast<bool>(serialized_type_);
  }

  void set_serialized_type(node_ptr<Type> serialized_type) {
    if (serialized_type_) removeChild(serialized_type_);
    serialized_type_ = serialized_type;
    addChild(serialized_type_);
  }

  /// Returns true if no ID was passed to the constructor.
  bool anonymous() const { return anonymous_; }
  void set_anonymous(bool anonymous = true) { anonymous_ = anonymous; }

  /// Returns true if field value is only needed during parsing.
  bool parsing_only() const { return parsing_only_; }
  void set_parsing_only(bool parsing_only = true) {
    parsing_only_ = parsing_only;
  }

  /// Returns true if field value is accessed by application.
  bool application_accessible() const { return app_accessible_; }
  void set_application_accessible(bool app_accessible = true) {
    app_accessible_ = app_accessible;
  }

  node_ptr<Unit> unit() const {
    return unit_ ? unit_->nodePtr<Unit>() : nullptr;
  }
  virtual void set_unit(Unit* unit) { unit_ = unit; }

  // Returns an expression associated with a property defined by the
  // item unit, or the module it's in. If the has a corresponding
  // attribute, that's checked first. The returned expression is the
  // most-specific one, i.e., the item is checked first, then the unit,
  // then the module. Returns a nullptr if none of them defines the
  // property/attribute.
  node_ptr<expression::Expression> inheritedProperty(
      const std::string& property);

  virtual node_ptr<Item> clone() = 0;

  std::string render() override;

  ACCEPT_VISITOR(Node)

 protected:
  Unit* unit_ptr() { return unit_; }

 private:
  // NOTE: if anything is added here, it may be necessary to
  // update IdResolver's visit method.

  node_ptr<ID> id_;
  bool anonymous_ = false;
  bool parsing_only_ = false;
  bool app_accessible_ = true;
  node_ptr<Type> type_;
  node_ptr<Type> serialized_type_;
  node_ptr<AttributeMap> attributes_;

  std::shared_ptr<Scope> scope_;

  // keeping pointer rather than node_ptr to avoid circular refs
  Unit* unit_ = nullptr;

  static int id_counter_;
};

class Variable : public Item {
 public:
  Variable(node_ptr<ID> id, node_ptr<Type> type,
           node_ptr<expression::Expression> default_,
           const attribute_list& attrs = attribute_list(),
           const Location& l = Location::None);
  virtual ~Variable();

  node_ptr<expression::Expression> default_value() { return default_value_; }

  node_ptr<Item> clone() override;

  std::string render() override;

  ACCEPT_VISITOR(Item)

 private:
  node_ptr<expression::Expression> default_value_;
};

class Property : public Item {
 public:
  explicit Property(node_ptr<Attribute> prop,
                    const Location& l = Location::None);
  virtual ~Property();

  node_ptr<Attribute> property() { return property_; }

  node_ptr<Item> clone() override;

  std::string render() override;

  ACCEPT_VISITOR(Item)

 private:
  node_ptr<Attribute> property_;
};

namespace field {

class Field : public Item {
 public:
  virtual ~Field();

  static node_ptr<Field> createByType(
      node_ptr<Type> type, node_ptr<ID> id,
      node_ptr<expression::Expression> cond = nullptr,
      const attribute_list& attrs = attribute_list(),
      const expression_list& params = expression_list(),
      const expression_list& sinks = expression_list(),
      const Location& l = Location::None);

  node_ptr<expression::Expression> condition() { return condition_; }

  expression_list parameters() const { return parameters_; }

  expression_list sinks() const { return sinks_; }

  std::string render() override;

  virtual ssize_t static_serialized_length();

  virtual node_ptr<expression::Expression> serialized_length();

  virtual std::string renderValue() = 0;

  ACCEPT_VISITOR(Item)

 protected:
  Field(node_ptr<ID> id, node_ptr<type::Type> type,
        node_ptr<expression::Expression> cond = nullptr,
        const attribute_list& attrs = attribute_list(),
        const expression_list& params = expression_list(),
        const expression_list& sinks = expression_list(),
        const Location& l = Location::None);

 private:
  // NOTE: if anything is added here, it may be necessary to
  // update IdResolver's visit method.

  node_ptr<expression::Expression> condition_;
  expression_list parameters_;
  expression_list sinks_;
};

class AtomicType : public Field {
 public:
  AtomicType(node_ptr<ID> id, node_ptr<type::Type> type,
             node_ptr<expression::Expression> cond = nullptr,
             const attribute_list& attrs = attribute_list(),
             const expression_list& sinks = expression_list(),
             const Location& l = Location::None);
  virtual ~AtomicType();

  node_ptr<Item> clone() override;

  ssize_t static_serialized_length() override;

  node_ptr<expression::Expression> serialized_length() override;

  std::string renderValue() override;

  ACCEPT_VISITOR(Field)
};

class Constant : public Field {
 public:
  Constant(node_ptr<ID> id, node_ptr<constant::Constant> constant,
           node_ptr<expression::Expression> cond = nullptr,
           const attribute_list& attrs = attribute_list(),
           const expression_list& sinks = expression_list(),
           const Location& l = Location::None);
  virtual ~Constant();

  node_ptr<constant::Constant> constant() const { return constant_; }

  node_ptr<Item> clone() override;

  ssize_t static_serialized_length() override;

  std::string renderValue() override;

  ACCEPT_VISITOR(Field)

 private:
  node_ptr<constant::Constant> constant_;
};

class Ctor : public Field {
 public:
  Ctor(node_ptr<ID> id, node_ptr<ctor::Ctor> ctor,
       node_ptr<expression::Expression> cond = nullptr,
       const attribute_list& attrs = attribute_list(),
       const expression_list& sinks = expression_list(),
       const Location& l = Location::None);
  virtual ~Ctor();

  node_ptr<ctor::Ctor> ctor() const { return ctor_; }

  node_ptr<Item> clone() override;

  ssize_t static_serialized_length() override;

  node_ptr<expression::Expression> serialized_length() override;

  std::string renderValue() override;

  ACCEPT_VISITOR(Field)

 private:
  node_ptr<ctor::Ctor> ctor_;
};

class Unknown : public Field {
 public:
  Unknown(node_ptr<ID> id, node_ptr<ID> referenced_id,
          node_ptr<expression::Expression> cond = nullptr,
          const attribute_list& attrs = attribute_list(),
          const expression_list& params = expression_list(),
          const expression_list& sinks = expression_list(),
          const Location& l = Location::None);
  virtual ~Unknown();

  node_ptr<ID> referenced_id() const { return referenced_id_; }

  node_ptr<Item> clone() override;

  std::string renderValue() override;

  ACCEPT_VISITOR(Field)

 private:
  node_ptr<ID> referenced_id_;
};

class Unit : public Field {
 public:
  Unit(node_ptr<ID> id, node_ptr<type::Type> type,
       node_ptr<expression::Expression> cond = nullptr,
       const attribute_list& attrs = attribute_list(),
       const expression_list& params = expression_list(),
       const expression_list& sinks = expression_list(),
       const Location& l = Location::None);
  virtual ~Unit();

  node_ptr<Item> clone() override;

  ssize_t static_serialized_length() override;

  node_ptr<expression::Expression> serialized_length() override;

  std::string renderValue() override;

  ACCEPT_VISITOR(Field)
};

namespace switch_ {

class Switch : public Field {
 public:
  typedef std::list<node_ptr<Case>> case_list;

  Switch(node_ptr<expression::Expression> expr, const case_list& cases,
         node_ptr<expression::Expression> cond = nullptr,
         const Location& l = Location::None);
  virtual ~Switch();

  node_ptr<expression::Expression> expression() const { return expression_; }

  void set_unit(unit::Unit* unit) override;

  case_list cases() const;

  /// Returns true if there's no field storing information.
  bool noFields() const;

  node_ptr<Item> clone() override;

  ssize_t static_serialized_length() override;

  node_ptr<expression::Expression> serialized_length() override;

  std::string renderValue() override;

  ACCEPT_VISITOR(Field)

 private:
  node_ptr<expression::Expression> expression_;
  std::list<node_ptr<Case>> cases_;
};

class Case : public Node {
 public:
  /// Constructor with single item implementing the case
  Case(const expression_list& exprs, node_ptr<Field> item,
       const Location& l = Location::None);

  /// Constructor with list of items implementing the case
  Case(const expression_list& exprs, const unit_field_list& items,
       const Location& l = Location::None);

  /// Constructor for default case with single item
  explicit Case(node_ptr<Field> item, const Location& l = Location::None);

  /// Constructor for default case with list of items
  explicit Case(const unit_field_list& items,
                const Location& l = Location::None);

  virtual ~Case();

  expression_list expressions() const;

  unit_field_list items() const;

  /// Returns true if this is the default case.
  bool _default() const { return default_; }

  node_ptr<Case> clone() const;

  std::string render() override;

  ACCEPT_VISITOR(Node)

 private:
  std::list<node_ptr<expression::Expression>> expressions_;
  std::list<node_ptr<Field>> items_;
  bool default_ = false;
};

}  // namespace switch_

namespace container {

class Container : public Field {
 public:
  Container(node_ptr<ID> id, node_ptr<Field> contained_field,
            node_ptr<expression::Expression> cond = nullptr,
            const attribute_list& attrs = attribute_list(),
            const expression_list& sinks = expression_list(),
            const Location& l = Location::None);
  virtual ~Container();

  node_ptr<Field> contained_field() const { return contained_field_; }

  void set_unit(unit::Unit* unit) override;

  ACCEPT_VISITOR(Field)

 private:
  node_ptr<Field> contained_field_;
};

class Vector : public Container {
 public:
  Vector(node_ptr<ID> id, node_ptr<Field> contained_field,
         node_ptr<expression::Expression> length,
         node_ptr<expression::Expression> cond = nullptr,
         const attribute_list& attrs = attribute_list(),
         const expression_list& sinks = expression_list(),
         const Location& l = Location::None);
  virtual ~Vector();

  node_ptr<expression::Expression> length() const { return length_; }

  node_ptr<Item> clone() override;

  ssize_t static_serialized_length() override;

  node_ptr<expression::Expression> serialized_length() override;

  std::string renderValue() override;

  ACCEPT_VISITOR(Container)

 private:
  node_ptr<expression::Expression> length_;
};

class List : public Container {
 public:
  List(node_ptr<ID> id, node_ptr<Field> contained_field,
       node_ptr<expression::Expression> cond = nullptr,
       const attribute_list& attrs = attribute_list(),
       const expression_list& sinks = expression_list(),
       const Location& l = Location::None);
  virtual ~List();

  node_ptr<Item> clone() override;

  std::string renderValue() override;

  ACCEPT_VISITOR(Container)
};

}  // namespace container

}  // namespace field

}  // namespace item

}  // namespace unit
}  // namespace type
}  // namespace ast
}  // namespace spec
}  // namespace diffingo

#endif  // SRC_SPEC_AST_TYPE_UNIT_H_
