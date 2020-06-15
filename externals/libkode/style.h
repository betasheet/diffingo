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
#ifndef KODE_STYLE_H
#define KODE_STYLE_H

#include <string>

namespace KODE {

/**
 * This class encapsulates the style settings.
 *
 * You can reimplement it to give the generated code
 * a custom style.
 */
class Style {
 public:
  /**
   * Creates a new style.
   */
  Style();

  /**
   * Creates a new style from @param other.
   */
  Style(const Style &other);

  /**
   * Destroys the style.
   */
  virtual ~Style();

  /**
   * Assignment operator.
   */
  Style &operator=(const Style &other);

  /**
   * Converts the class name.
   *
   * The default implementation upper cases the first
   * character of the name.
   */
  virtual std::string className(const std::string &str);

  /**
   * Returns a new version of @param str with the first
   * character be uppercase.
   */
  static std::string upperFirst(const std::string &str);

  /**
   * Returns a new version of @param str converted to a format
   * which could be compiled
   */
  static std::string sanitize(const std::string &str);

  /**
   * Returns a new version of @param str with the first
   * character be lowercase.
   */
  static std::string lowerFirst(const std::string &str);

  /**
   * Returns a new version of @param str after making it suitable for usage
   * as a C++ identifier.
   */
  static std::string makeIdentifier(const std::string &str);

 private:
  class Private;
  Private *d;
};
}

#endif
