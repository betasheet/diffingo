/*
 * constant.cpp
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

#include "spec/ast/declaration/constant.h"

#include <memory>
#include <string>

#include "spec/ast/expression/expression.h"
#include "spec/ast/id.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace declaration {

Constant::Constant(node_ptr<ID> id, Linkage linkage,
                   node_ptr<expression::Expression> value,
                   const Location& l)
    : Declaration(id, linkage, l), value_(value) {
  addChild(value_);
}

Constant::~Constant() {}

std::string Constant::render() {
  return linkageAsString() + " const " + id()->render() + " = " +
         value_->render();
}

}  // namespace declaration
}  // namespace ast
}  // namespace spec
}  // namespace diffingo
