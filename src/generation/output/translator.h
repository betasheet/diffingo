/*
 * translator.h
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

#ifndef SRC_GENERATION_OUTPUT_TRANSLATOR_H_
#define SRC_GENERATION_OUTPUT_TRANSLATOR_H_

#include <string>

#include "generation/output/expression_translator.h"
#include "generation/output/type_translator.h"
#include "spec/ast/expression/expression.h"
#include "spec/ast/id.h"
#include "spec/ast/node.h"
#include "spec/ast/type/type.h"

namespace diffingo {
namespace generation {
namespace output {

using spec::ast::node_ptr;

class Translator {
 public:
  Translator();

  std::string moduleNamespace(node_ptr<spec::ast::ID> id) const;

  std::string functionName(const std::string &name) const;
  std::string functionParamName(const std::string &name) const;

  std::string unitName(const std::string &name) const;
  std::string unitFieldName(const std::string &name) const;
  std::string unitVarName(const std::string &name) const;
  std::string unitParserName(const std::string &name) const;
  std::string unitSerializerName(const std::string &name) const;

  std::string enumName(const std::string &name) const;
  std::string enumLabel(const std::string &label) const;

  std::string type(node_ptr<spec::ast::type::Type> type);

  std::string expression(node_ptr<spec::ast::expression::Expression> expr);

 private:
  TypeTranslator type_translator_;
  ExpressionTranslator expression_translator_;
};

}  // namespace output
}  // namespace generation
}  // namespace diffingo

#endif  // SRC_GENERATION_OUTPUT_TRANSLATOR_H_
