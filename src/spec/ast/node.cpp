/*
 * node.cpp
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

#include "spec/ast/node.h"

#include <cxxabi.h>
#include <list>
#include <string>
#include <typeinfo>

#include "spec/ast/visitor.h"
#include "util/util.h"

namespace diffingo {
namespace spec {
namespace ast {

Node::Node(const Location& l) : location_(l) {}

Node::Node(const Node& other) {
  location_ = other.location_;
  comments_ = other.comments_;
  children_ = other.children_;
}

Node::~Node() {
  // TODO(ES): if we keep track of parents association, we may have to clean up
  // references here, too.
}

void Node::addChild(node_ptr<Node> node) {
  if (!node.get()) return;

  for (auto i = children_.begin(); i != children_.end(); i++) {
    if (&(*i) == &node) return;
  }

  assert(node);
  children_.push_back(node);
  // TODO(ES): may need to keep track of parents association, too?
}

void Node::removeChild(node_ptr<Node> node) {
  if (!node) return;

  bool changed = false;
  do {
    changed = false;

    for (auto i = children_.begin(); i != children_.end(); i++) {
      if ((*i).get() == node.get()) {
        // TODO(ES): may need to keep track of parents association, too?
        children_.erase(i);
        changed = true;
        break;
      }
    }
  } while (changed);
}

Node::operator std::string() {
  std::string s = render();
  std::string location = "-";

  if (location_) location = std::string(location_);

  return util::fmt("%s [%d %s %p] %s", util::type_name(*this), children_.size(),
                   location.c_str(), this, s.c_str());
}

void Node::accept(VisitorInterface* visitor, const node_ptr<Node>& node) {
  node_ptr<Node> nptr(node);
  visitor->visit(nptr);
}

const char* Node::acceptClass() const { return util::type_name(*this).c_str(); }

bool Node::hasChild(node_ptr<Node> node, bool recursive) const {
  return hasChild(node.get(), recursive);
}

bool Node::hasChild(Node* node, bool recursive) const {
  node_set done;
  return hasChildInternal(node, recursive, &done);
}

bool Node::hasChildInternal(Node* node, bool recursive, node_set* done) const {
  assert(done || !recursive);

  for (auto c : children_) {
    if (done) {
      if (done->find(c) != done->end()) continue;

      done->insert(c);
    }

    if (c.get() == node) return true;

    if (recursive && c->hasChildInternal(node, true, done)) return true;
  }

  return false;
}

const Node::node_list Node::children(bool recursive) const {
  if (!recursive) return children_;

  node_set children;
  childrenInternal(this, recursive, &children);

  node_list result;
  for (auto c : children) result.push_back(c);

  return result;
}

void Node::childrenInternal(const Node* node, bool recursive,
                            node_set* children) const {
  for (auto c : node->children_) {
    if (children->find(c) != children->end()) continue;

    children->insert(c);
    childrenInternal(c.get(), recursive, children);
  }
}

}  // namespace ast
}  // namespace spec
}  // namespace diffingo
