/*
 * driver.cpp
 *
 * TODO adopted from from Binpac++, add their copyright.
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

#include "spec/parser/driver.h"

#include <pantheios/pantheios.hpp>
#include <cassert>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "autogen/spec/parser/location.hh"
#include "autogen/spec/parser/parser.h"
#include "spec/ast/expression/expression.h"
#include "spec/ast/module.h"
#include "spec/parser/scanner.h"

namespace diffingo {
namespace spec {

Driver::Driver() {
  // TODO(Eric Seckler): Auto-generated constructor stub
}

Driver::~Driver() {
  // TODO(Eric Seckler): Auto-generated destructor stub
}

node_ptr<ast::Module> Driver::parse(generation::CompilerContext* ctx,
                                    std::istream* in,
                                    const std::string& stream_name) {
  context_ = ctx;
  stream_name_ = stream_name;

  Scanner scanner(in);
  scanner_ = &scanner;

  Parser parser(this);
  parser_ = &parser;
  parser_->parse();

  scanner_ = nullptr;
  parser_ = nullptr;

  if (errors_ > 0) {
    errors_ = 0;
    return nullptr;
  }

  return module();
}

node_ptr<ast::Module> Driver::module() const {
  assert(module_);
  return module_;
}

void Driver::set_module(node_ptr<ast::Module> module) { module_ = module; }

void Driver::include(std::string filename) { includes_.push_back(filename); }

void Driver::enablePatternMode() { scanner_->enablePatternMode(); }

void Driver::disablePatternMode() { scanner_->disablePatternMode(); }

void Driver::error(const std::string& m, const location& l) {
  std::stringstream loc_ss;
  loc_ss << l;
  pantheios::log(pantheios::error,
                 "error while parsing: " + m + " at " + loc_ss.str());
  errors_++;
}

}  // namespace spec
}  // namespace diffingo
