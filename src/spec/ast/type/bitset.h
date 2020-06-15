/*
 * bitset.h
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

#ifndef SRC_SPEC_AST_TYPE_BITSET_H_
#define SRC_SPEC_AST_TYPE_BITSET_H_

#include <list>
#include <memory>
#include <string>
#include <utility>

#include "spec/ast/location.h"
#include "spec/ast/type/type.h"
#include "spec/ast/visitor.h"

namespace diffingo {
namespace spec {
namespace ast {
namespace type {

class Bitset : public Type {
 public:
  typedef std::pair<node_ptr<ID>, int> Label;
  typedef std::list<Label> label_list;

  explicit Bitset(const label_list& labels, const Location& l = Location::None);

  /// Create a wildcard Bitset type.
  explicit Bitset(const Location& l = Location::None);

  virtual ~Bitset();

  label_list labels() const;

  int labelBit(node_ptr<ID> label);

  std::shared_ptr<Scope> typeScope() override;

  ssize_t static_serialized_length() override;

  std::string render() override;

  ACCEPT_VISITOR(Type)

 private:
  std::list<std::pair<node_ptr<ID>, int>> labels_;

  std::shared_ptr<Scope> scope_ = nullptr;
};

}  // namespace type
}  // namespace ast
}  // namespace spec
}  // namespace diffingo

#endif  // SRC_SPEC_AST_TYPE_BITSET_H_
