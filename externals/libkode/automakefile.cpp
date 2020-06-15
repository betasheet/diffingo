/*
    This file is part of KDE.

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

#include "automakefile.h"

#include <algorithm>
#include <list>
#include <map>
#include <string>

using namespace KODE;

class AutoMakefile::Target::Private {
 public:
  std::string mType;
  std::string mName;

  std::string mSources;
  std::string mLibAdd;
  std::string mLdAdd;
  std::string mLdFlags;
};

AutoMakefile::Target::Target() : d(new Private) {}

AutoMakefile::Target::Target(const AutoMakefile::Target &other)
    : d(new Private) {
  *d = *other.d;
}

AutoMakefile::Target::Target(const std::string &type, const std::string &name)
    : d(new Private) {
  d->mType = type;
  d->mName = name;
}

AutoMakefile::Target::~Target() { delete d; }

AutoMakefile::Target &AutoMakefile::Target::operator=(
    const AutoMakefile::Target &other) {
  if (this == &other) return *this;

  *d = *other.d;

  return *this;
}

void AutoMakefile::Target::setType(const std::string &type) { d->mType = type; }

std::string AutoMakefile::Target::type() const { return d->mType; }

void AutoMakefile::Target::setName(const std::string &name) { d->mName = name; }

std::string AutoMakefile::Target::name() const { return d->mName; }

void AutoMakefile::Target::setSources(const std::string &sources) {
  d->mSources = sources;
}

std::string AutoMakefile::Target::sources() const { return d->mSources; }

void AutoMakefile::Target::setLibAdd(const std::string &libAdd) {
  d->mLibAdd = libAdd;
}

std::string AutoMakefile::Target::libAdd() const { return d->mLibAdd; }

void AutoMakefile::Target::setLdAdd(const std::string &ldAdd) {
  d->mLdAdd = ldAdd;
}

std::string AutoMakefile::Target::ldAdd() const { return d->mLdAdd; }

void AutoMakefile::Target::setLdFlags(const std::string &ldFlags) {
  d->mLdFlags = ldFlags;
}

std::string AutoMakefile::Target::ldFlags() const { return d->mLdFlags; }

class AutoMakefile::Private {
 public:
  Target::List mTargets;
  std::list<std::string> mTargetTypes;

  std::list<std::string> mEntries;
  std::map<std::string, std::string> mValues;
};

AutoMakefile::AutoMakefile() : d(new Private) {}

AutoMakefile::AutoMakefile(const AutoMakefile &other) : d(new Private) {
  *d = *other.d;
}

AutoMakefile::~AutoMakefile() { delete d; }

AutoMakefile &AutoMakefile::operator=(const AutoMakefile &other) {
  if (this == &other) return *this;

  *d = *other.d;

  return *this;
}

void AutoMakefile::addTarget(const Target &target) {
  d->mTargets.push_back(target);

  if (std::find(d->mTargetTypes.begin(), d->mTargetTypes.end(),
                target.type()) == d->mTargetTypes.end())
    d->mTargetTypes.push_back(target.type());
}

AutoMakefile::Target::List AutoMakefile::targets() const { return d->mTargets; }

void AutoMakefile::addEntry(const std::string &variable,
                            const std::string &value) {
  if (variable.empty()) {
    d->mEntries.push_back(variable);
    return;
  }

  if (std::find(d->mEntries.begin(), d->mEntries.end(), variable) ==
      d->mEntries.end()) {
    d->mEntries.push_back(variable);
    auto it = d->mValues.find(variable);
    if (it == d->mValues.end()) {
      d->mValues[variable] = value;
    } else {
      d->mValues[variable].append(' ' + value);
    }
  }
}

void AutoMakefile::newLine() { addEntry(""); }

std::string AutoMakefile::text() const {
  std::string out;

  for (const auto &variable : d->mEntries) {
    if (variable.empty()) {
      out += '\n';
    } else {
      out += variable + " = " + d->mValues[variable] + '\n';
    }
  }
  out += '\n';

  for (const auto &targetType : d->mTargetTypes) {
    out += targetType + " = ";

    for (const auto &target : d->mTargets) {
      if (target.type() != targetType) continue;

      out += ' ' + target.name();
    }
    out += "\n\n";

    for (const auto &target : d->mTargets) {
      if (target.type() != targetType) continue;

      std::string name = target.name();
      std::replace(name.begin(), name.end(), '.', '_');

      out += name + "_SOURCES = " + target.sources() + '\n';
      if (!target.libAdd().empty())
        out += name + "_LIBADD = " + target.libAdd() + '\n';
      else
        out += name + "_LDADD = " + target.ldAdd() + '\n';
      out += name + "_LDFLAGS = " + target.ldFlags() + '\n';
    }
    out += '\n';
  }

  return out;
}
