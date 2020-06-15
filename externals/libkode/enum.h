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
#ifndef KODE_ENUM_H
#define KODE_ENUM_H

#include <list>
#include <string>
#include <tuple>

#include "function.h"

namespace KODE {
class Function;
}  // namespace KODE

namespace KODE {

class Enum {
 public:
  typedef std::list<Enum> List;
  typedef std::tuple<std::string, bool, int> label_with_value;

  /**
   * Creates a new enum.
   */
  Enum();

  /**
   * Creates a new enum from @param other.
   */
  Enum(const Enum &other);

  /**
   * Creates a new enum with the given name and enum labels.
   *
   * @param name The name of the enum.
   * @param enums The labels of the enum.
   * @param combinable If true the integer associations will be a power of two,
   *                   so the enum flags will be combinable.
   */
  Enum(const std::string &name, const std::list<std::string> &enums,
       bool combinable = false, bool use_enum_cls = false);

  /**
   * Creates a new enum with the given name and enum labels + values.
   *
   * @param name The name of the enum.
   * @param enums The values of the enum.
   */
  Enum(const std::string &name, const std::list<label_with_value> &enums,
       bool use_enum_cls = false);

  /**
   * Destroys the enum.
   */
  ~Enum();

  /**
   * Assignment operator.
   */
  Enum &operator=(const Enum &other);

  /**
   * Return name of enum.
  */
  std::string name() const;

  /**
   * Returns the textual presentation of the enum.
   */
  std::string declaration() const;

  KODE::Function parserMethod() const;
  KODE::Function writerMethod() const;

 private:
  class Private;
  Private *d;
};
}

#endif
