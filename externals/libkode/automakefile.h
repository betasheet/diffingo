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
#ifndef KODE_AUTOMAKEFILE_H
#define KODE_AUTOMAKEFILE_H

#include <list>
#include <string>

namespace KODE {

/**
 * This class abstracts an AutoMake file (Makefile.am)
 * which is used by the autotools to create Makefiles.
 */
class AutoMakefile {
 public:
  class Target {
   public:
    typedef std::list<Target> List;

    /**
     * Creates a new target.
     */
    Target();

    /**
     * Creates a new target from @other.
     */
    Target(const Target &other);

    /**
     * Creates a new target with a given @type and @name.
     */
    Target(const std::string &type, const std::string &name);

    /**
     * Destroys the target.
     */
    ~Target();

    /**
     * Assignment operator.
     */
    Target &operator=(const Target &other);

    /**
     * Sets the @param type of the target.
     */
    void setType(const std::string &type);

    /**
     * Returns the type of the target.
     */
    std::string type() const;

    /**
     * Sets the @param name of the target.
     */
    void setName(const std::string &name);

    /**
     * Returns the name of the target.
     */
    std::string name() const;

    /**
     * Sets the @param sources of the target.
     */
    void setSources(const std::string &sources);

    /**
     * Returns the sources of the target.
     */
    std::string sources() const;

    /**
     * Sets the LIBADD of the target to @param libAdd.
     */
    void setLibAdd(const std::string &libAdd);

    /**
     * Returns the LIBADD of the target.
     */
    std::string libAdd() const;

    /**
     * Sets the LDADD of the target to @param ldAdd.
     */
    void setLdAdd(const std::string &ldAdd);

    /**
     * Returns the LDADD of the target.
     */
    std::string ldAdd() const;

    /**
     * Sets the LDFLAGS of the target to @param ldFlags.
     */
    void setLdFlags(const std::string &ldFlags);

    /**
     * Returns the LDFLAGS of the target.
     */
    std::string ldFlags() const;

   private:
    class Private;
    Private *d;
  };

  /**
   * Creates a new automake file.
   */
  AutoMakefile();

  /**
   * Creates a new automake file from @param other.
   */
  AutoMakefile(const AutoMakefile &other);

  /**
   * Destroys the automake file.
   */
  ~AutoMakefile();

  /**
   * Assignment operator.
   */
  AutoMakefile &operator=(const AutoMakefile &other);

  /**
   * Add @param target to the automake file.
   */
  void addTarget(const Target &target);

  /**
   * Returns the list of targets of the automake file.
   */
  Target::List targets() const;

  /**
   * Adds a custom entry to the automake file.
   *
   * @param variable The variable name.
   * @param value The value of the variable.
   */
  void addEntry(const std::string &variable,
                const std::string &value = std::string());

  /**
   * Adds an empty line to the automake file.
   */
  void newLine();

  /**
   * Returns the textual presentation of the automake file.
   */
  std::string text() const;

 private:
  class Private;
  Private *d;
};
}

#endif
