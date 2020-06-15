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

#include "variable.h"

#include <string>
#include <cassert>

#include "membervariable.h"

using namespace KODE;

class Variable::Private {
 public:
  Private() : mIsStatic(false) {}

  std::string mType;
  std::string mName;
  bool mIsStatic;
  std::string mInitializer;
};

Variable::Variable() : d(new Private) {}

Variable::Variable(const Variable &other) : d(new Private) { *d = *other.d; }

Variable::Variable(const std::string &name, const std::string &type,
                   bool isStatic)
    : d(new Private) {
  d->mType = type;
  d->mIsStatic = isStatic;

  assert(!name.empty());

  if (name.empty()) {
    d->mName = "mUndefined";
  } else {
    d->mName = name;
  }
}

Variable::~Variable() { delete d; }

Variable &Variable::operator=(const Variable &other) {
  if (this == &other) return *this;

  *d = *other.d;

  return *this;
}

void Variable::setName(const std::string &name) { d->mName = name; }

std::string Variable::name() const { return d->mName; }

void Variable::setType(const std::string &type) { d->mType = type; }

std::string Variable::type() const { return d->mType; }

void Variable::setStatic(bool isStatic) { d->mIsStatic = isStatic; }

bool Variable::isStatic() const { return d->mIsStatic; }

void Variable::setInitializer(const std::string &initializer) {
  d->mInitializer = initializer;
}

std::string Variable::initializer() const { return d->mInitializer; }
