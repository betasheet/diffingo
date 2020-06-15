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
#ifndef KODE_MEMBERVARIABLE_H
#define KODE_MEMBERVARIABLE_H

#include <list>
#include <string>

#include "variable.h"

namespace KODE {

/**
 * This class represents a member variable.
 *
 * It's basically a @see Variable with a
 * special formatted name.
 */
class MemberVariable : public Variable {
 public:
  typedef std::list<MemberVariable> List;

  /**
   * The different access specifiers.
   *
   * @li Public     - Public access
   * @li Protected  - Protected access
   * @li Private    - Private access
   * @li Signal     - Qt Signal
   * @li Slot       - Qt Slot
   */
  enum AccessSpecifier {
    Public = 1,
    Protected = 2,
    Private = 4
  };

  /**
   * Creates a new member variable.
   */
  MemberVariable();

  /**
   * Creates a new member variable from @param other.
   */
  MemberVariable(const MemberVariable &other);

  /**
   * Creates a new member variable of the given @param type
   * and with the given @param name.
   *
   * @param isStatic If true the variable is marked as static.
   */
  MemberVariable(const std::string &name, const std::string &type,
                 bool isStatic = false, int access = Private);

  /**
   * Destroys the member variable.
   */
  ~MemberVariable();

  /**
   * Assignment operator.
   */
  MemberVariable &operator=(const MemberVariable &other);

  /**
   * Sets the access @param specifier of the function.
   */
  void setAccess(int specifier);

  /**
   * Returns the access specifier of the function.
   */
  int access() const;

  /**
   * Returns the name that will be used for a member variable.
   * This is the same result as creating a MemberVariable(inputName, someType)
   * and extracting the name() afterwards.
   */
  static std::string memberVariableName(const std::string &inputName);

 private:
  class MVPrivate;
  MVPrivate *d;
};
}

#endif
