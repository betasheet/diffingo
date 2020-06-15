/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2010 David Faure <dfaure@kdab.com>

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

#include "class.h"

#include <boost/algorithm/string.hpp>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <list>
#include <string>

#include "enum.h"
#include "function.h"
#include "membervariable.h"
#include "typedef.h"

using namespace KODE;

class Class::Private {
 public:
  Private() : mDPointer(), mUseSharedData(false), mCanBeCopied(false) {}

  std::string mName;
  std::string mNameSpace;
  std::string mExportDeclaration;
  std::string mDPointer;
  bool mUseSharedData;
  bool mCanBeCopied;
  Function::List mFunctions;
  MemberVariable::List mMemberVariables;
  std::list<std::string> mIncludes;
  std::list<std::string> mForwardDeclarations;
  std::list<std::string> mHeaderIncludes;
  Class::List mBaseClasses;
  Typedef::List mTypedefs;
  Enum::List mEnums;
  std::string mDocs;
  Class::List mNestedClasses;
  std::string mParentClassName;
  std::list<std::string> mDeclMacros;
};

Class::Class() : d(new Private) {}

Class::Class(const Class &other) : d(new Private) { *d = *other.d; }

Class::Class(const std::string &name, const std::string &nameSpace)
    : d(new Private) {
  assert(!name.empty());
  d->mName = name;
  d->mNameSpace = nameSpace;
}

Class::~Class() { delete d; }

Class &Class::operator=(const Class &other) {
  if (this == &other) return *this;

  *d = *other.d;

  return *this;
}

void Class::setName(const std::string &name) {
  assert(!name.empty());
  d->mName = name;
}

std::string Class::name() const { return d->mName; }

void Class::setNameSpace(const std::string &nameSpace) {
  d->mNameSpace = nameSpace;
}

std::string Class::nameSpace() const { return d->mNameSpace; }

std::string Class::qualifiedName() const {
  if (d->mNameSpace.empty()) return d->mName;
  return d->mNameSpace + "::" + d->mName;
}

void Class::setUseDPointer(bool useDPointer, const std::string &dPointer) {
  d->mDPointer = useDPointer ? dPointer : std::string();
}

bool Class::useDPointer() const { return !d->mDPointer.empty(); }

void Class::setUseSharedData(bool b, const std::string &dPointer) {
  d->mUseSharedData = b;
  if (b) {
    setUseDPointer(true, dPointer);
    d->mCanBeCopied = true;
  }
}

bool Class::useSharedData() const { return d->mUseSharedData; }

void Class::setCanBeCopied(bool b) { d->mCanBeCopied = b; }

bool Class::canBeCopied() const { return d->mCanBeCopied; }

void Class::addInclude(const std::string &include,
                       const std::string &forwardDeclaration) {
  if (!include.empty() &&
      std::find(d->mIncludes.begin(), d->mIncludes.end(), include) ==
          d->mIncludes.end())
    d->mIncludes.push_back(include);

  if (!forwardDeclaration.empty() &&
      std::find(d->mForwardDeclarations.begin(), d->mForwardDeclarations.end(),
                forwardDeclaration) == d->mForwardDeclarations.end())
    d->mForwardDeclarations.push_back(forwardDeclaration);
}

void Class::addIncludes(const std::list<std::string> &files,
                        const std::list<std::string> &forwardDeclarations) {
  for (const auto &f : files) {
    if (std::find(d->mIncludes.begin(), d->mIncludes.end(), f) ==
        d->mIncludes.end())
      if (!f.empty()) d->mIncludes.push_back(f);
  }

  for (const auto &f : forwardDeclarations) {
    if (std::find(d->mForwardDeclarations.begin(),
                  d->mForwardDeclarations.end(),
                  f) == d->mForwardDeclarations.end())
      d->mForwardDeclarations.push_back(f);
  }
}

std::list<std::string> Class::includes() const { return d->mIncludes; }

std::list<std::string> Class::forwardDeclarations() const {
  return d->mForwardDeclarations;
}

void Class::addHeaderInclude(const std::string &include) {
  if (include.empty()) return;

  if (std::find(d->mHeaderIncludes.begin(), d->mHeaderIncludes.end(),
                include) == d->mHeaderIncludes.end())
    d->mHeaderIncludes.push_back(include);
}

void Class::addHeaderIncludes(const std::list<std::string> &includes) {
  for (const auto &i : includes) addHeaderInclude(i);
}

std::list<std::string> Class::headerIncludes() const {
  return d->mHeaderIncludes;
}

void Class::addBaseClass(const Class &c) { d->mBaseClasses.push_back(c); }

Class::List Class::baseClasses() const { return d->mBaseClasses; }

void Class::addFunction(const Function &function) {
  d->mFunctions.push_back(function);
}

Function::List Class::functions() const { return d->mFunctions; }

void Class::addMemberVariable(const MemberVariable &v) {
  d->mMemberVariables.push_back(v);
}

MemberVariable::List Class::memberVariables() const {
  return d->mMemberVariables;
}

void Class::addTypedef(const Typedef &typeDefinition) {
  d->mTypedefs.push_back(typeDefinition);
}

Typedef::List Class::typedefs() const { return d->mTypedefs; }

void Class::addEnum(const Enum &enumValue) {
  d->mEnums.push_back(enumValue);
  KODE::Function enumParseFunction = enumValue.parserMethod();
  enumParseFunction.setReturnType(d->mName + "::" +
                                  enumParseFunction.returnType());
  enumParseFunction.setAccess(KODE::Function::Private);
  addFunction(enumParseFunction);

  KODE::Function enumWriteFunction = enumValue.writerMethod();
  enumParseFunction.setReturnType(d->mName + "::" +
                                  enumWriteFunction.returnType());
  enumParseFunction.setAccess(KODE::Function::Private);
  addFunction(enumWriteFunction);
}

Enum::List Class::enums() const { return d->mEnums; }

bool Class::hasEnum(const std::string &name) const {
  for (const auto &e : d->mEnums) {
    if (e.name() == name) return true;
  }
  return false;
}

bool Class::isValid() const { return !d->mName.empty(); }

bool Class::hasFunction(const std::string &functionName) const {
  for (const auto &f : d->mFunctions) {
    if (f.name() == functionName) return true;
  }

  return false;
}

bool Class::isQObject() const {
  for (const auto &f : d->mFunctions) {
    if ((f.access() & Function::Signal) || (f.access() & Function::Slot))
      return true;
  }

  return false;
}

void Class::setDocs(const std::string &str) { d->mDocs = str; }

std::string Class::docs() const { return d->mDocs; }

void Class::addNestedClass(const Class &nestedClass) {
  Class addedClass = nestedClass;
  addedClass.setParentClassName(name());

  d->mNestedClasses.push_back(addedClass);
}

Class::List Class::nestedClasses() const { return d->mNestedClasses; }

std::string Class::parentClassName() const { return d->mParentClassName; }

void Class::setParentClassName(const std::string &parentClassName) {
  d->mParentClassName = parentClassName;
}

std::string Class::dPointerName() const { return d->mDPointer; }

////

// Returns what a class depends on: its base class(es) and any by-value member
// var
static std::list<std::string> dependenciesForClass(
    const Class &aClass, const std::list<std::string> &allClasses,
    const std::list<std::string> &excludedClasses) {
  std::list<std::string> lst;
  for (const auto &baseClass : aClass.baseClasses()) {
    const std::string baseName = baseClass.name();
    if (!baseName.front() == 'Q' &&
        std::find(excludedClasses.begin(), excludedClasses.end(), baseName) ==
            excludedClasses.end())
      lst.push_back(baseClass.name());
  }
  if (!aClass.useDPointer()) {
    for (const auto &member : aClass.memberVariables()) {
      const std::string type = member.type();
      if (std::find(allClasses.begin(), allClasses.end(), type) !=
          allClasses.end()) {
        lst.push_back(type);
      }
    }
  }

  return lst;
}

static bool allKnown(const std::list<std::string> &deps,
                     const std::list<std::string> &classNames) {
  for (const auto & dep : deps) {
    if (std::find(classNames.begin(), classNames.end(), dep) ==
        classNames.end()) {
      return false;
    }
  }
  return true;
}

/**
 * This method sorts a list of classes in a way that the base class
 * of a class, as well as the classes it use by value in member vars,
 * always appear before the class itself.
 */
static Class::List sortByDependenciesHelper(
    const Class::List &classes, const std::list<std::string> &excludedClasses) {
  Class::List allClasses(classes);
  std::list<std::string> allClassNames;
  for (const auto & c : classes) allClassNames.push_back(c.name());

  Class::List retval;

  std::list<std::string> classNames;

  // copy all classes without dependencies
  Class::List::iterator it;
  for (it = allClasses.begin(); it != allClasses.end(); ++it) {
    if (dependenciesForClass(*it, allClassNames, excludedClasses).empty()) {
      retval.push_back(*it);
      classNames.push_back((*it).name());

      it = allClasses.erase(it);
      it--;
    }
  }

  while (allClasses.size() > 0) {
    const int currentCount = allClasses.size();
    // copy all classes which have a class from retval/classNames (i.e. already
    // written out)
    // as base class - or as member variable
    for (it = allClasses.begin(); it != allClasses.end(); ++it) {
      const std::list<std::string> deps =
          dependenciesForClass(*it, allClassNames, excludedClasses);
      if (allKnown(deps, classNames)) {
        retval.push_back(*it);
        classNames.push_back((*it).name());

        it = allClasses.erase(it);
        it--;
      }
    }
    if (allClasses.size() == currentCount) {
      // We didn't resolve anything this time around, so let's not loop forever
      std::cerr << "ERROR: Couldn't find class dependencies (base classes, "
                   "member vars) for classes "
                << boost::algorithm::join(allClasses.classNames(), ", ");
      for (const auto & c : allClasses) {
        std::cerr << c.name() << "depends on"
                  << boost::algorithm::join(
                         dependenciesForClass(c, allClassNames,
                                              excludedClasses),
                         ", ");
      }

      return retval;
    }
  }

  return retval;
}

void ClassList::sortByDependencies(
    const std::list<std::string> &excludedClasses) {
  *this = sortByDependenciesHelper(*this, excludedClasses);
}

ClassList::iterator ClassList::findClass(const std::string &name) {
  ClassList::iterator it = begin();
  for (; it != end(); ++it)
    if ((*it).name() == name) break;
  return it;
}

std::list<std::string> KODE::ClassList::classNames() const {
  std::list<std::string> names;
  ClassList::const_iterator it = begin();
  for (; it != end(); ++it) names.push_back((*it).name());
  return names;
}

void KODE::Class::addDeclarationMacro(const std::string &macro) {
  d->mDeclMacros.push_back(macro);
}

std::list<std::string> KODE::Class::declarationMacros() const {
  return d->mDeclMacros;
}

void KODE::Class::setNamespaceAndName(const std::string &name) {
  d->mName = name;
  d->mNameSpace.clear();
  while (boost::algorithm::contains(d->mName, "::")) {
    const int pos = d->mName.find("::");
    if (!d->mNameSpace.empty()) d->mNameSpace += "::";
    d->mNameSpace += d->mName.substr(0, pos);
    d->mName = d->mName.substr(pos + 2);
  }
}

void KODE::Class::setExportDeclaration(const std::string &name) {
  d->mExportDeclaration = name;
}

std::string KODE::Class::exportDeclaration() const {
  return d->mExportDeclaration;
}
