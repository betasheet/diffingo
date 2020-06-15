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

#include "spec/ast/type/reg_exp.h"

#include <list>
#include <memory>
#include <string>

#include "spec/ast/attribute.h"
#include "spec/ast/location.h"
#include "spec/ast/node.h"
#include "spec/ast/type/type.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace type {

RegExp::RegExp(const attribute_list& attrs, const Location& l)
    : Type(l), attributes_(std::make_shared<AttributeMap>(attrs)) {
  addChild(attributes_);
}

RegExp::RegExp(const Location& l) : RegExp(attribute_list(), l) {
  setWildcard(true);
}

RegExp::~RegExp() {}

std::string RegExp::render() { return "REGEXP " + Type::render(); }

}  // namespace type
}  // namespace ast
}  // namespace spec
}  // namespace diffingo
