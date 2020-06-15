/*
 * parser.yy
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

%{ /*** C/C++ Declarations ***/

#include <stdio.h>
#include <string>
#include <memory>

#undef yylex

#include "spec/parser/driver.h"
#include "spec/parser/scanner.h"

#include "spec/ast/attribute.h"
#include "spec/ast/function.h"
#include "spec/ast/id.h"
#include "spec/ast/location.h"
#include "spec/ast/module.h"
#include "spec/ast/constant/constant.h"
#include "spec/ast/constant/bool.h"
#include "spec/ast/constant/double.h"
#include "spec/ast/constant/integer.h"
#include "spec/ast/constant/none.h"
#include "spec/ast/constant/string.h"
#include "spec/ast/constant/tuple.h"
#include "spec/ast/ctor/ctor.h"
#include "spec/ast/ctor/bytes.h"
#include "spec/ast/ctor/reg_exp.h"
#include "spec/ast/declaration/declaration.h"
#include "spec/ast/declaration/constant.h"
#include "spec/ast/declaration/function.h"
#include "spec/ast/declaration/type.h"
#include "spec/ast/declaration/unit_instantiation.h"
#include "spec/ast/expression/expression.h"
#include "spec/ast/expression/assign.h"
#include "spec/ast/expression/conditional.h"
#include "spec/ast/expression/constant.h"
#include "spec/ast/expression/ctor.h"
#include "spec/ast/expression/find.h"
#include "spec/ast/expression/function.h"
#include "spec/ast/expression/id.h"
#include "spec/ast/expression/lambda.h"
#include "spec/ast/expression/list_comprehension.h"
#include "spec/ast/expression/member_attribute.h"
#include "spec/ast/expression/operator.h"
#include "spec/ast/expression/type.h"
#include "spec/ast/type/type.h"
#include "spec/ast/type/atomic_types.h"
#include "spec/ast/type/bitfield.h"
#include "spec/ast/type/bitset.h"
#include "spec/ast/type/container.h"
#include "spec/ast/type/enum.h"
#include "spec/ast/type/function.h"
#include "spec/ast/type/unit.h"

#define yylex driver->scanner()->lex

namespace diffingo {
namespace spec {

using ast::node_ptr;
using ast::newNodePtr;

inline ast::Location loc(const Parser::location_type& ploc) {
    if (ploc.begin.filename == ploc.end.filename)
        return ast::Location(*ploc.begin.filename, ploc.begin.line, ploc.end.line);
    else
        return ast::Location(*ploc.begin.filename, ploc.begin.line);
}

inline node_ptr<ast::expression::Expression> makeOp(ast::expression::Operator::Kind kind, const ast::expression_list& ops, const ast::Location& l) {
    return newNodePtr(std::make_shared<ast::expression::Operator>(kind, ops, l));
}

inline node_ptr<ast::type::unit::item::field::Field> makeVectorField(node_ptr<ast::type::unit::item::field::Field> elem,
                                                           node_ptr<ast::ID> name,
                                                           node_ptr<ast::expression::Expression> length,
                                                           const ast::Location& l) {
    return newNodePtr(std::make_shared<ast::type::unit::item::field::container::Vector>(name, elem, length, nullptr, ast::attribute_list(), ast::expression_list(), l));
}

}  // namespace spec
}  // namespace diffingo

static int _int_width = 0;

%}

/*** yacc/bison Declarations ***/

// Require bison 2.3 or later
%require "2.3"

// add debug output code to generated parser. disable this for release versions.
%debug

// start symbol
%start start

// write out a header file containing the token defines
%defines

// use newer C++ skeleton file
%skeleton "lalr1.cc"

// namespace to enclose parser in
%name-prefix "diffingo::spec"

// set the parser's class identifier
%define parser_class_name {Parser}

// keep track of the current position within the input
%locations
%initial-action
{
    // initialize the initial location object
    @$.begin.filename = @$.end.filename = driver->stream_name();
};

// The driver is passed to the parser and to the scanner.
%parse-param { class Driver* driver }
%lex-param   { class Driver* driver }

// verbose messages
%error-verbose
%verbose

// token definitions
%token <sval>  SCOPED_IDENT   "scoped identifier"
%token <sval>  DOLLAR_IDENT   "$-identifier"
%token <sval>  PATH_IDENT     "path"
%token <sval>  IDENT          "identifier"
%token <sval>  ATTRIBUTE      "attribute"
%token <sval>  PROPERTY       "property"

%token <ival>  CINTEGER       "integer"
%token <bval>  CBOOL          "bool"
%token <sval>  CSTRING        "string"
%token <sval>  CBYTES         "bytes"
%token <sval>  CREGEXP        "regular expression"
%token <dval>  CDOUBLE        "double"

%token <sval>  CPPCODE        "cpp code"

%token         END            0 "end of file"

%token         ANY
%token         AS
%token         BITFIELD
%token         BITSET
%token         BOOL
%token         BYTES
%token         CADDR
%token         CAST
%token         CONST
%token         DOUBLE
%token         ELSE
%token         ENUM
%token         EXPORT
%token         FOR
%token         FIND
%token         IF
%token         IMPORT
%token         IN
%token         INCLUDE
%token         INSTANTIATE
%token         INT
%token         INT8
%token         INT16
%token         INT32
%token         INT64
%token         LIST
%token         MAP
%token         MOD
%token         MODULE
%token         SET
%token         SINK
%token         STRING
%token         SWITCH
%token         THEN
%token         TUPLE
%token         TYPE
%token         UINT
%token         UINT8
%token         UINT16
%token         UINT32
%token         UINT64
%token         UNIT
%token         VAR
%token         VECTOR
%token         VOID

%token         AND
%token         ARROW
%token         BEGINCODE
%token         DARROW
%token         DOTDOT
%token         EQ
%token         ENDCODE
%token         GEQ
%token         HASATTR
%token         LEQ
%token         MINUSASSIGN
%token         MINUSMINUS
%token         NEQ
%token         OR
%token         PLUSASSIGN
%token         PLUSPLUS
%token         POW
%token         SHIFTLEFT
%token         SHIFTRIGHT

%token         NONE

%token         PREC_HIGH;  // pseudo token for easy maximum precedence grammar rules 

// type definitions
%type <bits>             bitfield_bits
%type <bits_spec>        bitfield_bits_spec
%type <constant>         constant tuple
%type <ctor>             ctor
%type <declaration>      global_decl type_decl const_decl func_decl inst_decl
%type <declarations>     opt_global_decls
%type <expression>       expr expr2 opt_unit_field_cond opt_init_expr init_expr member_expr tuple_expr opt_unit_vector_len opt_unit_switch_expr
%type <expressions>      exprs opt_exprs opt_unit_field_sinks opt_field_args
%type <id>               local_id scoped_id path_id opt_unit_field_name /* dollar_id */
%type <type>             base_type type enum_ bitset unit atomic_type container_type bitfield
%type <type_and_expr>    type_or_init
%type <id_and_int>       id_with_int
%type <id_and_ints>      id_with_ints
%type <bval>             opt_param_const
%type <parameter>        param
%type <result>           rtype
%type <parameters>       params opt_params opt_unit_params
%type <unit_item>        unit_item unit_prop unit_var unit_switch
%type <unit_items>       unit_items opt_unit_items
%type <unit_field>       unit_field unit_field_in_container
%type <unit_fields>      unit_fields
%type <inst_item>        inst_item inst_prop inst_field
%type <inst_items>       inst_items opt_inst_items
%type <linkage>          opt_linkage
%type <attribute>        type_attr property
%type <attributes>       opt_type_attrs
%type <switch_case>      unit_switch_case
%type <switch_cases>     unit_switch_cases
%type <re_patterns>      re_patterns
%type <sval>             re_pattern_constant cppblock
%type <types>            types
/* %type <map_element>      map_elem */
/* %type <map_elements>     map_elems opt_map_elems */

%left DARROW;
%left OR AND;
%left EQ NEQ LEQ GEQ '<' '>';
%left '+' '-';

// All keywords introducing constants must be listed here before PREC_HIGH.
%left INT INT8 INT16 INT32 INT64;
%left UINT UINT8 UINT16 UINT32 UINT64;
%left PREC_HIGH;


%% /*** Grammar Rules ***/


start         : module

module        : MODULE local_id ';'              { auto module = newNodePtr(std::make_shared<ast::Module>(driver->context(), $2, *driver->stream_name(), loc(@$)));
                                                   driver->set_module(module);
                                                 }

                opt_global_decls                 { driver->module()->addDeclarations($5); }

local_id      : IDENT                            { $$ = newNodePtr(std::make_shared<ast::ID>($1, loc(@$))); }

scoped_id     : IDENT                            { $$ = newNodePtr(std::make_shared<ast::ID>($1, loc(@$))); }
              | SCOPED_IDENT                     { $$ = newNodePtr(std::make_shared<ast::ID>($1, loc(@$))); }

path_id       : IDENT                            { $$ = newNodePtr(std::make_shared<ast::ID>($1, loc(@$))); }
              | PATH_IDENT                       { $$ = newNodePtr(std::make_shared<ast::ID>($1, loc(@$))); }

    /* not currently neccessary */
    // dollar_id     : DOLLAR_IDENT              { $$ = newNodePtr(std::make_shared<ast::ID>($1, loc(@$))); }

property      : PROPERTY ';'                     { $$ = newNodePtr(std::make_shared<ast::Attribute>($1, nullptr, loc(@$))); }
              | PROPERTY '=' expr ';'            { $$ = newNodePtr(std::make_shared<ast::Attribute>($1, $3, loc(@$))); }
              | PROPERTY '=' NONE ';'            { $$ = newNodePtr(std::make_shared<ast::Attribute>($1, nullptr, loc(@$))); }
              | PROPERTY '=' base_type ';'       { auto e = newNodePtr(std::make_shared<ast::expression::Type>($3, loc(@$)));
                                                   $$ = newNodePtr(std::make_shared<ast::Attribute>($1, e, loc(@$)));
                                                 }

opt_global_decls
              : global_decl opt_global_decls     { $$ = $2; if ( $1 ) $$.push_front($1); }
              | /* empty */                      { $$ = ast::declaration_list(); }

global_decl   : type_decl                        { $$ = $1; }
              | inst_decl                        { $$ = $1; }
              | const_decl                       { $$ = $1; }
              | func_decl                        { $$ = $1; }

              | property                         { driver->module()->addProperty($1); }
              | import                           { $$ = nullptr; }
              | include                          { $$ = nullptr; }

const_decl    : opt_linkage CONST local_id type_or_init ';'
                                                 { auto type = $4.first;
                                                   auto init = $4.second;

                                                   if ( init->canCoerceTo(type) )
                                                       init = init->coerceTo(type);
                                                   else {
                                                       error(@$, "cannot coerce init expression to type");
                                                       type = init->type();
                                                   }

                                                   $$ = newNodePtr(std::make_shared<ast::declaration::Constant>($3, $1, init, loc(@$)));
                                                 }

opt_linkage   : EXPORT                           { $$ = ast::declaration::Declaration::Linkage::EXPORTED; }
              | /* empty */ %prec PREC_HIGH      { $$ = ast::declaration::Declaration::Linkage::PRIVATE; }


type_or_init  : ':' base_type init_expr          { $$ = std::make_pair($2, $3); }
              | init_expr                        { $$ = std::make_pair($1->type(), $1); }

opt_init_expr : init_expr                        { $$ = $1; }
              | /* empty */                      { $$ = nullptr; }

init_expr     : '=' expr                         { $$ = $2; }

import        : IMPORT path_id ';'               { driver->module()->import($2); }

include       : INCLUDE CSTRING ';'              { driver->include($2); }

type_decl     : opt_linkage TYPE local_id '=' type ';'
                                                 { $$ = newNodePtr(std::make_shared<ast::declaration::Type>($3, $1, $5, loc(@$)));

                                                   if ( $1 == ast::declaration::Declaration::Linkage::EXPORTED )
                                                       driver->module()->exportType($5);
                                                 }

inst_decl     : INSTANTIATE UNIT scoped_id AS local_id '{' opt_inst_items '}' ';'
                                                 { $$ = newNodePtr(std::make_shared<ast::declaration::unit_instantiation::Instantiation>($5, $3, $7, loc(@$)));
                                                 }

func_decl     : opt_linkage rtype local_id '(' opt_params ')' cppblock
                                                 { auto ftype = newNodePtr(std::make_shared<ast::type::function::Function>($2, $5, loc(@$)));
                                                   auto func  = newNodePtr(std::make_shared<ast::Function>($3, ftype, driver->module(), $7, loc(@$)));
                                                   $$ = newNodePtr(std::make_shared<ast::declaration::Function>(func, $1, loc(@$)));
                                                 }

              | IMPORT rtype scoped_id '(' opt_params ')' ';'
                                                 { auto ftype = newNodePtr(std::make_shared<ast::type::function::Function>($2, $5, loc(@$)));
                                                   auto func  = newNodePtr(std::make_shared<ast::Function>($3, ftype, driver->module(), nullptr, loc(@$)));
                                                   $$ = newNodePtr(std::make_shared<ast::declaration::Function>(func, ast::declaration::Declaration::Linkage::IMPORTED, loc(@$)));
                                                 }

params        : param ',' params                 { $$ = $3; $$.push_front($1); }
              | param                            { $$ = ast::parameter_list(); $$.push_back($1); }

opt_params    : params                           { $$ = $1; }
              | /* empty */                      { $$ = ast::parameter_list(); }

param         : opt_param_const local_id ':' type
                                                 { $$ = newNodePtr(std::make_shared<ast::type::function::Parameter>($2, $4, $1, nullptr, loc(@$))); }

rtype         : opt_param_const type             { $$ = newNodePtr(std::make_shared<ast::type::function::Result>($2, $1, loc(@$))); }

opt_param_const
              : CONST                            { $$ = true; }
              | /* empty */                      { $$ = false; }

cppblock      : BEGINCODE CPPCODE ENDCODE        { $$ = $2; }

type          : base_type                        { $$ = $1; }
              | scoped_id                        { $$ = newNodePtr(std::make_shared<ast::type::Unknown>($1, loc(@$))); }

base_type     : atomic_type                      { $$ = $1; }
              | container_type                   { $$ = $1; }

atomic_type   : ANY                              { $$ = newNodePtr(std::make_shared<ast::type::Any>(loc(@$))); }
              | BOOL                             { $$ = newNodePtr(std::make_shared<ast::type::Bool>(loc(@$))); }
              | BYTES                            { $$ = newNodePtr(std::make_shared<ast::type::Bytes>(loc(@$))); }
              | CADDR                            { $$ = newNodePtr(std::make_shared<ast::type::CAddr>(loc(@$))); }
              | DOUBLE                           { $$ = newNodePtr(std::make_shared<ast::type::Double>(loc(@$))); }
              | SINK                             { $$ = newNodePtr(std::make_shared<ast::type::Sink>(loc(@$))); }
              | STRING                           { $$ = newNodePtr(std::make_shared<ast::type::String>(loc(@$))); }
              | VOID                             { $$ = newNodePtr(std::make_shared<ast::type::Void>(loc(@$))); }

              | INT '<' CINTEGER '>'             { $$ = newNodePtr(std::make_shared<ast::type::Integer>($3, true, loc(@$))); }
              | INT8                             { $$ = newNodePtr(std::make_shared<ast::type::Integer>(8, true, loc(@$))); }
              | INT16                            { $$ = newNodePtr(std::make_shared<ast::type::Integer>(16, true, loc(@$))); }
              | INT32                            { $$ = newNodePtr(std::make_shared<ast::type::Integer>(32, true, loc(@$))); }
              | INT64                            { $$ = newNodePtr(std::make_shared<ast::type::Integer>(64, true, loc(@$))); }

              | UINT '<' CINTEGER '>'            { $$ = newNodePtr(std::make_shared<ast::type::Integer>($3, false, loc(@$))); }
              | UINT8                            { $$ = newNodePtr(std::make_shared<ast::type::Integer>(8, false, loc(@$))); }
              | UINT16                           { $$ = newNodePtr(std::make_shared<ast::type::Integer>(16, false, loc(@$))); }
              | UINT32                           { $$ = newNodePtr(std::make_shared<ast::type::Integer>(32, false, loc(@$))); }
              | UINT64                           { $$ = newNodePtr(std::make_shared<ast::type::Integer>(64, false, loc(@$))); }

              | TUPLE '<' types '>'              { $$ = newNodePtr(std::make_shared<ast::type::Tuple>($3, loc(@$))); }
              | TUPLE '<' '*' '>'                { $$ = newNodePtr(std::make_shared<ast::type::Tuple>(loc(@$))); }

              | bitfield                         { $$ = $1; }
              | bitset                           { $$ = $1; }
              | enum_                            { $$ = $1; }
              | unit                             { $$ = $1; }

container_type:
                LIST '<' type '>'                { $$ = newNodePtr(std::make_shared<ast::type::List>($3, loc(@$))); }
              | MAP '<' type ',' type '>'        { $$ = newNodePtr(std::make_shared<ast::type::Map>($3, $5, loc(@$))); }
              | SET  '<' type '>'                { $$ = newNodePtr(std::make_shared<ast::type::Set>($3, loc(@$))); }
              | VECTOR '<' type '>'              { $$ = newNodePtr(std::make_shared<ast::type::Vector>($3, loc(@$))); }

bitset        : BITSET '{' id_with_ints '}'      { $$ = newNodePtr(std::make_shared<ast::type::Bitset>($3, loc(@$))); }

enum_         : ENUM '{' id_with_ints '}'        { $$ = newNodePtr(std::make_shared<ast::type::Enum>($3, loc(@$))); }

types         : type ',' types                   { $$ = $3; $$.push_front($1); }
              | type                             { $$ = ast::type_list(); $$.push_back($1); }

id_with_ints  : id_with_ints ',' id_with_int     { $$ = $1; $$.push_back($3); }
              | id_with_int                      { $$ = std::list<std::pair<node_ptr<ast::ID>, int>>(); $$.push_back($1); }

id_with_int   : local_id                         { $$ = std::make_pair($1, -1); }
              | local_id '=' CINTEGER            { $$ = std::make_pair($1, $3); }

bitfield      : BITFIELD '(' CINTEGER ')'
                                                 { _int_width = $3; }
                '{' bitfield_bits '}'
                                                 { $$ = newNodePtr(std::make_shared<ast::type::bitfield::Bitfield>($3, $7, loc(@$))); }

bitfield_bits:  bitfield_bits_spec bitfield_bits { $$ = $2; $$.push_front($1); }
              | /* empty */                      { $$ = ast::bitfield_bits_list(); }

bitfield_bits_spec
              : local_id ':' CINTEGER DOTDOT CINTEGER opt_type_attrs ';'
                                                 { $$ = newNodePtr(std::make_shared<ast::type::bitfield::Bits>($1, $3, $5, _int_width, $6, loc(@$))); }
              | local_id ':' CINTEGER opt_type_attrs ';'
                                                 { $$ = newNodePtr(std::make_shared<ast::type::bitfield::Bits>($1, $3, $3, _int_width, $4, loc(@$))); }

opt_unit_params
              : '(' opt_params ')'               { $$ = $2; }
              | /* empty */                      { $$ = ast::parameter_list(); }

unit          : UNIT opt_unit_params '{' opt_unit_items '}'
                                                 { $$ = newNodePtr(std::make_shared<ast::type::unit::Unit>($2, $4, loc(@$))); }

unit_items    : unit_item unit_items             { $$ = $2; $$.push_front($1); }
              | unit_item                        { $$ = ast::unit_item_list(); $$.push_back($1); }

opt_unit_items: unit_items                       { $$ = $1;}
              | /* empty */                      { $$ = ast::unit_item_list(); }


unit_item     : unit_var                         { $$ = $1; }
              | unit_field                       { $$ = $1; }
              | unit_switch                      { $$ = $1; }
              | unit_prop                        { $$ = $1; }

unit_var      : VAR local_id ':' type opt_init_expr opt_type_attrs ';'
                                                 { $$ = newNodePtr(std::make_shared<ast::type::unit::item::Variable>($2, $4, $5, $6, loc(@$)));
                                                 }

unit_prop     : property                         { $$ = newNodePtr(std::make_shared<ast::type::unit::item::Property>($1, loc(@$))); }

unit_field    : opt_unit_field_name base_type opt_field_args opt_unit_vector_len opt_type_attrs opt_unit_field_cond opt_unit_field_sinks ';'
                                                 { $$ = ast::type::unit::item::field::Field::createByType($2, $1, $6, $5, $3, $7, loc(@$));
                                                   if ( $4 )
                                                       $$ = makeVectorField($$, $1, $4, loc(@$));
                                                 }

              | opt_unit_field_name constant opt_unit_vector_len opt_type_attrs opt_unit_field_cond opt_unit_field_sinks ';'
                                                 { $$ = newNodePtr(std::make_shared<ast::type::unit::item::field::Constant>($1, $2, $5, $4, $6, loc(@$)));
                                                   if ( $3 ) $$ = makeVectorField($$, $1, $3, loc(@$));
                                                 }

              | opt_unit_field_name ctor opt_unit_vector_len opt_type_attrs opt_unit_field_cond opt_unit_field_sinks ';'
                                                 { $$ = newNodePtr(std::make_shared<ast::type::unit::item::field::Ctor>($1, $2, $5, $4, $6, loc(@$)));
                                                   if ( $3 ) $$ = makeVectorField($$, $1, $3, loc(@$));
                                                 }

              | opt_unit_field_name LIST '<' unit_field_in_container '>' opt_unit_vector_len opt_type_attrs opt_unit_field_cond opt_unit_field_sinks ';'
                                                 { $$ = newNodePtr(std::make_shared<ast::type::unit::item::field::container::List>($1, $4, $8, $7, $9, loc(@$)));
                                                  if ( $6 ) $$ = makeVectorField($$, $1, $6, loc(@$));
                                                 }

              | opt_unit_field_name scoped_id opt_field_args opt_unit_vector_len opt_type_attrs opt_unit_field_cond opt_unit_field_sinks ';'
                                                 { $$ = newNodePtr(std::make_shared<ast::type::unit::item::field::Unknown>($1, $2, $6, $5, $3, $7, loc(@$)));
                                                  if ( $4 ) $$ = makeVectorField($$, $1, $4, loc(@$));
                                                 }

unit_field_in_container :
                local_id opt_field_args          { $$ = newNodePtr(std::make_shared<ast::type::unit::item::field::Unknown>(nullptr, $1, nullptr, ast::attribute_list(), $2, ast::expression_list(), loc(@$))); }

unit_fields   : unit_field unit_fields         { $$ = $2; $$.push_front($1); }
              | unit_field                     { $$ = ast::unit_field_list(); $$.push_back($1); }

unit_switch   : SWITCH opt_unit_switch_expr '{' unit_switch_cases '}' opt_unit_field_cond ';'
                                                 { $$ = newNodePtr(std::make_shared<ast::type::unit::item::field::switch_::Switch>($2, $4, $6, loc(@$))); }

opt_unit_switch_expr: '(' expr ')'               { $$ = $2; }
              | /* empty */                      { $$ = nullptr; }

unit_switch_cases
              : unit_switch_case unit_switch_cases
                                                 { $$ = $2; $$.push_front($1); }
              | unit_switch_case                 { $$ = ast::switch_case_list(); $$.push_back($1); }

unit_switch_case
              : exprs ARROW '{' unit_fields '}'  { $$ = newNodePtr(std::make_shared<ast::type::unit::item::field::switch_::Case>($1, $4, loc(@$))); }
              | '*'   ARROW '{' unit_fields '}'  { $$ = newNodePtr(std::make_shared<ast::type::unit::item::field::switch_::Case>($4, loc(@$))); }
              | exprs ARROW unit_field           { $$ = newNodePtr(std::make_shared<ast::type::unit::item::field::switch_::Case>($1, $3, loc(@$))); }
              | '*'   ARROW unit_field           { $$ = newNodePtr(std::make_shared<ast::type::unit::item::field::switch_::Case>($3, loc(@$))); }
              | unit_field                       { $$ = newNodePtr(std::make_shared<ast::type::unit::item::field::switch_::Case>(ast::expression_list(), $1, loc(@$))); }


opt_type_attrs: type_attr opt_type_attrs         { $$ = $2; $$.push_front($1); }
              | /* empty */                      { $$ = ast::attribute_list(); }

type_attr     : ATTRIBUTE '(' expr ')'           { $$ = newNodePtr(std::make_shared<ast::Attribute>($1, $3, loc(@$))); }
              | ATTRIBUTE '=' expr               { $$ = newNodePtr(std::make_shared<ast::Attribute>($1, $3, loc(@$))); }
              | ATTRIBUTE                        { $$ = newNodePtr(std::make_shared<ast::Attribute>($1, nullptr, loc(@$))); }

opt_field_args: '(' opt_exprs ')'                { $$ = $2; }
              | /* empty */                      { $$ = ast::expression_list(); }

opt_unit_field_name
              : local_id  ':'                    { $$ = $1; }
              | ':'                              { $$ = nullptr; }

opt_unit_field_cond
              : IF '(' expr ')'                  { $$ = $3; }
              | /* empty */                      { $$ = nullptr; }

opt_unit_field_sinks
              : ARROW exprs                      { $$ = $2; }
              | /* empty */                      { $$ = ast::expression_list(); }

opt_unit_vector_len
              : '[' expr ']'                     { $$ = $2; }
              | /* empty */                      { $$ = nullptr; }

inst_items    : inst_item inst_items             { $$ = $2; $$.push_front($1); }
              | inst_item                        { $$ = ast::inst_item_list(); $$.push_back($1); }

opt_inst_items: inst_items                       { $$ = $1;}
              | /* empty */                      { $$ = ast::inst_item_list(); }

inst_item     : inst_field                       { $$ = $1; }
              | inst_prop                        { $$ = $1; }

inst_field    : local_id opt_type_attrs ';'      { $$ = newNodePtr(std::make_shared<ast::declaration::unit_instantiation::Field>($1, $2, false, loc(@$))); }

inst_prop     : property                         { $$ = newNodePtr(std::make_shared<ast::declaration::unit_instantiation::Property>($1, loc(@$))); }

              /* When adding rules here, add leading keywords to the precedence declaration above. */
constant      : NONE                             { $$ = newNodePtr(std::make_shared<ast::constant::None>(loc(@$))); }
              | CINTEGER                         { $$ = newNodePtr(std::make_shared<ast::constant::Integer>($1, 64, loc(@$))); }
              | UINT '<' CINTEGER '>' '(' CINTEGER ')'
                                                 { $$ = newNodePtr(std::make_shared<ast::constant::Integer>($6, $3, false, loc(@$))); }
              | UINT8 '(' CINTEGER ')'           { $$ = newNodePtr(std::make_shared<ast::constant::Integer>($3, 8, false, loc(@$))); }
              | UINT16 '(' CINTEGER ')'          { $$ = newNodePtr(std::make_shared<ast::constant::Integer>($3, 16, false, loc(@$))); }
              | UINT32 '(' CINTEGER ')'          { $$ = newNodePtr(std::make_shared<ast::constant::Integer>($3, 32, false, loc(@$))); }
              | UINT64 '(' CINTEGER ')'          { $$ = newNodePtr(std::make_shared<ast::constant::Integer>($3, 64, false, loc(@$))); }

              | INT '<' CINTEGER '>' '(' CINTEGER ')'
                                                 { $$ = newNodePtr(std::make_shared<ast::constant::Integer>($6, $3, true, loc(@$))); }
              | INT8 '(' CINTEGER ')'            { $$ = newNodePtr(std::make_shared<ast::constant::Integer>($3, 8, true, loc(@$))); }
              | INT16 '(' CINTEGER ')'           { $$ = newNodePtr(std::make_shared<ast::constant::Integer>($3, 16, true, loc(@$))); }
              | INT32 '(' CINTEGER ')'           { $$ = newNodePtr(std::make_shared<ast::constant::Integer>($3, 32, true, loc(@$))); }
              | INT64 '(' CINTEGER ')'           { $$ = newNodePtr(std::make_shared<ast::constant::Integer>($3, 64, true, loc(@$))); }

              | CBOOL                            { $$ = newNodePtr(std::make_shared<ast::constant::Bool>($1, loc(@$))); }
              | CDOUBLE                          { $$ = newNodePtr(std::make_shared<ast::constant::Double>($1, loc(@$))); }
              | CSTRING                          { $$ = newNodePtr(std::make_shared<ast::constant::String>($1, loc(@$))); }
              | tuple                            { $$ = $1; }

tuple         : '(' opt_exprs ')'                { $$ = newNodePtr(std::make_shared<ast::constant::Tuple>($2, loc(@$))); }

ctor          : CBYTES                           { $$ = newNodePtr(std::make_shared<ast::ctor::Bytes>($1, loc(@$))); }
              | re_patterns                      { $$ = newNodePtr(std::make_shared<ast::ctor::RegExp>($1, ast::attribute_list(), loc(@$))); }

    /* other constructors disabled for now */
    //          | LIST '(' opt_exprs ')'           { $$ = newNodePtr(std::make_shared<ast::ctor::List>(nullptr, $3, loc(@$))); }
    //          | LIST '<' type '>' '(' opt_exprs ')'
    //                                             { $$ = newNodePtr(std::make_shared<ast::ctor::List>($3, $6, loc(@$))); }

    //          | SET '(' opt_exprs ')'            { $$ = newNodePtr(std::make_shared<ast::ctor::Set>(nullptr, $3, loc(@$))); }
    //          | SET '<' type '>' '(' opt_exprs ')'
    //                                             { $$ = newNodePtr(std::make_shared<ast::ctor::Set>($3, $6, loc(@$))); }

    //          | MAP '(' opt_map_elems ')'        { $$ = newNodePtr(std::make_shared<ast::ctor::Map>(nullptr, nullptr, $3, loc(@$))); }
    //          | MAP '<' type ',' type '>' '(' opt_map_elems ')'
    //                                             { $$ = newNodePtr(std::make_shared<ast::ctor::Map>($3, $5, $8, loc(@$))); }

    //          | VECTOR '(' opt_exprs ')'         { $$ = newNodePtr(std::make_shared<ast::ctor::Vector>(nullptr, $3, loc(@$))); }
    //          | VECTOR '<' type '>' '(' opt_exprs ')'
    //                                             { $$ = newNodePtr(std::make_shared<ast::ctor::Vector>($3, $6, loc(@$))); }

    /* map constructors disabled for now */
    // opt_map_elems : map_elems                        { $$ = $1; }
    //          | /* empty */                      { $$ = ast::map_element_list(); }

    // map_elems     : map_elems ',' map_elem           { $$ = $1; $$.push_back($3); }
    //          | map_elem                         { $$ = ast::map_element_list(); $$.push_back($1); }

    // map_elem      : expr ':' expr                    { $$ = ast::ctor::Map::element($1, $3); }

re_patterns   : re_patterns '|' re_pattern_constant { $$ = $1; $$.push_back($3); }
              | re_pattern_constant                 { $$ = ast::ctor::RegExp::pattern_list(); $$.push_back($1); }

re_pattern_constant
              : '/' { driver->enablePatternMode(); } CREGEXP { driver->disablePatternMode(); } '/' { $$ = $3; }

expr          : expr2                            { /*driver->set_expression($1);*/ }

expr2         : scoped_id                        { $$ = newNodePtr(std::make_shared<ast::expression::ID>($1, loc(@$))); }
              | '(' expr ')'                     { $$ = $2; }
              | ctor                             { $$ = newNodePtr(std::make_shared<ast::expression::Ctor>($1, loc(@$))); }
              | constant                         { $$ = newNodePtr(std::make_shared<ast::expression::Constant>($1, loc(@$))); }
              | expr '=' expr                    { $$ = newNodePtr(std::make_shared<ast::expression::Assign>($1, $3, loc(@$))); }
              | expr '?' expr                    { $$ = newNodePtr(std::make_shared<ast::expression::Conditional>($1, $3, nullptr, loc(@$))); }
              | expr '?' expr ':' expr           { $$ = newNodePtr(std::make_shared<ast::expression::Conditional>($1, $3, $5, loc(@$))); }
              | IF expr THEN expr                { $$ = newNodePtr(std::make_shared<ast::expression::Conditional>($2, $4, nullptr, loc(@$))); }
              | IF expr THEN expr ELSE expr      { $$ = newNodePtr(std::make_shared<ast::expression::Conditional>($2, $4, $6, loc(@$))); }
              | '[' expr FOR local_id IN expr ']'{ $$ = newNodePtr(std::make_shared<ast::expression::ListComprehension>($2, $4, $6, nullptr, loc(@$))); }
              | '[' expr FOR local_id IN expr IF expr ']'
                                                 { $$ = newNodePtr(std::make_shared<ast::expression::ListComprehension>($2, $4, $6, $8, loc(@$))); }
              | expr DARROW expr                 { auto id_expr = ast::tryCast<ast::expression::ID>($1);
                                                   if (! id_expr) {
                                                     error(@$, "lambda expression requires identifier as first parameter");
                                                     id_expr = newNodePtr(std::make_shared<ast::expression::ID>(nullptr));
                                                   }
                                                   $$ = newNodePtr(std::make_shared<ast::expression::Lambda>(id_expr->id(), $3, loc(@$)));
                                                 }
              | FIND tuple_expr                  { auto tuple_expr = ast::checkedCast<ast::expression::Constant>($2);
                                                   auto tuple = ast::checkedCast<ast::constant::Tuple>(tuple_expr->constant());
                                                   auto value = tuple->value();
                                                   if (value.size() != 4) {
                                                     error(@$, "find expression requires four arguments");
                                                     $$ = newNodePtr(std::make_shared<ast::expression::Find>(nullptr, nullptr, nullptr, nullptr));
                                                   } else {
                                                     auto value_it = value.begin();
                                                     auto list_expr = *value_it++;
                                                     auto cond_expr = ast::tryCast<ast::expression::Lambda>(*value_it++);
                                                     auto found_expr = ast::tryCast<ast::expression::Lambda>(*value_it++);
                                                     auto not_found_expr = *value_it;
                                                     if (!list_expr || !cond_expr || !found_expr || !not_found_expr){
                                                       error(@$, "incompatible arguments for find expression");
                                                     }
                                                     $$ = newNodePtr(std::make_shared<ast::expression::Find>(list_expr, cond_expr, found_expr, not_found_expr, loc(@$)));
                                                   }
                                                 }

              /* Overloaded operators */

              | expr tuple_expr                  { $$ = makeOp(ast::expression::Operator::Kind::Call, { $1, $2 }, loc(@$)); }
              | expr '[' expr ']'                { $$ = makeOp(ast::expression::Operator::Kind::Index, { $1, $3 }, loc(@$)); }
              | expr AND expr                    { $$ = makeOp(ast::expression::Operator::Kind::LogicalAnd, { $1, $3 }, loc(@$)); }
              | expr OR expr                     { $$ = makeOp(ast::expression::Operator::Kind::LogicalOr, { $1, $3 }, loc(@$)); }
              | expr '&' expr                    { $$ = makeOp(ast::expression::Operator::Kind::BitAnd, { $1, $3 }, loc(@$)); }
              | expr '|' expr                    { $$ = makeOp(ast::expression::Operator::Kind::BitOr, { $1, $3 }, loc(@$)); }
              | expr '^' expr                    { $$ = makeOp(ast::expression::Operator::Kind::BitXor, { $1, $3 }, loc(@$)); }
              | expr SHIFTLEFT expr              { $$ = makeOp(ast::expression::Operator::Kind::ShiftLeft, { $1, $3 }, loc(@$)); }
              | expr SHIFTRIGHT expr             { $$ = makeOp(ast::expression::Operator::Kind::ShiftRight, { $1, $3 }, loc(@$)); }
              | expr POW expr                    { $$ = makeOp(ast::expression::Operator::Kind::Power, { $1, $3 }, loc(@$)); }
              | expr '.' member_expr             { $$ = makeOp(ast::expression::Operator::Kind::Attribute, { $1, $3 }, loc(@$)); }
              | expr '.' member_expr '=' expr    { $$ = makeOp(ast::expression::Operator::Kind::AttributeAssign, { $1, $3, $5 }, loc(@$)); }
              | expr HASATTR member_expr         { $$ = makeOp(ast::expression::Operator::Kind::HasAttribute, { $1, $3 }, loc(@$)); }
              | expr PLUSPLUS                    { $$ = makeOp(ast::expression::Operator::Kind::IncrPostfix, { $1 }, loc(@$)); }
              | PLUSPLUS expr                    { $$ = makeOp(ast::expression::Operator::Kind::IncrPrefix, { $2 }, loc(@$)); }
              | expr MINUSMINUS                  { $$ = makeOp(ast::expression::Operator::Kind::DecrPostfix, { $1 }, loc(@$)); }
              | MINUSMINUS expr                  { $$ = makeOp(ast::expression::Operator::Kind::DecrPrefix, { $2 }, loc(@$)); }
              | '*' expr                         { $$ = makeOp(ast::expression::Operator::Kind::Deref, { $2 }, loc(@$)); }
              | '!' expr                         { $$ = makeOp(ast::expression::Operator::Kind::Not, { $2 }, loc(@$)); }
              | '|' expr '|'                     { $$ = makeOp(ast::expression::Operator::Kind::Size, { $2 }, loc(@$)); }
              | expr '+' expr                    { $$ = makeOp(ast::expression::Operator::Kind::Plus, { $1, $3 }, loc(@$)); }
              | expr '-' expr                    { $$ = makeOp(ast::expression::Operator::Kind::Minus, { $1, $3 }, loc(@$)); }
              | expr '*' expr                    { $$ = makeOp(ast::expression::Operator::Kind::Mult, { $1, $3 }, loc(@$)); }
              | expr '/' expr                    { $$ = makeOp(ast::expression::Operator::Kind::Div, { $1, $3 }, loc(@$)); }
              | expr MOD expr                    { $$ = makeOp(ast::expression::Operator::Kind::Mod, { $1, $3 }, loc(@$)); }
              | expr EQ expr                     { $$ = makeOp(ast::expression::Operator::Kind::Equal, { $1, $3 }, loc(@$)); }
              | expr IN expr                     { $$ = makeOp(ast::expression::Operator::Kind::In, { $1, $3 }, loc(@$)); }
              | expr '<' expr                    { $$ = makeOp(ast::expression::Operator::Kind::Less, { $1, $3 }, loc(@$)); }
              | expr PLUSASSIGN expr             { $$ = makeOp(ast::expression::Operator::Kind::PlusAssign, { $1, $3 }, loc(@$)); }
              | expr MINUSASSIGN expr            { $$ = makeOp(ast::expression::Operator::Kind::MinusAssign, { $1, $3 }, loc(@$)); }
              | expr '[' expr ']' '=' expr       { $$ = makeOp(ast::expression::Operator::Kind::IndexAssign, { $1, $3, $6 }, loc(@$)); }
              | expr '.' member_expr tuple_expr  { $$ = makeOp(ast::expression::Operator::Kind::MethodCall, { $1, $3, $4 }, loc(@$)); }
              | CAST '<' type '>' '(' expr ')'   { $$ = makeOp(ast::expression::Operator::Kind::Cast, { $6, newNodePtr(std::make_shared<ast::expression::Type>($3)) }, loc(@$)); }

              /* Operators derived from other operators. */

              | expr NEQ expr                    { auto e = makeOp(ast::expression::Operator::Kind::Equal, { $1, $3 }, loc(@$));
                                                   $$ = makeOp(ast::expression::Operator::Kind::Not, { e }, loc(@$));
                                                 }

              | expr LEQ expr                    { auto e = makeOp(ast::expression::Operator::Kind::Greater, { $1, $3 }, loc(@$));
                                                   $$ = makeOp(ast::expression::Operator::Kind::Not, { e }, loc(@$));
                                                 }

              | expr GEQ expr                    { auto e = makeOp(ast::expression::Operator::Kind::Less, { $1, $3 }, loc(@$));
                                                   $$ = makeOp(ast::expression::Operator::Kind::Not, { e }, loc(@$));
                                                 }

              | expr '>' expr                    { auto e1 = makeOp(ast::expression::Operator::Kind::Less, { $1, $3 }, loc(@$));
                                                   auto e2 = makeOp(ast::expression::Operator::Kind::Equal, { $1, $3 }, loc(@$));
                                                   auto e3 = makeOp(ast::expression::Operator::Kind::LogicalOr, { e1, e2 }, loc(@$));
                                                   $$ = makeOp(ast::expression::Operator::Kind::Not, { e3 }, loc(@$));
                                                 }

member_expr   : local_id                         { $$ = newNodePtr(std::make_shared<ast::expression::MemberAttribute>($1, loc(@$))); }

tuple_expr    : tuple                            { $$ = newNodePtr(std::make_shared<ast::expression::Constant>($1, loc(@$))); }

exprs         : expr ',' exprs                   { $$ = $3; $$.push_front($1); }
              | expr                             { $$ = ast::expression_list(); $$.push_back($1); }

opt_exprs     : exprs                            { $$ = $1; }
              | /* empty */                      { $$ = ast::expression_list(); }


%% /*** Additional Code ***/


void diffingo::spec::Parser::error(const Parser::location_type& l, const std::string& m)
{
    driver->error(m, l);
}

