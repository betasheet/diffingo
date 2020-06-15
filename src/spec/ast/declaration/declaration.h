/*
 * declaration.h
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

#ifndef SRC_SPEC_AST_DECLARATION_DECLARATION_H_
#define SRC_SPEC_AST_DECLARATION_DECLARATION_H_

#include <memory>
#include <string>

#include "spec/ast/id.h"
#include "spec/ast/location.h"
#include "spec/ast/node.h"
#include "spec/ast/visitor.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace declaration {

class Declaration : public Node {
 public:
  enum Linkage { EXPORTED, PRIVATE, IMPORTED };

  Declaration(node_ptr<ID> id, Linkage linkage,
              const Location& l = Location::None);
  virtual ~Declaration();

  node_ptr<ID> id() const { return id_; }
  Linkage linkage() const { return linkage_; }

  ACCEPT_VISITOR(Node)

 protected:
  std::string linkageAsString() const;

 private:
  node_ptr<ID> id_;
  Linkage linkage_;
};

}  // namespace declaration
}  // namespace ast
}  // namespace spec
}  // namespace diffingo

#endif  // SRC_SPEC_AST_DECLARATION_DECLARATION_H_
