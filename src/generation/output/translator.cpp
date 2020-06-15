/*
 * translator.cpp
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

#include "generation/output/translator.h"

#include <string>

#include "spec/ast/exception.h"
#include "spec/ast/expression/expression.h"
#include "spec/ast/id.h"
#include "spec/ast/node.h"
#include "spec/ast/type/type.h"
#include "util/util.h"

namespace diffingo {
namespace generation {
namespace output {

Translator::Translator() : expression_translator_(this) {}

std::string Translator::moduleNamespace(node_ptr<spec::ast::ID> id) const {
  return util::fmt("diffingo::gen::%s", id->name());
}

std::string Translator::functionName(const std::string& name) const {
  return name;
}

std::string Translator::functionParamName(const std::string& name) const {
  return name;
}

std::string Translator::unitName(const std::string& name) const { return name; }

std::string Translator::unitFieldName(const std::string& name) const {
  return name;
}

std::string Translator::unitVarName(const std::string& name) const {
  return name;
}

std::string Translator::enumName(const std::string& name) const { return name; }

std::string Translator::enumLabel(const std::string& label) const {
  return label;
}

std::string Translator::type(node_ptr<spec::ast::type::Type> type) {
  std::string result;
  bool success = type_translator_.translate(type, &result);
  if (!success)
    throw spec::ast::RuntimeError("could not translate type", type.get());
  return result;
}

std::string Translator::unitParserName(const std::string& name) const {
  return unitName(name) + "Parser";
}

std::string Translator::unitSerializerName(const std::string& name) const {
  return unitName(name) + "Serializer";
}

std::string Translator::expression(
    node_ptr<spec::ast::expression::Expression> expr) {
  std::string result;
  bool success = expression_translator_.translate(expr, &result);
  if (!success)
    throw spec::ast::RuntimeError("could not translate expression", expr.get());
  return result;
}

}  // namespace output
}  // namespace generation
}  // namespace diffingo
