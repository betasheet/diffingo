/*
 * scanner.h
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

#ifndef SRC_SPEC_PARSER_SCANNER_H_
#define SRC_SPEC_PARSER_SCANNER_H_

#include <string>

#include "spec/parser/driver.h"
#include "autogen/spec/parser/parser.h"

namespace diffingo {
namespace spec {

class Scanner : public DiffingoFlexLexer {
 public:
  explicit Scanner(std::istream* yyin = nullptr, std::ostream* yyout = nullptr)
      : DiffingoFlexLexer(yyin, yyout) {}
  virtual Parser::token_type lex(Parser::semantic_type* yylval,
                                 Parser::location_type* yylloc,
                                 diffingo::spec::Driver* driver);

  void disablePatternMode();
  void enablePatternMode();
};

}  // namespace spec
}  // namespace diffingo

#endif  // SRC_SPEC_PARSER_SCANNER_H_
