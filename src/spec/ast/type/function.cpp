/*
 * function.cpp
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

#include "spec/ast/type/function.h"

#include <list>
#include <memory>
#include <string>

#include "spec/ast/location.h"
#include "spec/ast/node.h"
#include "spec/ast/type/type.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace type {
namespace function {

Parameter::Parameter(node_ptr<ID> id, node_ptr<Type> type,
                     bool constant,
                     node_ptr<expression::Expression> default_value,
                     Location l)
    : Node(l),
      id_(id),
      type_(type),
      constant_(constant),
      default_value_(default_value) {
  if (id_) addChild(id_);
  addChild(type_);
  if (default_value_) addChild(default_value_);
}

Parameter::Parameter(node_ptr<Type> type, bool constant, Location l)
    : Parameter(nullptr, type, constant, nullptr, l) {}

Parameter::~Parameter() {}

std::string Parameter::render() {
  std::string s = "";
  if (id_) s += id_->render() + " : ";
  if (constant_) s += "const ";
  s += type_->render();
  return s;
}

Result::Result(node_ptr<Type> type, bool constant, Location l)
    : Node(l), type_(type), constant_(constant) {
  addChild(type_);
}

Result::~Result() {}

std::string Result::render() {
  std::string s = "";
  if (constant_) s += "const ";
  s += type_->render();
  return s;
}

Function::Function(node_ptr<Result> result, const parameter_list& args,
                   const Location& l)
    : Type(l), result_(result) {
  addChild(result_);

  for (auto a : args) args_.push_back(a);
  for (auto a : args_) addChild(a);
}

Function::Function(const Location& l) : Type(l) { setWildcard(true); }

Function::~Function() {}

parameter_list Function::args() const {
  parameter_list args;
  for (auto a : args_) args.push_back(a);
  return args;
}

std::string Function::render() {
  std::string s = "FUNCTION ";
  s += "(" + std::to_string(args_.size()) + " params) ";
  if (result_) s += ": " + result_->render();
  s += Type::render();
  return s;
}

}  // namespace function
}  // namespace type
}  // namespace ast
}  // namespace spec
}  // namespace diffingo
