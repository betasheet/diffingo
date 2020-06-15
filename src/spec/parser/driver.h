/*
 * driver.h
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

#ifndef SRC_SPEC_PARSER_DRIVER_H_
#define SRC_SPEC_PARSER_DRIVER_H_

#include <pantheios/pantheios.hpp>
#include <memory>
#include <list>
#include <string>
#include <utility>

#include "spec/ast/node.h"
#include "spec/ast/declaration/declaration.h"
#include "spec/ast/ctor/reg_exp.h"
#include "generation/compiler_context.h"

#undef YY_DECL
#define YY_DECL                                                    \
  diffingo::spec::Parser::token_type diffingo::spec::Scanner::lex( \
      diffingo::spec::Parser::semantic_type* yylval,               \
      diffingo::spec::Parser::location_type* yylloc,               \
      diffingo::spec::Driver* driver)

#ifndef __FLEX_LEXER_H
#define yyFlexLexer DiffingoFlexLexer
#include "FlexLexer.h"
#undef yyFlexLexer
#endif

using diffingo::spec::ast::node_ptr;

// possible semantic types
// TODO(Eric Seckler): update these to fit diffingo AST types
struct yystype_diffingo {
  bool bval;
  double dval;
  int64_t ival;
  std::string sval;

  /* disabled for now */
  /*
   diffingo::spec::ast::ctor::Map::element map_element;
   diffingo::spec::ast::map_element_list map_elements;
   */

  diffingo::spec::ast::declaration::Declaration::Linkage linkage;

  node_ptr<diffingo::spec::ast::Attribute> attribute;
  node_ptr<diffingo::spec::ast::constant::Constant> constant;
  node_ptr<diffingo::spec::ast::ctor::Ctor> ctor;
  node_ptr<diffingo::spec::ast::declaration::Declaration> declaration;
  node_ptr<diffingo::spec::ast::declaration::unit_instantiation::Item>
      inst_item;
  node_ptr<diffingo::spec::ast::expression::Expression> expression;
  node_ptr<diffingo::spec::ast::ID> id;
  node_ptr<diffingo::spec::ast::Module> module;
  node_ptr<diffingo::spec::ast::type::Type> type;
  node_ptr<diffingo::spec::ast::type::function::Parameter> parameter;
  node_ptr<diffingo::spec::ast::type::function::Result> result;
  node_ptr<diffingo::spec::ast::type::unit::item::Item> unit_item;
  node_ptr<diffingo::spec::ast::type::unit::item::field::Field> unit_field;
  node_ptr<diffingo::spec::ast::type::unit::item::field::switch_::Case>
      switch_case;  // NOLINT
  node_ptr<diffingo::spec::ast::type::bitfield::Bits> bits_spec;

  diffingo::spec::ast::attribute_list attributes;
  diffingo::spec::ast::declaration_list declarations;
  diffingo::spec::ast::expression_list expressions;
  diffingo::spec::ast::parameter_list parameters;

  diffingo::spec::ast::id_list ids;
  diffingo::spec::ast::type_list types;
  diffingo::spec::ast::inst_item_list inst_items;
  diffingo::spec::ast::unit_item_list unit_items;
  diffingo::spec::ast::unit_field_list unit_fields;
  diffingo::spec::ast::switch_case_list switch_cases;
  diffingo::spec::ast::bitfield_bits_list bits;
  diffingo::spec::ast::ctor::RegExp::pattern_list re_patterns;

  std::list<std::pair<node_ptr<diffingo::spec::ast::ID>, int>> id_and_ints;

  std::pair<node_ptr<diffingo::spec::ast::type::Type>,
            node_ptr<diffingo::spec::ast::expression::Expression>>
      type_and_expr;
  std::pair<node_ptr<diffingo::spec::ast::ID>, int> id_and_int;
};

#define YYSTYPE yystype_diffingo

namespace diffingo {
namespace spec {

class Parser;
class Scanner;
class location;

class Driver {
 public:
  Driver();
  virtual ~Driver();

  node_ptr<ast::Module> parse(generation::CompilerContext* ctx,
                              std::istream* in, const std::string& stream_name);

  // The Bison parser needs a non-const pointer here. Grmpf.
  std::string* stream_name() { return &stream_name_; }

  Scanner* scanner() const { return scanner_; }
  Parser* parser() const { return parser_; }
  generation::CompilerContext* context() const { return context_; }

  node_ptr<ast::Module> module() const;
  void set_module(node_ptr<ast::Module> module);

  void include(std::string filename);
  std::list<std::string> includes() { return includes_; }

  void enablePatternMode();
  void disablePatternMode();

  void error(const std::string& m, const location& l);

 private:
  std::string stream_name_;
  generation::CompilerContext* context_ = nullptr;
  node_ptr<ast::Module> module_ = nullptr;

  Scanner* scanner_ = nullptr;
  Parser* parser_ = nullptr;
  int errors_ = 0;
  std::list<std::string> includes_;
};

}  // namespace spec
}  // namespace diffingo

#endif  // SRC_SPEC_PARSER_DRIVER_H_
