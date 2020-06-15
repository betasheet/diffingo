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

#include "membervariable.h"

#include <cctype>
#include <string>

#include "style.h"

using namespace KODE;

class MemberVariable::MVPrivate {
 public:
  int mAccess;
};

MemberVariable::MemberVariable() : Variable(), d(0) {}

MemberVariable::MemberVariable(const MemberVariable &other)
    : Variable(other), d(new MVPrivate) {
  *d = *other.d;
}

MemberVariable::MemberVariable(const std::string &name, const std::string &type,
                               bool isStatic, int access)
    : Variable(name, type, isStatic), d(new MVPrivate) {
  d->mAccess = access;
  setName(memberVariableName(name));
}

MemberVariable::~MemberVariable() { delete d; }

MemberVariable &MemberVariable::operator=(const MemberVariable &other) {
  if (this == &other) return *this;

  Variable::operator=(other);
  *d = *other.d;

  return *this;
}

void MemberVariable::setAccess(int access) { d->mAccess = access; }

int MemberVariable::access() const { return d->mAccess; }

std::string MemberVariable::memberVariableName(const std::string &name) {
  //  std::string n;
  //
  //  if (name.empty()) {
  //    n = "mUndefined";
  //  } else if (name.length() >= 2 && name[0] == 'm' &&
  //             (toupper(name[1]) == name[1])) {
  //    n = name;
  //  } else if (name == "q" || name == "d") {
  //    n = name;
  //  } else {
  //    n = "m";
  //    n += toupper(name[0]);
  //    n += name.substr(1);
  //  }

  return Style::makeIdentifier(name);
}
