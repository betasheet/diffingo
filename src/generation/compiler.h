/*
 * compiler.h
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

#ifndef SRC_GENERATION_COMPILER_H_
#define SRC_GENERATION_COMPILER_H_

#include <string>

#include "generation/pass.h"
#include "spec/ast/module.h"
#include "spec/ast/node.h"

namespace diffingo {
namespace generation {

using diffingo::spec::ast::node_ptr;

struct Options {
  std::string file;
  std::string name_space;
  std::string output_dir;
  bool print_ast;
  bool instantiation_only;
  bool input_pointers;
  bool store_parsing_only;
};

class Compiler {
 public:
  explicit Compiler(Options* options);
  virtual ~Compiler();

  bool run();

  bool executePasses(node_ptr<spec::ast::Module> module);

  template <typename Result, typename Arg1, typename Arg2>
  bool executePass(node_ptr<spec::ast::Module> module,
                   generation::Pass<Result, Arg1, Arg2>* pass);

 private:
  Options* options_;
};

}  // namespace generation
}  // namespace diffingo

#endif  // SRC_GENERATION_COMPILER_H_
