/*
    This file is part of KDE.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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

#include "enum.h"

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <sys/types.h>
#include <list>
#include <string>

#include "code.h"
#include "style.h"

using namespace KODE;

class Enum::Private {
 public:
  Private() : mCombinable(false) {}

  std::string mName;
  std::list<label_with_value> mEnums;
  bool mCombinable;
  bool mUseEnumCls;
};

Enum::Enum() : d(new Private) {}

Enum::Enum(const Enum &other) : d(new Private) { *d = *other.d; }

Enum::Enum(const std::string &name, const std::list<std::string> &enums,
           bool combinable, bool use_enum_cls)
    : d(new Private) {
  d->mName = name;
  d->mCombinable = combinable;
  d->mUseEnumCls = use_enum_cls;
  for (const auto &e : enums) {
    label_with_value l(e, false, 0);
    d->mEnums.push_back(l);
  }
}

Enum::Enum(const std::string &name, const std::list<label_with_value> &enums,
           bool use_enum_cls)
    : d(new Private) {
  d->mName = name;
  d->mEnums = enums;
  d->mUseEnumCls = use_enum_cls;
}

Enum::~Enum() { delete d; }

Enum &Enum::operator=(const Enum &other) {
  if (this == &other) return *this;

  *d = *other.d;

  return *this;
}

std::string Enum::name() const { return d->mName; }

/**
 * @brief Enum::declaration
 * @return Returns a std::string contatining the enum declaration in the
 *following format:
 * enum Foo { Foo_a, Foo_b, Foo_Invalid };
 *
 * If the enum name is suffixed with Enum (FooEnum) then the Enum suffix will be
 *removed
 * from the enum item's names:
 * enum FooEnum { Foo_a, Foo_b, Foo_Invalid };
 *
 * The last item (Foo_Invalid) is automatically generated to each elements, and
 *the
 * enum parsing methods will return this value if the XML contains a non-enum
 *value.
 */
std::string Enum::declaration() const {
  std::string retval("enum ");
  if (d->mUseEnumCls) retval += "class ";
  retval += d->mName + " {";
  int value = 0;
  std::string baseName = name();
  if (boost::algorithm::ends_with(d->mName, "Enum") && d->mName.length() > 4) {
    baseName = d->mName.substr(0, d->mName.length() - 4);
  }

  for (const auto &e : d->mEnums) {
    if (d->mCombinable) {
      if (value == 0)
        retval += (boost::format(" %1% = %2%") %
                   (Style::sanitize(std::get<0>(e))) % (1 << value)).str();
      else
        retval += (boost::format(", %1% = %2%") %
                   (Style::sanitize(std::get<0>(e))) % (1 << value)).str();
    } else {
      if (value == 0) {
        if (std::get<1>(e))
          retval +=
              (boost::format(" %1% = %2%") % (Style::sanitize(std::get<0>(e))) %
               (std::get<2>(e))).str();
        else
          retval += ' ' + Style::sanitize(std::get<0>(e));
      } else {
        if (std::get<1>(e))
          retval +=
              (boost::format(", %1% = %2%") %
               (Style::sanitize(std::get<0>(e))) % (std::get<2>(e))).str();
        else
          retval += ", " + Style::sanitize(std::get<0>(e));
      }
    }
    ++value;
  }

  retval += " };";

  return retval;
}

KODE::Function Enum::parserMethod() const {
  std::string baseName = name();
  if (boost::algorithm::ends_with(d->mName, "Enum") && d->mName.length() > 4) {
    baseName = d->mName.substr(0, d->mName.length() - 4);
  }

  KODE::Function ret(KODE::Style::lowerFirst(this->name()) + "FromString",
                     this->name());
  ret.setStatic(true);

  ret.addArgument("const std::string & v");
  ret.addArgument("bool *ok", "NULL");

  KODE::Code code;
  code += "if (ok) *ok = true;";
  code.newLine();
  bool first = true;
  for (const auto &enumItem : d->mEnums) {
    if (first) {
      code += "if ( v == \"" + std::get<0>(enumItem) + "\" ) {";
      first = false;
    } else {
      code += "} else if ( v == \"" + std::get<0>(enumItem) + "\" ) {";
    }
    code.indent();
    code += "return " + baseName + '_' +
            Style::sanitize(std::get<0>(enumItem)) + ";";
    code.unindent();
  }
  code += "} else {";
  code.indent();
  code += "if (ok) *ok = false;";
  code += "return " + baseName + "_Invalid;";
  code.unindent();
  code += "}";
  code.newLine();
  code += "return " + baseName + "_Invalid;";

  ret.setBody(code);
  return ret;
}

KODE::Function Enum::writerMethod() const {
  std::string baseName = name();
  if (boost::algorithm::ends_with(d->mName, "Enum") && d->mName.length() > 4) {
    baseName = d->mName.substr(0, d->mName.length() - 4);
  }

  KODE::Function ret(KODE::Style::lowerFirst(this->name()) + "ToString",
                     "std::string");
  ret.setStatic(true);

  ret.addArgument((boost::format("const %1% & v") % (this->name())).str());

  KODE::Code code;
  code += "switch( v ) {";
  code.indent();
  for (const auto &enumItem : d->mEnums) {
    code += (boost::format("case %1%: return \"%2%\";") %
             (baseName + '_' + Style::sanitize(std::get<0>(enumItem))) %
             (std::get<0>(enumItem))).str();
  }
  code += "case " + baseName + "_Invalid:";
  code += "default:";
  code.indent();
  code += (boost::format(
               "std::cerr << \"Unable to serialize a(n) %1 enum because it "
               "has invalid value\" << %2;") %
           this->name() % "v").str();
  code += "return std::string();";
  code.unindent();
  code.unindent();
  code += "}";
  ret.setBody(code);
  return ret;
}
