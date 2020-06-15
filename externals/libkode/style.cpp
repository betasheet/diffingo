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

#include "style.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <string>

using namespace KODE;

class Style::Private {
 public:
};

Style::Style() : d(0) {}

Style::Style(const Style & /*other*/) : d(0) {
  //  *d = *other.d;
}

Style::~Style() { delete d; }

Style &Style::operator=(const Style &other) {
  if (this == &other) return *this;

  // *d = *other,d;

  return *this;
}

std::string Style::className(const std::string &str) {
  assert(!str.empty());
  std::string cl = sanitize(upperFirst(str));
  return cl;
}

std::string Style::upperFirst(const std::string &str) {
  if (str.empty()) return str;

  return std::string(1, toupper(str.at(0))) + str.substr(1);
}

std::string Style::sanitize(const std::string &str) {
  if (str == "class") {
    return "class_name";
  }

  if (str == "signals") {
    return "signal_s";
  }

  if (str == "slots") {
    return "slot_s";
  }

  std::string ret = str;
  std::replace(ret.begin(), ret.end(), '-', '_');
  std::replace(ret.begin(), ret.end(), ';', '_');
  std::replace(ret.begin(), ret.end(), ':', '_');
  return ret;
}

std::string Style::lowerFirst(const std::string &str) {
  if (str.empty()) return str;

  return std::string(1, tolower(str.at(0))) + str.substr(1);
}

std::string Style::makeIdentifier(const std::string &str) {
  std::string identifier = str;
  std::replace(identifier.begin(), identifier.end(), '-', '_');
  std::replace(identifier.begin(), identifier.end(), '.', '_');
  std::replace(identifier.begin(), identifier.end(), '/', '_');
  std::replace(identifier.begin(), identifier.end(), ':', '_');
  std::replace(identifier.begin(), identifier.end(), ' ', '_');

  // Can't start with a number, either.
  const int firstNum = identifier.at(0) - '0';
  identifier = (firstNum >= 0 && firstNum <= 9)
                   ? std::string(1, '_') + identifier
                   : identifier;

  return identifier;
}
