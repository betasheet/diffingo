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

#include "file.h"

#include <boost/algorithm/string.hpp>
#include <list>
#include <string>
#include <cassert>

#include "class.h"
#include "code.h"
#include "enum.h"
#include "function.h"
#include "license.h"
#include "variable.h"

using namespace KODE;

class File::Private {
 public:
  Private() : mProject() {}

  std::string mHeaderFilename;
  std::string mImplFilename;
  std::string mNameSpace;
  std::string mProject;
  std::list<std::string> mCopyrightStrings;
  License mLicense;
  std::list<std::string> mIncludes;
  Class::List mClasses;
  Variable::List mFileVariables;
  Function::List mFileFunctions;
  Enum::List mFileEnums;
  std::list<std::string> mExternCDeclarations;
  Code mFileCode;
  Code mHeaderCode;
};

File::File() : d(new Private) {}

File::File(const File &other) : d(new Private) { *d = *other.d; }

File::~File() { delete d; }

File &File::operator=(const File &other) {
  if (this == &other) return *this;

  *d = *other.d;

  return *this;
}

void File::setFilename(const std::string &filename) {
  d->mImplFilename = filename + ".cpp";
  d->mHeaderFilename = filename + ".h";
}

void File::setImplementationFilename(const std::string &filename) {
  d->mImplFilename = filename;
}

void File::setHeaderFilename(const std::string &filename) {
  d->mHeaderFilename = filename;
}

std::string File::filenameHeader() const {
  if (!d->mHeaderFilename.empty()) return d->mHeaderFilename;

  if (!d->mClasses.empty()) {
    std::string className = d->mClasses.front().name();
    boost::algorithm::to_lower(className);
    return className + ".h";
  }

  return std::string();
}

std::string File::filenameImplementation() const {
  if (!d->mImplFilename.empty()) return d->mImplFilename;

  if (!d->mClasses.empty()) {
    std::string className = d->mClasses.front().name();
    boost::algorithm::to_lower(className);
    return className + ".cpp";
  }

  return std::string();
}

void File::setNameSpace(const std::string &nameSpace) {
  d->mNameSpace = nameSpace;
}

std::string File::nameSpace() const { return d->mNameSpace; }

void File::setProject(const std::string &project) {
  if (project.empty()) return;

  d->mProject = project;
}

std::string File::project() const { return d->mProject; }

void File::addCopyright(int year, const std::string &name,
                        const std::string &email) {
  std::string str =
      "Copyright (c) " + std::to_string(year) + ' ' + name + " <" + email + '>';

  d->mCopyrightStrings.push_back(str);
}

std::list<std::string> File::copyrightStrings() const {
  return d->mCopyrightStrings;
}

void File::setLicense(const License &license) { d->mLicense = license; }

License File::license() const { return d->mLicense; }

void File::addInclude(const std::string &_include) {
  std::string include = _include;
  if (!boost::algorithm::ends_with(include, ".h")) include.append(".h");

  if (std::find(d->mIncludes.begin(), d->mIncludes.end(), include) ==
      d->mIncludes.end())
    d->mIncludes.push_back(include);
}

std::list<std::string> File::includes() const { return d->mIncludes; }

Class *File::insertClass(const Class &newClass) {
  assert(!newClass.name().empty());
  Class::List::iterator it;
  for (it = d->mClasses.begin(); it != d->mClasses.end(); ++it) {
    if ((*it).qualifiedName() == newClass.qualifiedName()) {
      it = d->mClasses.erase(it);
      it = d->mClasses.insert(it, newClass);
      return &(*it);
    }
  }

  d->mClasses.push_back(newClass);
  return &d->mClasses.back();
}

Class::List File::classes() const { return d->mClasses; }

bool File::hasClass(const std::string &name) {
  for (const auto &c : d->mClasses) {
    if (c.name() == name) return true;
  }

  return false;
}

Class *File::findClass(const std::string &name) {
  for (auto &c : d->mClasses) {
    if (c.name() == name) return &c;
  }

  return nullptr;
}

Variable *File::addFileVariable(const Variable &variable) {
  d->mFileVariables.push_back(variable);
  return &d->mFileVariables.back();
}

Variable::List File::fileVariables() const { return d->mFileVariables; }

Function *File::addFileFunction(const Function &function) {
  d->mFileFunctions.push_back(function);
  return &d->mFileFunctions.back();
}

Function::List File::fileFunctions() const { return d->mFileFunctions; }

Enum *File::addFileEnum(const Enum &enumValue) {
  d->mFileEnums.push_back(enumValue);
  return &d->mFileEnums.back();
}

Enum::List File::fileEnums() const { return d->mFileEnums; }

void File::addExternCDeclaration(const std::string &externalCDeclaration) {
  d->mExternCDeclarations.push_back(externalCDeclaration);
}

std::list<std::string> File::externCDeclarations() const {
  return d->mExternCDeclarations;
}

Code *File::setFileCode(const Code &code) {
  d->mFileCode = code;
  return &d->mFileCode;
}

Code *File::fileCode() const { return &d->mFileCode; }

Code *File::setHeaderCode(const Code &code) {
  d->mHeaderCode = code;
  return &d->mHeaderCode;
}

Code *File::headerCode() const { return &d->mHeaderCode; }

void File::clearClasses() { d->mClasses.clear(); }

void File::clearFileFunctions() { d->mFileFunctions.clear(); }

void File::clearFileVariables() { d->mFileVariables.clear(); }

void File::clearCode() {
  clearClasses();
  clearFileFunctions();
  clearFileVariables();
}
