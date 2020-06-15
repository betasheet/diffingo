/*
 * reg_exp.cpp
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

#include "spec/ast/ctor/reg_exp.h"
#include "spec/ast/type/reg_exp.h"

#include <list>
#include <memory>
#include <string>

#include "spec/ast/node.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace ctor {

RegExp::RegExp(const std::string& regexp, const attribute_list& attrs,
               const Location& l)
    : RegExp((pattern_list){regexp}, attrs, l) {}

RegExp::RegExp(const pattern_list& patterns, const attribute_list& attrs,
               const Location& l)
    : Ctor(l),
      type_(std::make_shared<type::RegExp>(l)),
      patterns_(patterns),
      attributes_(std::make_shared<AttributeMap>(attrs)) {
  addChild(type_);
  addChild(attributes_);
}

RegExp::~RegExp() {}

node_ptr<type::Type> RegExp::type() const { return type_; }

std::string RegExp::render() {
  std::string s = "";

  bool first = true;
  for (auto pat : patterns_) {
    if (!first)
      s += " | ";
    else
      first = false;

    s += "/" + pat + "/";
  }

  return s;
}

}  // namespace ctor
}  // namespace ast
}  // namespace spec
}  // namespace diffingo
