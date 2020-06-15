/*
 * compiler.cpp
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

#include "generation/compiler.h"  // NOLINT

#include <boost/filesystem.hpp>
#include <pantheios/pantheios.hpp>
#include <fstream>
#include <list>
#include <string>

#include "generation/compiler_context.h"
#include "generation/output/code_generator.h"
#include "generation/pass.h"
#include "generation/preprocessing/id_resolver.h"
#include "generation/preprocessing/instantiation_dependency_resolver.h"
#include "generation/preprocessing/instantiation_type_generator.h"
#include "generation/preprocessing/scope_builder.h"
#include "generation/preprocessing/transform_resolver.h"
#include "generation/preprocessing/unit_scope_builder.h"
#include "spec/ast/module.h"
#include "spec/ast/node.h"
#include "spec/ast/visitor.h"
#include "spec/parser/driver.h"
#include "util/util.h"

namespace diffingo {
namespace generation {

using diffingo::spec::ast::node_ptr;

Compiler::Compiler(Options* options) : options_(options) {}

Compiler::~Compiler() {}

bool Compiler::run() {
  pantheios::log(pantheios::informational,
                 "Parsing spec file " + options_->file + " ...");
  std::fstream fs;
  fs.open(options_->file, std::fstream::in);

  if (!fs.is_open()) {
    pantheios::log(pantheios::error, "could not open spec file - aborting.");
    return 1;
  }

  diffingo::generation::CompilerContext ctx;

  diffingo::spec::Driver driver;
  node_ptr<diffingo::spec::ast::Module> m =
      driver.parse(&ctx, &fs, options_->file);

  fs.close();
  if (!m) {
    pantheios::log(pantheios::error, "parsing failed - aborting.");
    return 1;
  }

  boost::filesystem::path file_path(options_->file);

  // TODO(ES): currently only supporting one level of includes
  auto includes = driver.includes();
  includes.reverse();
  for (auto incl : includes) {
    auto filename = file_path.parent_path();
    filename /= incl;
    fs.open(filename.string(), std::fstream::in);

    if (!fs.is_open()) {
      pantheios::log(
          pantheios::error,
          util::fmt("could not open included file %s - aborting.", incl));
      return 1;
    }

    node_ptr<diffingo::spec::ast::Module> m2 =
        driver.parse(&ctx, &fs, filename.string());

    // TODO(ES): currently no support for namespaces here.
    m2->addDeclarations(m->declarations());
    for (auto prop : m->properties()->attributes()) {
      m->addProperty(prop);
    }
    m = m2;
  }

  if (options_->print_ast) {
    pantheios::log(pantheios::informational, "Generated AST:");
    // std::cout << static_cast<std::string>(*(m->id())) << std::endl;
    diffingo::spec::ast::AstPrinter printer;
    printer.run(m);
  }

  if (!executePasses(m)) {
    pantheios::log(pantheios::error, "one of the passes failed - aborting.");
    return 1;
  }

  boost::filesystem::path output_path;
  if (options_->output_dir.empty()) {
    output_path = file_path.parent_path();
  } else {
    output_path = options_->output_dir;
  }

  pantheios::log(pantheios::informational,
                 "Generating output files in " + output_path.string() + " ...");
  boost::filesystem::create_directories(output_path);
  output_path /= file_path.stem();

  diffingo::generation::output::CodeGenerator codeGen(output_path.string(),
                                                      options_->name_space);
  codeGen.run(m, *options_);

  pantheios::log(pantheios::informational, "Complete.");
  return 0;
}

bool Compiler::executePasses(node_ptr<spec::ast::Module> module) {
  diffingo::generation::preprocessing::ScopeBuilder scope_builder;
  diffingo::generation::preprocessing::UnitScopeBuilder unit_scope_builder;
  diffingo::generation::preprocessing::IdResolver id_resolver;
  diffingo::generation::preprocessing::TransformResolver transform_resolver;
  diffingo::generation::preprocessing::InstantiationDependencyResolver
      inst_dep_resolver;
  diffingo::generation::preprocessing::InstantiationTypeGenerator
      inst_type_generator;

  if (!executePass(module, &scope_builder)) return false;
  if (!executePass(module, &id_resolver)) return false;
  if (!executePass(module, &unit_scope_builder)) return false;
  if (!executePass(module, &id_resolver)) return false;
  if (!executePass(module, &transform_resolver)) return false;
  if (!executePass(module, &inst_dep_resolver)) return false;
  if (!executePass(module, &inst_type_generator)) return false;

  id_resolver.set_report_unresolved();
  if (!executePass(module, &id_resolver)) return false;

  return true;
}

template <typename Result, typename Arg1, typename Arg2>
bool Compiler::executePass(node_ptr<spec::ast::Module> module,
                           generation::Pass<Result, Arg1, Arg2>* pass) {
  pantheios::log(pantheios::informational,
                 util::fmt("Running %s ...", util::type_name(*pass)));
  return pass->run(module, *options_);
}

}  // namespace generation
}  // namespace diffingo
