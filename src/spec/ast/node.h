/*
 * node.h
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

#ifndef SRC_SPEC_AST_NODE_H_
#define SRC_SPEC_AST_NODE_H_

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <list>
#include <memory>
#include <set>
#include <string>
#include <typeinfo>

#include "spec/ast/location.h"
#include "spec/ast/node_types.h"
#include "util/util.h"

namespace diffingo {
namespace spec {
namespace ast {

#define DEBUG 1

#ifdef DEBUG
#define __checkCast(p, _p)                                               \
  if ((!p) && *_p) {                                                     \
    fprintf(stderr, "failed cast in ast::Node: got object of type %s\n", \
            util::type_name(*(*_p).get()).c_str());                      \
    abort();                                                             \
  }
#else
#define __checkCast(p, _p)
#endif

class Node;
class VisitorInterface;

/// A shared pointer to a Node. This pointer operates similar to a
/// shared_ptr: a node will generally be deleted once no node_ptr refers to
/// it anymore. There's a major difference though: if a node_ptr is copied,
/// the new pointer stays attached to the existing one. If either is changed
/// to point to a different Node, the change will be reflected by both
/// pointers. This allows to have multiple AST nodes storing pointers to the
/// same child node with all of them noticing when the child gets replaced
/// with something else.
///
/// node_ptr can in general by used transparently as a shared_ptr as well,
/// there are corresponding conversion operators. Note however that the tight
/// link gets lots if you first convert it to a shared_ptr and then create a
/// node_ptr from there. Always copy/assign node_ptr directly.
template <typename T>
class node_ptr {
 public:
  typedef std::shared_ptr<Node> ptr;

  node_ptr() { p_ = std::shared_ptr<ptr>(new ptr()); }
  // NOLINTNEXTLINE(runtime/explicit)
  node_ptr(std::nullptr_t) { p_ = std::shared_ptr<ptr>(new ptr()); }

  template <typename S, typename = typename std::enable_if<
                            std::is_convertible<S*, T*>::value>::type>
  node_ptr(const node_ptr<S>& other) {
    p_ = other.get_shared();
  }

  template <typename S>
  explicit node_ptr(std::shared_ptr<S> p) {
    p_ = std::shared_ptr<ptr>(new ptr(p));
    setNodePtrRef();
  }

  explicit node_ptr(std::shared_ptr<ptr> p) {
    p_ = p;
    setNodePtrRef();
  }

  explicit node_ptr(std::weak_ptr<ptr> p) { p_ = std::shared_ptr<ptr>(p); }

  T* get() const {
    assert(p_);
    auto p = std::dynamic_pointer_cast<T>(*p_).get();
    __checkCast(p, p_);
    return p;
  }

  T* operator->() const { return get(); }
  T& operator*() const { return *get(); }

  explicit operator bool() const { return p_.get()->get() != 0; }

  // Dynamically casts the pointer to a shared_ptr of given type. Returns a
  // nullptr if the dynamic cast fails
  // TODO(ES): may need this again later.
  template <typename S>
  explicit operator std::shared_ptr<S>() const {
    auto p = std::dynamic_pointer_cast<S>(*p_);
    __checkCast(p, p_);
    return p;
  }

  // Updates this node_ptr to be a copy of the given other one.
  // There still may be other node_ptr pointing to the original
  // Node object after this.
  node_ptr<T>& operator=(const node_ptr<T>& other) {
    p_ = other.p_;
    return *this;
  }

  // Updates this and all other related node_ptrs to point to
  // the given new Node object.
  template <typename S>
  node_ptr<T>& operator=(const std::shared_ptr<S>& other) {
    clearNodePtrRef();
    *p_ = other;
    setNodePtrRef();
    return *this;
  }

  // Updates this and all other related node_ptrs to point to
  // the Node object referenced by the given node_ptr.
  template <typename S>
  void replace(const node_ptr<S>& other) {
    clearNodePtrRef();
    *p_ = *other.get_shared();
  }

  std::shared_ptr<ptr> get_shared() const { return p_; }

  bool operator==(const node_ptr<T>& other) const { return *p_ == *other.p_; }
  bool operator!=(const node_ptr<T>& other) const { return *p_ != *other.p_; }
  bool operator<(const node_ptr<T>& other) const { return *p_ < *other.p_; }

 private:
  std::shared_ptr<ptr> p_;

  void clearNodePtrRef() { get()->node_ptr__.reset(); }

  void setNodePtrRef() { get()->node_ptr__ = p_; }

  template <typename T_, typename S_>
  friend node_ptr<T_> checkedCast(node_ptr<S_>);
  template <typename T_, typename S_>
  friend node_ptr<T_> tryCast(node_ptr<S_>);

  template <typename S>
  void copyFrom(const node_ptr<S>& other) {
    p_ = other.get_shared();
  }
};

template <typename T, typename U>
bool operator==(node_ptr<T> const& a, node_ptr<U> const& b) {
  return a.get() == b.get();
}

template <typename T, typename U>
bool operator!=(node_ptr<T> const& a, node_ptr<U> const& b) {
  return a.get() != b.get();
}

template <typename T>
node_ptr<T> newNodePtr(std::shared_ptr<T> sp) {
  return node_ptr<T>(sp);
}

typedef std::list<std::string> string_list;

typedef node_ptr<Attribute> attr;
typedef std::list<node_ptr<Attribute>> attribute_list;
typedef std::list<node_ptr<declaration::Declaration>> declaration_list;
typedef std::list<node_ptr<expression::Expression>> expression_list;
typedef std::list<node_ptr<expression::Operator>> operator_list;
typedef std::list<node_ptr<ID>> id_list;
typedef std::list<node_ptr<type::Type>> type_list;
typedef std::list<node_ptr<type::unit::item::Item>> unit_item_list;
typedef std::list<node_ptr<declaration::unit_instantiation::Item>>
    inst_item_list;
typedef std::list<node_ptr<type::unit::item::field::Field>> unit_field_list;
typedef std::list<node_ptr<type::function::Parameter>> parameter_list;
typedef std::list<node_ptr<type::unit::item::field::switch_::Case>>
    switch_case_list;  // NOLINT
typedef std::list<node_ptr<diffingo::spec::ast::type::bitfield::Bits>>
    bitfield_bits_list;  // NOLINT

class Node {
 public:
  typedef std::list<node_ptr<Node>> node_list;

  explicit Node(const Location& l = Location::None);
  Node(const Node& other);
  virtual ~Node();

  void addChild(node_ptr<Node> node);

  void removeChild(node_ptr<Node> node);

  /// Returns the location associated with the node, or Location::None if
  /// none.
  const Location& location() const { return location_; }

  /// Returns a non-recursive textual representation of the node. The output
  /// of render() will be included.
  operator std::string();

  /// Returns a readable one-line representation of the node.
  virtual std::string render() { return std::string("<node>"); }

  virtual void accept(VisitorInterface* visitor, const node_ptr<Node>& node);
  virtual const char* acceptClass() const;

  /// Returns true if a given node is a child of this one.
  ///
  /// recursive: If true, descend the tree recursively for searching the
  /// child
  bool hasChild(node_ptr<Node> n, bool recursive = false) const;

  /// Returns true if a given node is a child of this one.
  ///
  /// recursive: If true, descend the tree recursively for searching the
  /// child
  bool hasChild(Node* n, bool recursive = false) const;

  /// Returns a list of all child nodes, recursively if requested. Child
  /// nodes are added via addChild().
  const node_list children(bool recursive = false) const;

  /// Returns an iterator to the first child node.
  node_list::const_iterator begin() const { return children_.begin(); }

  /// Returns an iterator marking the end of the child list.
  node_list::const_iterator end() const { return children_.end(); }

  /// Returns a reverse iterator to the last child node.
  node_list::const_reverse_iterator rbegin() const {
    return children_.rbegin();
  }

  /// Returns a reverse iterator marking the end of the reversed child list.
  node_list::const_reverse_iterator rend() const { return children_.rend(); }

  /// Returns any comment that may be associated with the node.
  const std::list<std::string>& comments() const { return comments_; }

  /// Associates a comment with the node.
  void addComment(const std::string& comment) { comments_.push_back(comment); }

  template <typename T>
  node_ptr<T> nodePtr() {
    if (node_ptr__.expired()) {
      fprintf(stderr,
              "no node_ptr associated with node of type %s (anymore?)\n",
              util::type_name(*this).c_str());
      abort();
    }

    auto p = node_ptr<T>(node_ptr__);
    assert(p);
    return p;
  }

  /// Returns a shared_ptr for the node, cast to a specified type. The cast
  /// must not fail, behavior is undefined if it does.
  //  template<typename T>
  //  std::shared_ptr<T> sharedPtr() {
  //     auto p = std::dynamic_pointer_cast<T>(shared_from_this());
  //     assert(p);
  //     return p;
  //  }

  /// Returns a shared_ptr for the node, cast to a specified type. The cast
  /// must not fail, behavior is undefined if it does.
  //  template<typename T>
  //  std::shared_ptr<const T> sharedPtr() const {
  //     auto p = std::dynamic_pointer_cast<const T>(shared_from_this());
  //     assert(p);
  //     return p;
  //  }

 private:
  typedef std::set<node_ptr<Node>> node_set;

  bool hasChildInternal(Node* node, bool recursive, node_set* done) const;
  void childrenInternal(const Node* node, bool recursive,
                        node_set* children) const;

  Location location_;
  node_list children_;
  std::list<std::string> comments_;

  template <typename T>
  friend class node_ptr;
  mutable std::weak_ptr<node_ptr<Node>::ptr> node_ptr__;
};

/// Dynamic check whether a node is an instance of a class derived from a given
/// Base.
///
/// Returns: True if n is derived from class T.
template <typename T>
inline bool isA(node_ptr<Node> n) {
  return std::dynamic_pointer_cast<T>(*n.get_shared()) != 0;
}

/// Dynamic cast of a node pointer to a specific Node-derived class. This
/// version continues if the cast fails by returning null.
///
/// Returns: The cast pointer, or null if the dynamic cast fails.
template <typename T, typename S>
inline node_ptr<T> tryCast(node_ptr<S> n) {
  if (!isA<T>(n)) {
    return nullptr;
  } else {
    auto np = node_ptr<T>();
    np.copyFrom(n);
    return np;
  }
}

/// Dynamic cast of a node pointer to a specific Node-derived class. This
/// version continues if the cast fails by returning null.
///
/// Returns: The cast pointer, or null if the dynamic cast fails.
template <typename T>
inline T* tryCast(Node* n) {
  return dynamic_cast<T*>(n);
}

/// Dynamic cast of a node pointer to a specific Node-derived class. This
/// version aborts if the cast fails.
///
/// Returns: The cast pointer.
template <typename T, typename S>
inline node_ptr<T> checkedCast(node_ptr<S> n) {
  if (!isA<T>(n)) {
    fprintf(
        stderr,
        "internal error: ast::checkedCast() failed; want '%s' but got a '%s'",
        util::type_name<T>().c_str(), util::type_name(*n.get()).c_str());
    abort();
  } else {
    auto np = node_ptr<T>();
    np.copyFrom(n);
    return np;
  }
}

/// Dynamic cast of a node pointer to a specific Node-derived class. This
/// version aborts if the cast fails.
///
/// Returns: The cast pointer.
template <typename T>
inline T* checkedCast(Node* n) {
  auto c = dynamic_cast<T*>(n);

  if (!c) {
    fprintf(
        stderr,
        "internal error: ast::checkedCast() failed; want '%s' but got a '%s'",
        util::type_name<T>(), util::type_name(*n));
    abort();
  }

  return c;
}

}  // namespace ast
}  // namespace spec
}  // namespace diffingo

#endif  // SRC_SPEC_AST_NODE_H_
