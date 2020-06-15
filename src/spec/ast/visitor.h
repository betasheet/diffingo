/*
 * visitor.h
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

#ifndef SRC_SPEC_AST_VISITOR_H_
#define SRC_SPEC_AST_VISITOR_H_

#include <pantheios/pantheios.hpp>
#include <cassert>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <typeinfo>
#include <utility>

#include "spec/ast/location.h"
#include "spec/ast/node.h"
#include "util/util.h"

namespace diffingo {
namespace spec {
namespace ast {

#define ACCEPT_VISITOR(base)                                             \
  void accept(VisitorInterface* visitor, const node_ptr<Node>& node)     \
      override {                                                         \
    this->base::accept(visitor, node);                                   \
    auto nptr =                                                          \
        checkedCast<std::remove_reference<decltype(*this)>::type>(node); \
    visitor->visit(nptr);                                                \
  }                                                                      \
  const char* acceptClass() const override {                             \
    return util::type_name(*this).c_str();                               \
  }

#define ACCEPT_VISITOR_FORWARD_TO_BASE(base)                         \
  void accept(VisitorInterface* visitor, const node_ptr<Node>& node) \
      override {                                                     \
    this->base::accept(visitor, node);                               \
  }                                                                  \
  const char* acceptClass() const { return util::type_name(*this).c_str(); }

#define ACCEPT_DISABLED()                                               \
  void accept(VisitorInterface* visitor, const node_ptr<Node>& node)    \
      override {                                                        \
    visitor->log(pantheios::emergency, "disabled accept called", this); \
  }                                                                     \
  const char* acceptClass() const { return util::type_name(*this).c_str(); }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
class VisitorInterface {
 public:
  VisitorInterface() {}
  virtual ~VisitorInterface() {}

  virtual void visit(node_ptr<Node> node) {}
  virtual void visit(node_ptr<Attribute> node) {}
  virtual void visit(node_ptr<AttributeMap> node) {}
  virtual void visit(node_ptr<Function> node) {}
  virtual void visit(node_ptr<ID> node) {}
  virtual void visit(node_ptr<Module> node) {}
  virtual void visit(node_ptr<Location> node) {}
  virtual void visit(node_ptr<Transform> node) {}
  virtual void visit(node_ptr<constant::Bitset> node) {}
  virtual void visit(node_ptr<constant::Bool> node) {}
  virtual void visit(node_ptr<constant::Constant> node) {}
  virtual void visit(node_ptr<constant::Double> node) {}
  virtual void visit(node_ptr<constant::Enum> node) {}
  virtual void visit(node_ptr<constant::Integer> node) {}
  virtual void visit(node_ptr<constant::None> node) {}
  virtual void visit(node_ptr<constant::String> node) {}
  virtual void visit(node_ptr<constant::Tuple> node) {}
  virtual void visit(node_ptr<ctor::Bytes> node) {}
  virtual void visit(node_ptr<ctor::Ctor> node) {}
  virtual void visit(node_ptr<ctor::RegExp> node) {}
  virtual void visit(node_ptr<declaration::Constant> node) {}
  virtual void visit(node_ptr<declaration::Declaration> node) {}
  virtual void visit(node_ptr<declaration::Function> node) {}
  virtual void visit(node_ptr<declaration::Transform> node) {}
  virtual void visit(node_ptr<declaration::Type> node) {}
  virtual void visit(
      node_ptr<declaration::unit_instantiation::Instantiation> node) {}
  virtual void visit(node_ptr<declaration::unit_instantiation::Item> node) {}
  virtual void visit(node_ptr<declaration::unit_instantiation::Field> node) {}
  virtual void visit(node_ptr<declaration::unit_instantiation::Property> node) {
  }
  virtual void visit(node_ptr<expression::Assign> node) {}
  virtual void visit(node_ptr<expression::Conditional> node) {}
  virtual void visit(node_ptr<expression::Constant> node) {}
  virtual void visit(node_ptr<expression::Ctor> node) {}
  virtual void visit(node_ptr<expression::Expression> node) {}
  virtual void visit(node_ptr<expression::Find> node) {}
  virtual void visit(node_ptr<expression::Function> node) {}
  virtual void visit(node_ptr<expression::ID> node) {}
  virtual void visit(node_ptr<expression::Lambda> node) {}
  virtual void visit(node_ptr<expression::ListComprehension> node) {}
  virtual void visit(node_ptr<expression::MemberAttribute> node) {}
  virtual void visit(node_ptr<expression::Operator> node) {}
  virtual void visit(node_ptr<expression::ParserState> node) {}
  virtual void visit(node_ptr<expression::Transform> node) {}
  virtual void visit(node_ptr<expression::Type> node) {}
  virtual void visit(node_ptr<expression::Variable> node) {}
  virtual void visit(node_ptr<type::Any> node) {}
  virtual void visit(node_ptr<type::Bitset> node) {}
  virtual void visit(node_ptr<type::Bool> node) {}
  virtual void visit(node_ptr<type::Bytes> node) {}
  virtual void visit(node_ptr<type::CAddr> node) {}
  virtual void visit(node_ptr<type::Container> node) {}
  virtual void visit(node_ptr<type::Double> node) {}
  virtual void visit(node_ptr<type::Enum> node) {}
  virtual void visit(node_ptr<type::Integer> node) {}
  virtual void visit(node_ptr<type::List> node) {}
  virtual void visit(node_ptr<type::Map> node) {}
  virtual void visit(node_ptr<type::MemberAttribute> node) {}
  virtual void visit(node_ptr<type::RegExp> node) {}
  virtual void visit(node_ptr<type::Set> node) {}
  virtual void visit(node_ptr<type::Sink> node) {}
  virtual void visit(node_ptr<type::String> node) {}
  virtual void visit(node_ptr<type::Tuple> node) {}
  virtual void visit(node_ptr<type::Type> node) {}
  virtual void visit(node_ptr<type::TypeType> node) {}
  virtual void visit(node_ptr<type::Unknown> node) {}
  virtual void visit(node_ptr<type::UnknownElementType> node) {}
  virtual void visit(node_ptr<type::Vector> node) {}
  virtual void visit(node_ptr<type::Void> node) {}
  virtual void visit(node_ptr<type::bitfield::Bitfield> node) {}
  virtual void visit(node_ptr<type::bitfield::Bits> node) {}
  virtual void visit(node_ptr<type::function::Function> node) {}
  virtual void visit(node_ptr<type::function::Parameter> node) {}
  virtual void visit(node_ptr<type::function::Result> node) {}
  virtual void visit(node_ptr<type::unit::Unit> node) {}
  virtual void visit(node_ptr<type::unit::item::Item> node) {}
  virtual void visit(node_ptr<type::unit::item::Property> node) {}
  virtual void visit(node_ptr<type::unit::item::Variable> node) {}
  virtual void visit(node_ptr<type::unit::item::field::AtomicType> node) {}
  virtual void visit(node_ptr<type::unit::item::field::Constant> node) {}
  virtual void visit(node_ptr<type::unit::item::field::Ctor> node) {}
  virtual void visit(node_ptr<type::unit::item::field::Field> node) {}
  virtual void visit(node_ptr<type::unit::item::field::Unit> node) {}
  virtual void visit(node_ptr<type::unit::item::field::Unknown> node) {}
  virtual void visit(
      node_ptr<type::unit::item::field::container::Container> node) {}
  virtual void visit(node_ptr<type::unit::item::field::container::List> node) {}
  virtual void visit(
      node_ptr<type::unit::item::field::container::Vector> node) {}
  virtual void visit(node_ptr<type::unit::item::field::switch_::Switch> node) {}
  virtual void visit(node_ptr<type::unit::item::field::switch_::Case> node) {}
  virtual void visit(node_ptr<variable::Variable> node) {}
  virtual void visit(node_ptr<variable::Local> node) {}
};
#pragma GCC diagnostic pop

/// Base class for all AST visitors.
///
/// The class supports different types of visitors, including:
///
///    - Visitors that iterate over all nodes in pre-/post order.
///
///    - Visitors that want to control the iteration themselves by having their
///      visit methods forward to child nodes as desired.
///
///    - Visitors with visit methods that receive up to 2 further arguments,
///    in addition to the Node currently being visited.
///
///    - Visitors that compute and return value.
///
/// All these can also be mixed. Argument and result types are parameterized
/// by template parameters.
template <typename Result = int, typename Arg1 = int, typename Arg2 = int>
class Visitor : public VisitorInterface {
 public:
  typedef std::list<node_ptr<Node>> node_list;

  class LogEmergency : std::exception {
   public:
    explicit LogEmergency(std::string msg) : msg_(msg) {}
    virtual ~LogEmergency() {}

    std::string msg() const { return msg_; }

    const char* what() const noexcept override { return msg_.c_str(); }

   private:
    std::string msg_;
  };

  Visitor() {}
  virtual ~Visitor() {}

  /// Marks the visitor as one who may modify node relationships while
  /// running.
  void set_modifier(bool is_modifier) { _modifier = is_modifier; }

  /// Processes all child nodes pre-order, i.e., parent nodes are visited
  /// before their children.
  ///
  /// node: The node where to start visiting.
  ///
  /// reverse: If true, visits siblings in reverse order.
  bool processAllPreOrder(node_ptr<Node> node, bool reverse = false);

  /// Processes all child nodes pre-order, i.e., parent nodes are visited
  /// after their children.
  ///
  /// node: The node where to start visiting.
  ///
  /// reverse: If true, visits siblings in reverse order.
  bool processAllPostOrder(node_ptr<Node> node, bool reverse = false);

  /// Visits just a single node. The methods doesn't recurse any further,
  /// although the individual visit methods can manually do so by invoking
  /// any of the #call methods.
  ///
  /// In other words, this method is external interface to visiting a single
  /// node, whereas *call* is internal interface when a visiting process is
  /// already in progress. Don't mix the two.
  ///
  /// node: The node to visit.
  ///
  /// Returns: True if no fatalError() has been reported.
  bool processOne(node_ptr<Node> node) {
    saveArgs();
    auto b = processOneInternal(node);
    restoreArgs();
    return b;
  }

  /// Like processOne(), but also allows the visit method to set a result
  /// value. If no result is set via setResult(), the default set with
  /// setDefault() is returned. If neither is set, that's an error.
  ///
  /// node: The node to visit.
  ///
  /// result: Pointer to instance to store result in.
  bool processOne(node_ptr<Node> node, Result* result) {
    saveArgs();
    auto b = processOneInternal(node, result);
    restoreArgs();
    return b;
  }

  /// Like processOne(), but also sets the visitors first argument. Visit
  /// methods can then retrieve it with #arg1.
  bool processOne(node_ptr<Node> node, Arg1 arg1) {
    saveArgs();
    this->setArg1(arg1);
    auto r = processOneInternal(node);
    restoreArgs();
    return r;
  }

  /// Like processOne(), but also sets the visitors first and second
  /// arguments. Visit methods can then retrieve it with #arg1 and #arg2.
  bool processOne(node_ptr<Node> node, Arg1 arg1, Arg2 arg2) {
    saveArgs();
    this->setArg1(arg1);
    this->setArg2(arg2);
    auto r = processOneInternal(node);
    restoreArgs();
    return r;
  }

  /// Like processOne(), with setting an argument and getting a result back.
  bool processOne(node_ptr<Node> node, Result* result, Arg1 arg1) {
    saveArgs();
    this->setArg1(arg1);
    auto r = processOneInternal(node, result);
    restoreArgs();
    return r;
  }

  /// Like processOne(), with setting arguments and getting a result back.
  bool processOne(node_ptr<Node> node, Result* result, Arg1 arg1, Arg2 arg2) {
    saveArgs();
    this->setArg1(arg1);
    this->setArg2(arg2);
    auto r = processOneInternal(node, result);
    restoreArgs();
    return r;
  }

  /// Returns true if we currently in a visit method that was triggered by
  /// recursing down from a higher-level method for type T. The result is
  /// undefined if there's currently no visiting in progress.
  template <typename T>
  bool in() {
    return current_<T>() != 0;
  }

  /// Returns the closest higher-level instance of a given type. This method
  /// must only be called during an ongoing visiting process from another
  /// visit method. It will return a higher-level objects of type T, or null
  /// if there's no such. Note that this may return the current instance,
  /// use parent() if that's not desired.
  template <typename T>
  node_ptr<T> current() {
    auto t = current_<T>();
    return t;
  }

  Location currentLocation() {
    for (auto i = _current.rbegin(); i != _current.rend(); i++) {
      if ((*i)->location() != Location::None) return (*i)->location();
    }

    return Location::None;
  }

  /// Returns the closest higher-level instance of a given type (excluding
  /// the current one). This method must only be called during an ongoing
  /// visiting process from another visit method. It will return a
  /// higher-level objects of type T, or null if there's no such.
  template <typename T>
  node_ptr<T> parent() {
    auto t = current_<T, true>();
    return t;
  }

  /// Returns the number of higher-level instance of a given type. This method
  /// must only be called during an ongoing visiting process from another
  /// visit method.
  template <typename T>
  int depth() {
    int d = 0;
    for (auto i = _current.rbegin(); i != _current.rend(); i++) {
      if (typeid(**i) == typeid(T)) ++d;
    }

    return d;
  }

  /// Returns the current list of nodes we have traversed so far from the
  /// top of the tree to get to the current node. The latter is included.
  const node_list& currentNodes() const { return _current; }

  /// Dumps the list of currentNodes(). This is primarily for
  /// debugging to track down the chain of nodes.
  void dumpCurrentNodes(std::ostream* out) {
    *out << "Current visitor nodes:" << std::endl;
    for (auto n : currentNodes()) *out << "   | " << *n << std::endl;
    *out << std::endl;
  }

  /// Returns the first argument given to the visitor. The argument can be
  /// set by calling setArg1() prior to starting the visiting via any of \c
  /// process* methods. The return value is undefined if no argument has
  /// been set.
  Arg1 arg1() const { return _arg1; }

  /// Returns the second argument given to the visitor. The argument can be
  /// set by calling setArg2() prior to starting the visiting via any of \c
  /// process* methods. The return value is undefined if no argument has
  /// been set.
  Arg2 arg2() const { return _arg2; }

  /// Sets the result to be returned by any of the \c *withResult() methods.
  /// This must only be called from a visit method during traversal.
  void setResult(Result result) {
    _result_set = true;
    _result = result;
  }

  /// Clears the result back to being unset.
  void clearResult() {
    _result_set = false;
    _result = Result();
  }

  /// Returns true if the result has been set.
  bool hasResult() const { return _result_set; }

  /// Returns the current result. The value is undefined if the result has
  /// not yet been set.
  Result result() const { return _result_set ? _result : Result(); }

  /// Recurse to another node. Must only be called by a visit method during
  /// traversal.
  void call(node_ptr<Node> node) {
    this->pushCurrent(node);
    this->printDebug(node);
    this->preAccept(node);
    node->accept(this, node);
    this->postAccept(node);
    this->popCurrent();
  }

  /// Returns the current recursion level during visiting.
  int level() const { return _level; }

  /// Returns a std::string of whitespace reflecting an indentation according to
  /// the current recursion level.
  std::string levelIndent() const;

  /// XXX
  void pushCurrentLocationNode(node_ptr<Node> node) {
    _loc_nodes.push_back(node);
  }

  /// XXX
  void popCurrentLocationNode() { _loc_nodes.pop_back(); }

  const node_ptr<Node> currentLocationNode() const { return _loc_nodes.back(); }

  void log(pantheios::pan_sev_t level, node_ptr<Node> node, std::string msg) {
    auto loc_node = _loc_nodes.back() ? _loc_nodes.back() : node;
    auto loc = (loc_node ? loc_node->location() : Location::None);

    std::stringstream full_msg;
    full_msg << "[" << util::type_name(*this) << "] ";
    full_msg << msg;
    if (loc_node) {
      auto r = static_cast<std::string>(*node);
      if (r != "<node>")
        full_msg << std::endl
                 << ">>> " << r;
    }

    pantheios::log(level, full_msg.str());

    if (level <= pantheios::error) {
      ++errors_;
      if (level <= pantheios::emergency) {
        throw LogEmergency(msg);
      }
    }
  }

  void log(pantheios::pan_sev_t level, std::string msg) {
    log(level, nullptr, msg);
  }

  int errors() const { return errors_; }

  /// Enables verbose debugging for this visitor. This will log all
  /// visits()to stderr.
  void enableDebugging(bool enabled) { _debug = enabled; }

  /// Returns whether visit() logging is enabled (either specifically for
  /// this visitor, or globally for all).
  bool debugging() const { return _debug; }

 protected:
  /// Sets the first argument. visit methods can then retrieve it with
  /// #arg1.
  ///
  /// This method is protected because it should normally be called from a
  /// wrapper method provided by the derived visitor class as its main entry
  /// point.
  void setArg1(Arg1 arg1) { _arg1 = arg1; }

  /// Sets the first argument. visit methods can then retrieve it with
  /// #arg2.
  ///
  /// This method is protected because it should normally be called from a
  /// wrapper method provided by the derived visitor class as its main entry
  /// point.
  void setArg2(Arg2 arg2) { _arg2 = arg2; }

  /// Sets the default result to be returned by any of the \c *withResult()
  /// methods if no visit methods sets one. It's an error if no node sets a
  /// result without a default being defined.
  ///
  /// This method is protected because it should normally be called from a
  /// wrapper method provided by the derived visitor class as its main entry
  /// point.
  void setDefaultResult(Result def) {
    _default = def;
    _default_set = true;
  }

  /// Resets the visiting process, clearing all previous state. Derived
  /// classes can override this but must call the parent implementations.
  virtual void reset() {
    _visited.clear();
    errors_ = 0;
  }

/// Called just before a node's accept(). The default implementation does
/// nothing.

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
  virtual void preAccept(node_ptr<ast::Node> node) {}
  virtual void postAccept(node_ptr<ast::Node> node) {}
#pragma GCC diagnostic pop

  void preOrder(node_ptr<Node> node, bool reverse = false);
  void postOrder(node_ptr<Node> node, bool reverse = false);

  // Prints the debug output for the current visit() call.
  virtual void printDebug(node_ptr<Node> node) {
    if (!debugging()) return;

    std::cerr << '[' << util::type_name(*this) << "::visit()]";

    for (auto i = _current.size(); i > 0; --i) std::cerr << "  ";

    std::cerr << std::string(*node) << std::endl;
  }

 private:
  void pushCurrent(node_ptr<Node> node) {
    _current.push_back(node);
    ++_level;
  }

  void popCurrent() {
    _current.pop_back();
    --_level;
  }

  template <typename T, bool skip_first = false>
  node_ptr<T> current_() const {
    auto i = _current.rbegin();

    if (skip_first) ++i;

    for (; i != _current.rend(); ++i) {
      if (node_ptr<T> ptr = tryCast<T>(*i)) return ptr;
    }

    return nullptr;
  }

  bool processOneInternal(node_ptr<Node> node);
  bool processOneInternal(node_ptr<Node> node, Result* result);
  void saveArgs();
  void restoreArgs();

  node_list _current;

  Arg1 _arg1;
  Arg2 _arg2;
  int _errors = 0;
  int _level = 0;
  bool _debug = false;
  bool _modifier = false;

  std::list<node_ptr<Node>> _loc_nodes = {nullptr};  // Initial null element.

  bool _result_set = false;
  Result _result;

  bool _default_set = false;
  Result _default;

  typedef std::set<node_ptr<Node>> node_set;
  node_set _visited;

  struct SavedArgs {
    Arg1 arg1;
    Arg2 arg2;
    bool result_set;
    bool default_set;
    Result result;
    Result default_;
  };

  std::list<SavedArgs> _saved_args;

  int errors_ = 0;
};

template <typename Result, typename Arg1, typename Arg2>
inline bool Visitor<Result, Arg1, Arg2>::processAllPreOrder(node_ptr<Node> node,
                                                            bool reverse) {
  reset();

  try {
    preOrder(node, reverse);
    return (errors() == 0);
  } catch (const LogEmergency& err) {
    // Message has already been printed.
    return false;
  }
}

template <typename Result, typename Arg1, typename Arg2>
inline bool Visitor<Result, Arg1, Arg2>::processAllPostOrder(
    node_ptr<Node> node, bool reverse) {
  reset();

  try {
    postOrder(node, reverse);
    return (errors() == 0);
  } catch (const LogEmergency& err) {
    // Message has already been printed.
    return false;
  }
}

template <typename Result, typename Arg1, typename Arg2>
inline void Visitor<Result, Arg1, Arg2>::saveArgs() {
  SavedArgs args;
  args.arg1 = _arg1;
  args.arg2 = _arg2;
  args.result_set = _result_set;
  args.default_set = _default_set;
  args.result = _result;
  args.default_ = _default;
  _saved_args.push_back(args);
}

template <typename Result, typename Arg1, typename Arg2>
inline void Visitor<Result, Arg1, Arg2>::restoreArgs() {
  auto args = _saved_args.back();
  _arg1 = args.arg1;
  _arg2 = args.arg2;
  _result_set = args.result_set;
  _default_set = args.default_set;
  _result = args.result;
  _default = args.default_;
  _saved_args.pop_back();
}

template <typename Result, typename Arg1, typename Arg2>
inline bool Visitor<Result, Arg1, Arg2>::processOneInternal(
    node_ptr<Node> node) {
  reset();

  try {
    this->call(node);
    return (errors() == 0);
  } catch (const LogEmergency& err) {
    // Message has already been printed.
    return false;
  }
}

template <typename Result, typename Arg1, typename Arg2>
inline bool Visitor<Result, Arg1, Arg2>::processOneInternal(node_ptr<Node> node,
                                                            Result* result) {
  assert(result);

  reset();

  try {
    _result_set = false;
    this->call(node);

    if (_result_set) {
      *result = _result;
    } else if (_default_set) {
      *result = _default;
    } else {
      log(pantheios::error, node,
          util::fmt("visit method in %s did not set result for %s",
                    util::type_name(*this), node->acceptClass()));
      abort();
    }

    return true;
  } catch (const LogEmergency& err) {
    // Message has already been printed.
    return false;
  }
}

template <typename Result, typename Arg1, typename Arg2>
inline void Visitor<Result, Arg1, Arg2>::preOrder(node_ptr<Node> node,
                                                  bool reverse) {
  if (_visited.find(node) != _visited.end())
    // Done already.
    return;

  _visited.insert(node);

  pushCurrent(node);
  this->printDebug(node);
  node->accept(this, node);

  Node::node_list tmp;
  Node::node_list::const_iterator begin;
  Node::node_list::const_iterator end;
  Node::node_list::const_reverse_iterator rbegin;
  Node::node_list::const_reverse_iterator rend;

  if (!_modifier) {
    begin = node->begin();
    end = node->end();
    rbegin = node->rbegin();
    rend = node->rend();
  } else {
    for (auto i = node->begin(); i != node->end(); i++) tmp.push_back(*i);

    begin = tmp.begin();
    end = tmp.end();
    rbegin = tmp.rbegin();
    rend = tmp.rend();
  }

  if (!reverse) {
    for (auto i = begin; i != end; i++) this->preOrder(*i);
  } else {
    for (auto i = rbegin; i != rend; i++) this->preOrder(*i);
  }

  popCurrent();
}

template <typename Result, typename Arg1, typename Arg2>
inline void Visitor<Result, Arg1, Arg2>::postOrder(node_ptr<Node> node,
                                                   bool reverse) {
  if (_visited.find(node) != _visited.end())
    // Done already.
    return;

  _visited.insert(node);

  pushCurrent(node);

  Node::node_list tmp;
  Node::node_list::const_iterator begin;
  Node::node_list::const_iterator end;
  Node::node_list::const_reverse_iterator rbegin;
  Node::node_list::const_reverse_iterator rend;

  if (!_modifier) {
    begin = node->begin();
    end = node->end();
    rbegin = node->rbegin();
    rend = node->rend();
  } else {
    for (auto i = node->begin(); i != node->end(); i++) tmp.push_back(*i);

    begin = tmp.begin();
    end = tmp.end();
    rbegin = tmp.rbegin();
    rend = tmp.rend();
  }

  if (!reverse) {
    for (auto i = begin; i != end; i++) this->postOrder(*i);
  } else {
    for (auto i = rbegin; i != rend; i++) this->postOrder(*i);
  }

  this->printDebug(node);
  node->accept(this, node);
  popCurrent();
}

template <typename Result, typename Arg1, typename Arg2>
inline std::string Visitor<Result, Arg1, Arg2>::levelIndent() const {
  auto s = std::string("");
  for (int i = 0; i < _level; ++i) s += "  ";
  return s;
}

class AstPrinter : public Visitor<> {
 public:
  AstPrinter() {}
  virtual ~AstPrinter() {}

  void run(node_ptr<Node> node) { processAllPreOrder(node, false); }

  using Visitor::visit;

  void visit(node_ptr<Node> node) override {
    std::cout << levelIndent();
    std::cout << static_cast<std::string>(*node) << std::endl;
  }
};

}  // namespace ast
}  // namespace spec
}  // namespace diffingo

#endif  // SRC_SPEC_AST_VISITOR_H_
