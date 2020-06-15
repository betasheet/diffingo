/*
 * unit_scope_builder.h
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

#ifndef SRC_GENERATION_PREPROCESSING_UNIT_SCOPE_BUILDER_H_
#define SRC_GENERATION_PREPROCESSING_UNIT_SCOPE_BUILDER_H_

#include "generation/pass.h"
#include "spec/ast/declaration/type.h"
#include "spec/ast/module.h"
#include "spec/ast/node.h"
#include "spec/ast/visitor.h"

namespace diffingo {
namespace generation {
namespace preprocessing {

using spec::ast::node_ptr;

class UnitScopeBuilder : public Pass<> {
 public:
  UnitScopeBuilder();
  virtual ~UnitScopeBuilder();

  bool run(node_ptr<spec::ast::Module> node,
           const Options& options) override;

  using Visitor::visit;

  void visit(node_ptr<spec::ast::declaration::Type> node) override;
};

}  // namespace preprocessing
}  // namespace generation
}  // namespace diffingo

#endif  // SRC_GENERATION_PREPROCESSING_UNIT_SCOPE_BUILDER_H_
