/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "code.h"

#include <boost/algorithm/string.hpp>
#include <list>
#include <string>

using namespace KODE;

static int s_defaultIndentation = 2;

class Code::Private {
 public:
  Private() : mIndent(0) {}

  std::string mText;
  int mIndent;
};

Code::Code() : d(new Private) {}

Code::Code(const Code &other) : d(new Private) { *d = *other.d; }

Code::Code(int indent) : d(new Private) { d->mIndent = indent; }

Code::~Code() { delete d; }

Code &Code::operator=(const Code &other) {
  if (this == &other) return *this;

  *d = *other.d;

  return *this;
}

void Code::clear() {
  d->mIndent = 0;
  d->mText.clear();
}

bool Code::isEmpty() const { return d->mText.empty(); }

void Code::setIndent(int indent) { d->mIndent = indent; }

void Code::indent() { d->mIndent += s_defaultIndentation; }

void Code::unindent() {
  d->mIndent -= s_defaultIndentation;
  if (d->mIndent < 0) d->mIndent = 0;
}

std::string Code::text() const { return d->mText; }

void Code::addLine(const std::string &line) {
  d->mText += spaces(d->mIndent);
  d->mText += line;
  d->mText += '\n';
}

void Code::addLine(const char c) {
  d->mText += spaces(d->mIndent);
  d->mText += c;
  d->mText += '\n';
}

void Code::newLine() { d->mText += '\n'; }

std::string Code::spaces(int count) {
  std::string str;
  for (int i = 0; i < count; ++i) str += ' ';

  return str;
}

void Code::addBlock(const std::string &block) {
  std::list<std::string> lines;
  boost::algorithm::split(lines, block, boost::algorithm::is_any_of("\n"));
  if (!lines.empty() && lines.back().empty()) {
    lines.pop_back();
  }

  for (const auto &l : lines) {
    if (!l.empty()) d->mText += spaces(d->mIndent);

    d->mText += l;
    d->mText += '\n';
  }
}

void Code::addBlock(const std::string &block, int indent) {
  int tmp = d->mIndent;
  d->mIndent = indent;
  addBlock(block);
  d->mIndent = tmp;
}

void Code::addBlock(const Code &c) { addBlock(c.text()); }

void Code::addWrappedText(const std::string &txt) {
  int maxWidth = 80 - d->mIndent;
  int pos = 0;
  while (pos < txt.length()) {
    std::string line = txt.substr(pos, maxWidth);
    addLine(line);
    pos += maxWidth;
  }
}

void Code::addFormattedText(const std::string &text) {
  int maxWidth = 80 - d->mIndent;
  int lineLength = 0;

  std::string line;
  std::list<std::string> words;
  boost::algorithm::split(words, text, boost::algorithm::is_any_of(" \n\t"),
                          boost::algorithm::token_compress_on);

  for (const auto &w : words) {
    if (w.length() + lineLength >= maxWidth) {
      boost::algorithm::trim(line);
      addLine(line);
      line.clear();
      lineLength = 0;
    }

    line += w + ' ';
    lineLength += w.length() + 1;
  }

  boost::algorithm::trim(line);
  addLine(line);
}

Code &Code::operator+=(const std::string &str) {
  addLine(str);
  return *this;
}

// Code &Code::operator+=(const QByteArray &str) {
//  addLine(std::string::fromLocal8Bit(str.data(), str.size()));
//  return *this;
// }

Code &Code::operator+=(const char *str) {
  addLine(str);
  return *this;
}

Code &Code::operator+=(const char c) {
  addLine(c);
  return *this;
}

Code &Code::operator+=(const Code &code) {
  d->mText += code.d->mText;
  return *this;
}

void Code::setDefaultIndentation(int indent) { s_defaultIndentation = indent; }

int Code::defaultIndentation() { return s_defaultIndentation; }
