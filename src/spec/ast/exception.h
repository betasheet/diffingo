/*
 * exception.h
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

#ifndef SRC_SPEC_AST_EXCEPTION_H_
#define SRC_SPEC_AST_EXCEPTION_H_

#include <exception>
#include <string>

#include "spec/ast/location.h"
#include "spec/ast/node.h"

namespace diffingo {
namespace spec {
namespace ast {

/// Base class for all AST exceptions.
class Exception : public std::exception {
 public:
  /// Constructor. Optionally, a Node can be associated with the exception.
  ///
  /// what: A textual description. Will be passed to std::exception.
  ///
  /// node: The optional Node.
  explicit Exception(std::string what, const Node* node = nullptr) throw() {
    _what = what;
    _node = node;
  }

  virtual ~Exception() throw() {}

  /// Returns the Node associated with the exception.
  const Node* node() const { return _node; }

  /// Returns the associated Node's location. The return value will be
  /// Location::None if no node has been set.
  const Location& location() const {
    return _node ? _node->location() : Location::None;
  }

  // Returns the textual description. If the exception has a Node associated
  // that has a location, that is added automatically.
  virtual const char* what() const throw();

 private:
  // Exception(const Exception&) = delete;  // FIXME: Not sure why I can't
  // delete this here.
  Exception& operator=(const Exception&) = delete;

  std::string _what;
  const Node* _node;
};

/// Exception reporting an run-time error due to unexpected user input.
class RuntimeError : public Exception {
 public:
  /// Constructor. Optionally, a Node can be associated with the exception.
  ///
  /// what: A textual description. Will be passed to std::exception.
  ///
  /// node: The optional Node.
  explicit RuntimeError(std::string what, const Node* node = nullptr)
      : Exception(what, node) {}
};

/// Exception reporting an internal logic error.
class InternalError : public Exception {
 public:
  /// Constructor. Optionally, a Node can be associated with the exception.
  ///
  /// what: A textual description. Will be passed to std::exception.
  ///
  /// node: The optional Node.
  explicit InternalError(std::string what, const Node* node = nullptr)
      : Exception(what, node) {}
};

}  // namespace ast
}  // namespace spec
}  // namespace diffingo

#endif  // SRC_SPEC_AST_EXCEPTION_H_
