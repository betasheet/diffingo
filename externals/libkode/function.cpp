/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2009 David Faure <dfaure@kdab.com>

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

#include "function.h"

#include <boost/algorithm/string.hpp>
#include <cassert>
#include <list>
#include <string>

#include "code.h"

using namespace KODE;

class Function::Argument::ArgumentPrivate {
 public:
  std::string declaration;
  std::string defaultArgument;
};

Function::Argument::Argument(const std::string &declaration,
                             const std::string &defaultArgument)
    : d(new ArgumentPrivate) {
  d->declaration = declaration;
  d->defaultArgument = defaultArgument;
}

Function::Argument::Argument(const Function::Argument &other)
    : d(new ArgumentPrivate) {
  *d = *other.d;
}

std::string Function::Argument::headerDeclaration() const {
  if (d->defaultArgument.empty()) {
    return d->declaration;
  } else {
    return d->declaration + " = " + d->defaultArgument;
  }
}

std::string Function::Argument::bodyDeclaration() const {
  return d->declaration;
}

Function::Argument &Function::Argument::operator=(
    const Function::Argument &other) {
  if (this == &other) return *this;

  *d = *other.d;

  return *this;
}

Function::Argument::~Argument() { delete d; }

class Function::FunctionPrivate {
 public:
  FunctionPrivate()
      : mAccess(Public),
        mIsConst(false),
        mIsStatic(false),
        mVirtualMode(NotVirtual) {}

  int mAccess;
  bool mIsConst;
  bool mIsStatic;
  std::string mReturnType;
  std::string mName;
  Argument::List mArguments;
  std::list<std::string> mInitializers;
  std::string mBody;
  std::string mDocs;
  Function::VirtualMode mVirtualMode;
};

Function::Function() : d(new FunctionPrivate) {}

Function::Function(const Function &other) : d(new FunctionPrivate) {
  *d = *other.d;
}

Function::Function(const std::string &name, const std::string &returnType,
                   int access, bool isStatic)
    : d(new FunctionPrivate) {
  d->mAccess = access;
  d->mIsStatic = isStatic;
  d->mName = name;
  setReturnType(returnType);
}

Function::~Function() { delete d; }

Function &Function::operator=(const Function &other) {
  if (this == &other) return *this;

  *d = *other.d;

  return *this;
}

void Function::setConst(bool isConst) { d->mIsConst = isConst; }

bool Function::isConst() const { return d->mIsConst; }

void Function::setStatic(bool isStatic) { d->mIsStatic = isStatic; }

bool Function::isStatic() const { return d->mIsStatic; }

void Function::addArgument(const Function::Argument &argument) {
  d->mArguments.push_back(argument);
}

void Function::addArgument(const std::string &argument,
                           const std::string &defaultArgument) {
  d->mArguments.push_back(Argument(argument, defaultArgument));
}

void Function::setArgumentString(const std::string &argumentString) {
  d->mArguments.clear();

  std::list<std::string> arguments;
  boost::algorithm::split(arguments, argumentString,
                          boost::algorithm::is_any_of(","));
  for (const auto &arg : arguments) {
    addArgument(arg);
  }
}

Function::Argument::List Function::arguments() const { return d->mArguments; }

void Function::addInitializer(const std::string &initializer) {
  d->mInitializers.push_back(initializer);
}

std::list<std::string> Function::initializers() const {
  return d->mInitializers;
}

void Function::setBody(const std::string &body) { d->mBody = body; }

void Function::setBody(const Code &body) { d->mBody = body.text(); }

void Function::addBodyLine(const std::string &bodyLine) {
  d->mBody.append(bodyLine);
  if (bodyLine.back() != '\n') d->mBody.append("\n");
}

std::string Function::body() const { return d->mBody; }

void Function::setAccess(int access) { d->mAccess = access; }

int Function::access() const { return d->mAccess; }

std::string Function::accessAsString() const {
  std::string access;

  if (d->mAccess & Public) access = "public";
  if (d->mAccess & Protected) access = "protected";
  if (d->mAccess & Private) access = "private";

  if (d->mAccess & Signal) access = "signals";
  if (d->mAccess & Slot) access += " slots";

  return access;
}

void Function::setReturnType(const std::string &returnType) {
  assert(returnType != "*");
  d->mReturnType = returnType;
}

std::string Function::returnType() const { return d->mReturnType; }

void Function::setName(const std::string &name) { d->mName = name; }

std::string Function::name() const { return d->mName; }

void Function::setDocs(const std::string &docs) { d->mDocs = docs; }

std::string Function::docs() const { return d->mDocs; }

bool Function::hasArguments() const { return !d->mArguments.empty(); }

void Function::setVirtualMode(Function::VirtualMode v) { d->mVirtualMode = v; }

Function::VirtualMode Function::virtualMode() const { return d->mVirtualMode; }
