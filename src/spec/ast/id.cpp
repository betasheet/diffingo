/*
 * id.cpp
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

#include "spec/ast/id.h"

#include <list>
#include <memory>
#include <string>

#include "util/util.h"

namespace diffingo {
namespace spec {
namespace ast {

ID::ID(std::string path, const Location& l)
    : Node(l), path_(util::strsplit(path, "::")) {}

ID::ID(component_list path, const Location& l) : Node(l), path_(path) {}

ID::~ID() {}

std::string ID::pathAsString(node_ptr<ID> relative_to) const {
  auto p1 = path_;
  auto p2 = relative_to ? relative_to->path_ : component_list();

  while (p1.size() && p2.size() && p1.front() == p2.front()) {
    p1.pop_front();
    p2.pop_front();
  }

  return util::strjoin(p1, "::");
}

std::string ID::render() { return pathAsString(); }

node_ptr<spec::ast::ID> ID::combineIDs(node_ptr<spec::ast::ID> first,
                                       node_ptr<spec::ast::ID> second) {
  auto id = ast::newNodePtr(std::make_shared<ast::ID>(first->path()));
  id->addPath(second->path());
  return id;
}

}  // namespace ast
}  // namespace spec
}  // namespace diffingo
