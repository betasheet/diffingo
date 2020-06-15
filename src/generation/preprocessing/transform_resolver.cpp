/*
 * transform_resolver.cpp
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

#include "generation/preprocessing/transform_resolver.h"

#include <pantheios/pantheios.hpp>
#include <memory>

#include "spec/ast/attribute.h"
#include "spec/ast/expression/id.h"
#include "spec/ast/expression/type.h"
#include "spec/ast/module.h"
#include "spec/ast/node.h"
#include "spec/ast/type/atomic_types.h"
#include "spec/ast/type/unit.h"
#include "spec/ast/visitor.h"

namespace ast = diffingo::spec::ast;

namespace diffingo {
namespace generation {
namespace preprocessing {

TransformResolver::TransformResolver() {}

TransformResolver::~TransformResolver() {}

bool TransformResolver::run(node_ptr<spec::ast::Module> node,
                            const Options& options) {
  return processAllPreOrder(node);
}

void TransformResolver::visit(
    node_ptr<spec::ast::type::unit::item::Item> node) {
  if (node->attributes()->has("transform_to")) {
    auto attr = node->attributes()->lookup("transform_to");
    auto type_id = ast::tryCast<ast::expression::ID>(attr->value());
    auto type_expr = ast::tryCast<ast::expression::Type>(attr->value());
    if (!type_id && !type_expr) {
      log(pantheios::error, attr,
          "expected ID or Type expression for value of transform_to "
          "attribute");
    }

    node->set_serialized_type(node->type());

    // make new type
    node_ptr<ast::type::Type> type;
    if (type_id) {
      type = ast::newNodePtr(std::make_shared<ast::type::Unknown>(
          type_id->id(), type_id->location()));
    } else {
      type = type_expr->contained_type();
    }

    node->set_type(type);
  } else if (node->attributes()->has("transform")) {
    // TODO(ES): support custom transform functions
  }
}

}  // namespace preprocessing
}  // namespace generation
}  // namespace diffingo
