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

#include "printer.h"

#include <boost/algorithm/string/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <list>
#include <set>
#include <string>

#include "class.h"
#include "code.h"
#include "enum.h"
#include "file.h"
#include "function.h"
#include "membervariable.h"
#include "style.h"
#include "typedef.h"

using namespace KODE;

class Printer::Private {
 public:
  explicit Private(Printer *parent)
      : mParent(parent),
        mCreationWarning(false),
        mLabelsDefineIndent(true),
        mIndentLabels(true),
        mGenerator("libkode") {}

  void addLabel(Code &code, const std::string &label);
  std::string classHeader(const Class &classObject, bool publicMembers,
                          bool nestedClass = false);
  std::string classImplementation(const Class &classObject,
                                  bool nestedClass = false);
  void addFunctionHeaders(Code &code, const Function::List &functions,
                          const std::string &className, int access);
  std::string formatType(const std::string &type) const;

  Printer *mParent;
  Style mStyle;
  bool mCreationWarning;
  bool mLabelsDefineIndent;
  bool mIndentLabels;
  std::string mGenerator;
  std::string mOutputDirectory;
  std::string mSourceFile;
};

void Printer::Private::addLabel(Code &code, const std::string &label) {
  if (!mIndentLabels) code.unindent();
  code += label;
  if (!mIndentLabels) code.indent();
}

std::string Printer::Private::formatType(const std::string &type) const {
  std::string s = type;
  if (s.back() == '*' || s.back() == '&') {
    if (s.at(s.length() - 2) != ' ') {
      // Turn "Foo*" into "Foo *" for readability
      s.insert(s.length() - 1, " ");
    }
  } else {
    s += ' ';
  }
  return s;
}

std::string Printer::Private::classHeader(const Class &classObject,
                                          bool publicMembers,
                                          bool nestedClass) {
  Code code;

  int numNamespaces = 0;
  if (!classObject.nameSpace().empty()) {
    std::list<std::string> nsList;
    std::string ns_str = classObject.nameSpace();
    boost::algorithm::split_regex(nsList, ns_str, boost::regex("::"));

    for (const auto &ns : nsList) {
      code += "namespace " + ns + " {";
      code.indent();
      ++numNamespaces;
    }
  }

  if (nestedClass) code.indent();

  if (!classObject.docs().empty()) {
    code += "/**";
    code.indent();
    code.addFormattedText(classObject.docs());
    code.unindent();
    code += " */";
  }

  std::string txt = "class ";
  if (!classObject.exportDeclaration().empty()) {
    auto upper = classObject.exportDeclaration();
    boost::algorithm::to_upper(upper);
    txt += upper + "_EXPORT ";
  }
  txt += mStyle.className(classObject.name());

  Class::List baseClasses = classObject.baseClasses();
  if (!baseClasses.empty()) {
    txt += " : ";
    bool first = true;
    for (const auto &bc : baseClasses) {
      if (first)
        first = false;
      else
        txt += ", ";

      txt += "public ";
      if (!bc.nameSpace().empty()) txt += bc.nameSpace() + "::";

      txt += bc.name();
    }
  }
  code += txt;

  if (nestedClass) {
    code.indent();
    code += '{';
  } else {
    code += '{';
    // We always want to indent here; so that Q_OBJECT and enums etc. are
    // indented.
    // However with mIndentLabels=false, we'll unindent before printing out
    // "public:".
    code.indent();
  }

  if (classObject.isQObject()) {
    code += "Q_OBJECT";
    code.newLine();
  }
  for (const auto &declMacro : classObject.declarationMacros()) {
    code += declMacro;
    code.newLine();
  }

  Class::List nestedClasses = classObject.nestedClasses();
  // Generate nestedclasses
  if (!classObject.nestedClasses().empty()) {
    addLabel(code, "public:");

    for (const auto &c : nestedClasses) {
      code += classHeader(c, false, true);
    }

    code.newLine();
  }

  Typedef::List typedefs = classObject.typedefs();
  if (typedefs.size() > 0) {
    addLabel(code, "public:");
    if (mLabelsDefineIndent) code.indent();

    for (const auto &t : typedefs) code += t.declaration();

    if (mLabelsDefineIndent) code.unindent();
    code.newLine();
  }

  Enum::List enums = classObject.enums();
  if (enums.size() > 0) {
    addLabel(code, "public:");
    if (mLabelsDefineIndent) code.indent();

    for (const auto &e : enums) code += e.declaration();

    if (mLabelsDefineIndent) code.unindent();
    code.newLine();
  }

  Function::List functions = classObject.functions();

  addFunctionHeaders(code, functions, classObject.name(), Function::Public);

  if (classObject.canBeCopied() && classObject.useDPointer() &&
      !classObject.memberVariables().empty()) {
    Function cc(classObject.name());
    cc.addArgument("const " + classObject.name() + '&');
    Function op("operator=", classObject.name() + '&');
    op.addArgument("const " + classObject.name() + '&');
    Function::List list;
    list.push_back(cc);
    list.push_back(op);
    addFunctionHeaders(code, list, classObject.name(), Function::Public);
  }

  addFunctionHeaders(code, functions, classObject.name(),
                     Function::Public | Function::Slot);
  addFunctionHeaders(code, functions, classObject.name(), Function::Signal);
  addFunctionHeaders(code, functions, classObject.name(), Function::Protected);
  addFunctionHeaders(code, functions, classObject.name(),
                     Function::Protected | Function::Slot);
  addFunctionHeaders(code, functions, classObject.name(), Function::Private);
  addFunctionHeaders(code, functions, classObject.name(),
                     Function::Private | Function::Slot);

  if (!classObject.memberVariables().empty()) {
    // Do we have any private function?
    bool hasPrivateFunc = false;
    bool hasPrivateSlot = false;
    for (const auto &f : functions) {
      if (f.access() == Function::Private) {
        hasPrivateFunc = true;
      } else if (f.access() == (Function::Private | Function::Slot)) {
        hasPrivateSlot = true;
      }
    }

    if (publicMembers)
      addLabel(code, "public:");
    else if (!hasPrivateFunc || hasPrivateSlot)
      addLabel(code, "private:");

    if (mLabelsDefineIndent) code.indent();

    if (classObject.useDPointer() && !classObject.memberVariables().empty()) {
      code += "class PrivateDPtr;";
      if (classObject.useSharedData())
        code += "QSharedDataPointer<PrivateDPtr> " +
                classObject.dPointerName() + ";";
      else
        code += "PrivateDPtr *" + classObject.dPointerName() + ";";
    } else {
      MemberVariable::List variables = classObject.memberVariables();
      for (const auto &v : variables) {
        if (v.access() == MemberVariable::Private) {
          std::string decl;
          if (v.isStatic()) decl += "static ";

          decl += formatType(v.type());

          decl += v.name() + ';';

          code += decl;
        }
      }

      if (mLabelsDefineIndent) code.unindent();
      addLabel(code, "protected:");
      if (mLabelsDefineIndent) code.indent();

      for (const auto &v : variables) {
        if (v.access() == MemberVariable::Protected) {
          std::string decl;
          if (v.isStatic()) decl += "static ";

          decl += formatType(v.type());

          decl += v.name() + ';';

          code += decl;
        }
      }

      if (mLabelsDefineIndent) code.unindent();
      addLabel(code, "public:");
      if (mLabelsDefineIndent) code.indent();

      for (const auto &v : variables) {
        if (v.access() == MemberVariable::Public) {
          std::string decl;
          if (v.isStatic()) decl += "static ";

          decl += formatType(v.type());

          decl += v.name() + ';';

          code += decl;
        }
      }
    }
    if (mLabelsDefineIndent) code.unindent();
  }

  if (!nestedClass)
    code.setIndent(0);
  else
    code.unindent();

  code += "};";

  for (int i = 0; i < numNamespaces; ++i) {
    code.unindent();
    code += "} // namespace end";
  }

  return code.text();
}

std::string Printer::Private::classImplementation(const Class &classObject,
                                                  bool nestedClass) {
  Code code;

  bool needNewLine = false;

  std::string functionClassName = classObject.name();
  if (nestedClass)
    functionClassName.insert(0, classObject.parentClassName() + "::");
  else if (!classObject.nameSpace().empty())
    functionClassName.insert(0, classObject.nameSpace() + "::");

  // Generate private class
  if (classObject.useDPointer() && !classObject.memberVariables().empty()) {
    Class privateClass(functionClassName + "::PrivateDPtr");
    if (classObject.useSharedData()) {
      privateClass.addBaseClass(Class("QSharedData"));
    }
    MemberVariable::List vars = classObject.memberVariables();
    Function ctor("PrivateDPtr");
    bool hasInitializers = false;
    for (const auto &v : vars) {
      privateClass.addMemberVariable(v);
      if (!v.initializer().empty()) {
        ctor.addInitializer(v.name() + '(' + v.initializer() + ')');
        hasInitializers = true;
      }
    }
    if (hasInitializers) privateClass.addFunction(ctor);
    code += classHeader(privateClass, true /*publicMembers*/);
    if (hasInitializers) code += classImplementation(privateClass);
  }

  // Generate static vars
  MemberVariable::List vars = classObject.memberVariables();
  for (const auto &v : vars) {
    if (!v.isStatic()) continue;

    // ## I thought the static int foo = 42; syntax was not portable?
    code += v.type() + functionClassName + "::" + v.name() + " = " +
            v.initializer() + ';';
    needNewLine = true;
  }

  if (needNewLine) code.newLine();

  Function::List functions = classObject.functions();
  for (const auto &f : functions) {
    // Omit signals
    if (f.access() == Function::Signal) continue;
    // Omit pure virtuals without a body
    if (f.virtualMode() == Function::PureVirtual && f.body().empty()) continue;

    code += mParent->functionSignature(f, functionClassName, true);

    std::list<std::string> inits = f.initializers();
    if (classObject.useDPointer() && !classObject.memberVariables().empty() &&
        f.name() == classObject.name()) {
      inits.push_back(classObject.dPointerName() + "(new PrivateDPtr)");
    }
    if (!classObject.useDPointer() && f.name() == classObject.name() &&
        f.arguments().empty()) {
      // Default constructor: add initializers for variables
      for (const auto &v : vars) {
        if (!v.initializer().empty()) {
          inits.push_back(v.name() + '(' + v.initializer() + ')');
        }
      }
    }

    if (!inits.empty()) {
      code.indent();
      code += ": " + boost::algorithm::join(inits, ", ");
      code.unindent();
    }

    code += '{';
    code.addBlock(f.body(), Code::defaultIndentation());

    if (classObject.useDPointer() && !classObject.useSharedData() &&
        !classObject.memberVariables().empty() &&
        f.name() == '~' + classObject.name()) {
      // Delete d pointer
      code.newLine();
      code.indent();
      code += "delete " + classObject.dPointerName() + ";";
      code += classObject.dPointerName() + " = 0;";
      code.unindent();
    }
    code += '}';
    code.newLine();
  }

  if (classObject.useDPointer() && classObject.canBeCopied() &&
      !classObject.memberVariables().empty()) {
    // print copy constructor
    Function cc(classObject.name());
    cc.addArgument("const " + functionClassName + "& other");

    Code body;
    if (!classObject.useSharedData()) {
      body += classObject.dPointerName() + " = new PrivateDPtr;";
      body += "*" + classObject.dPointerName() + " = *other." +
              classObject.dPointerName() + ";";
    }
    cc.setBody(body);

    code += mParent->functionSignature(cc, functionClassName, true);

    // call copy constructor of base classes
    std::list<std::string> list;
    Class::List baseClasses = classObject.baseClasses();
    for (const auto &b : baseClasses) {
      list.push_back(b.name() + "( other )");
    }
    if (classObject.useSharedData()) {
      list.push_back(classObject.dPointerName() + "( other." +
                     classObject.dPointerName() + " )");
    }
    if (!list.empty()) {
      code.indent();
      code += ": " + boost::algorithm::join(list, ", ");
      code.unindent();
    }

    code += '{';
    code.addBlock(cc.body(), Code::defaultIndentation());
    code += '}';
    code.newLine();

    // print assignment operator
    Function op("operator=", functionClassName + "& ");
    op.addArgument("const " + functionClassName + "& other");

    body.clear();
    body += "if ( this == &other )";
    body.indent();
    body += "return *this;";
    body.unindent();
    body.newLine();
    if (classObject.useSharedData())
      body += classObject.dPointerName() + " = other." +
              classObject.dPointerName() + ";";
    else
      body += "*" + classObject.dPointerName() + " = *other." +
              classObject.dPointerName() + ";";
    for (const auto &b : baseClasses) {
      body += "* static_cast<" + b.name() + " *>(this) = other;";
    }

    body.newLine();
    body += "return *this;";
    op.setBody(body);

    code += mParent->functionSignature(op, functionClassName, true);
    code += '{';
    code.addBlock(op.body(), Code::defaultIndentation());
    code += '}';
    code.newLine();
  }

  // Generate nested class functions
  if (!classObject.nestedClasses().empty()) {
    for (const auto &nestedClass : classObject.nestedClasses()) {
      code += classImplementation(nestedClass, true);
    }
  }

  return code.text();
}

void Printer::Private::addFunctionHeaders(Code &code,
                                          const Function::List &functions,
                                          const std::string &className,
                                          int access) {
  bool needNewLine = false;
  bool hasAccess = false;

  for (const auto &f : functions) {
    if (f.access() == access) {
      if (!hasAccess) {
        addLabel(code, f.accessAsString() + ':');
        hasAccess = true;
      }
      if (mLabelsDefineIndent) code.indent();
      if (!f.docs().empty()) {
        code += "/**";
        code.indent();
        code.addFormattedText(f.docs());
        code.unindent();
        code += " */";
      }
      code += mParent->functionSignature(f, className, false) + ';';
      if (mLabelsDefineIndent) code.unindent();
      needNewLine = true;
    }
  }

  if (needNewLine) code.newLine();
}

Printer::Printer() : d(new Private(this)) {}

Printer::Printer(const Printer &other) : d(new Private(this)) {
  *d = *other.d;
  d->mParent = this;
}

Printer::Printer(const Style &style) : d(new Private(this)) {
  d->mStyle = style;
}

Printer::~Printer() { delete d; }

Printer &Printer::operator=(const Printer &other) {
  if (this == &other) return *this;

  *d = *other.d;
  d->mParent = this;

  return *this;
}

void Printer::setCreationWarning(bool v) { d->mCreationWarning = v; }

void Printer::setGenerator(const std::string &generator) {
  d->mGenerator = generator;
}

void Printer::setOutputDirectory(const std::string &outputDirectory) {
  d->mOutputDirectory = outputDirectory;
}

void Printer::setSourceFile(const std::string &sourceFile) {
  d->mSourceFile = sourceFile;
}

void Printer::setLabelsDefineIndent(bool b) { d->mLabelsDefineIndent = b; }

void Printer::setIndentLabels(bool b) { d->mIndentLabels = b; }

std::string Printer::functionSignature(const Function &function,
                                       const std::string &className,
                                       bool forImplementation) {
  std::string s;

  if (function.isStatic() && !forImplementation) {
    s += "static ";
  }

  if (function.virtualMode() != Function::NotVirtual && !forImplementation) {
    s += "virtual ";
  }

  std::string ret = function.returnType();
  if (!ret.empty()) {
    s += d->formatType(ret);
  }

  if (forImplementation) s += d->mStyle.className(className) + "::";

  if (className == function.name()) {
    // Constructor
    s += d->mStyle.className(function.name());
  } else {
    s += function.name();
  }

  s += '(';
  if (function.hasArguments()) {
    std::list<std::string> arguments;
    for (const auto &argument : function.arguments()) {
      if (!forImplementation) {
        arguments.push_back(argument.headerDeclaration());
      } else {
        arguments.push_back(argument.bodyDeclaration());
      }
    }
    s += ' ' + boost::algorithm::join(arguments, ", ") + ' ';
  }
  s += ')';

  if (function.isConst()) s += " const";

  if (function.virtualMode() == Function::PureVirtual) s += " = 0";

  return s;
}

std::string Printer::creationWarning() const {
  // Create warning about generated file
  std::string str = "// This file is generated by " + d->mGenerator;
  if (!d->mSourceFile.empty()) str += " from " + d->mSourceFile;

  str += ".\n";

  str += "// All changes you do to this file will be lost.";

  return str;
}

std::string Printer::licenseHeader(const File &file) const {
  Code code;

  const std::list<std::string> copyrights = file.copyrightStrings();
  if (!file.project().empty() || !copyrights.empty() ||
      !file.license().text().empty()) {
    code += "/*";
    code.setIndent(4);

    if (!file.project().empty()) {
      code += "This file is part of " + file.project() + '.';
      code.newLine();
    }

    if (!copyrights.empty()) {
      code.addBlock(boost::algorithm::join(copyrights, "\n"));
      code.newLine();
    }

    code.addBlock(file.license().text());
    code.setIndent(0);
    code += "*/";
  }

  return code.text();
}

static std::list<std::string> commonLeft(const std::list<std::string> &l1,
                                         const std::list<std::string> &l2) {
  std::list<std::string> r;
  auto it1 = l1.begin();
  auto it2 = l2.begin();
  for (; it1 != l1.end() && it2 != l2.end(); ++it1, ++it2) {
    if (*it1 == *it2)
      r.push_back(*it1);
    else
      return r;
  }
  return r;
}

void Printer::printHeader(const File &file) {
  Code out;

  if (d->mCreationWarning) out += creationWarning();

  out.addBlock(licenseHeader(file));

  // Create include guard
  boost::filesystem::path p(file.filenameHeader());
  std::string className =
      p.filename().string();  // remove path, keep only filename
  std::replace(className.begin(), className.end(), '.', '_');

  std::string includeGuard;
  if (!file.nameSpace().empty()) includeGuard += file.nameSpace() + '_';

  includeGuard += className;
  boost::algorithm::to_upper(includeGuard);
  std::replace(includeGuard.begin(), includeGuard.end(), '.', '_');

  out += "#ifndef " + includeGuard;
  out += "#define " + includeGuard;

  out.newLine();

  // Create includes
  std::set<std::string> processed;
  const Class::List classes = file.classes();
  for (const auto &cl : classes) {
    assert(!cl.name().empty());
    std::list<std::string> includes = cl.headerIncludes();
    if (cl.useSharedData()) includes.push_back("QtCore/QSharedData");
    // qDebug() << "includes=" << includes;
    for (const auto &i : includes) {
      auto it = processed.find(i);
      if (it == processed.end()) {
        out += "#include <" + i + '>';
        processed.insert(i);
      }
    }
  }

  if (!processed.empty()) out.newLine();

  if (!file.headerCode()->isEmpty()) {
    out += *file.headerCode();
    out.newLine();
  }

  // Create forward declarations
  processed.clear();
  for (const auto &c : classes) {
    const std::list<std::string> decls = c.forwardDeclarations();
    std::copy(decls.begin(), decls.end(),
              std::inserter(processed, processed.begin()));
  }
  std::list<std::string> fwdClasses;
  std::copy(processed.begin(), processed.end(), std::back_inserter(fwdClasses));
  fwdClasses.sort();
  fwdClasses.push_back(
      std::string());  // for proper closing of the namespace blocks below

  std::list<std::string> prevNS;

  for (const auto &fwd : fwdClasses) {
    // handle namespaces by opening and closing namespace blocks accordingly
    // the sorting will ensure sensible grouping
    std::list<std::string> seg;
    boost::algorithm::split_regex(seg, fwd, boost::regex("::"));
    std::list<std::string> ns = seg;
    ns.pop_back();
    const std::string clas = seg.empty() ? std::string() : seg.back();
    const std::list<std::string> common = commonLeft(ns, prevNS);
    for (int i = common.size(); i < prevNS.size(); ++i) {
      out.unindent();
      out += "}";
      out.newLine();
    }

    int i = 0;
    for (const auto &ns_seg : ns) {
      if (i >= common.size()) {
        out += "namespace " + ns_seg + " {";
        out.indent();
      }
      ++i;
    }

    if (!clas.empty()) {
      const bool isQtClass = clas.front() == 'Q';
      if (isQtClass) out += "QT_BEGIN_NAMESPACE";
      out += "class " + clas + ';';
      if (isQtClass) out += "QT_END_NAMESPACE";
    }
    prevNS = ns;
  }

  if (!processed.empty()) out.newLine();

  if (!file.nameSpace().empty()) {
    out += "namespace " + file.nameSpace() + " {";
    out.newLine();
  }

  // Create enums
  Enum::List enums = file.fileEnums();
  for (const auto &e : enums) {
    out += e.declaration();
    out.newLine();
  }

  // Create content
  for (const auto &c : classes) {
    out.addBlock(d->classHeader(c, false));
    out.newLine();
  }

  if (!file.nameSpace().empty()) {
    out += '}';
    out.newLine();
  }

  // Finish file
  out += "#endif";

  // Print to file
  std::string filename = file.filenameHeader();

  if (!d->mOutputDirectory.empty())
    filename.insert(0, d->mOutputDirectory + '/');

  std::ofstream header;
  header.open(filename, std::ios::out | std::ios::trunc);
  if (!header.is_open()) {
    std::cerr << "Can't open '" << filename << "' for writing." << std::endl;
    return;
  }

  header << out.text();

  header.close();
}

void Printer::printImplementation(const File &file, bool createHeaderInclude) {
  Code out;

  if (d->mCreationWarning) out += creationWarning();

  out.addBlock(licenseHeader(file));

  out.newLine();

  // Create includes
  if (createHeaderInclude) {
    out += "#include \"" + file.filenameHeader() + "\"";
    out.newLine();
  }

  std::list<std::string> includes = file.includes();
  for (const auto &i : includes) out += "#include <" + i + '>';

  if (!includes.empty()) out.newLine();

  // Create class includes
  std::set<std::string> processed;
  Class::List classes = file.classes();
  for (const auto &c : classes) {
    std::list<std::string> includes = c.includes();
    for (const auto &i : includes) {
      auto it = processed.find(i);
      if (it == processed.end()) {
        out += "#include <" + i + '>';
        processed.insert(i);
      }
    }
  }

  if (!processed.empty()) out.newLine();

  if (!file.nameSpace().empty()) {
    out += "namespace " + file.nameSpace() + " {";
    out.newLine();
  }

  // 'extern "C"' declarations
  const std::list<std::string> externCDeclarations = file.externCDeclarations();
  if (!externCDeclarations.empty()) {
    out += "extern \"C\" {";
    for (const auto &d : externCDeclarations) {
      out += d + ';';
    }
    out += '}';
    out.newLine();
  }

  // File variables
  Variable::List vars = file.fileVariables();
  for (const auto &v : vars) {
    std::string str;
    if (v.isStatic()) str += "static ";
    str += v.type() + ' ' + v.name() + ';';
    out += str;
  }

  if (!vars.empty()) out.newLine();

  // File code
  if (!file.fileCode()->isEmpty()) {
    out += *file.fileCode();
    out.newLine();
  }

  // File functions
  Function::List funcs = file.fileFunctions();
  for (const auto &f : funcs) {
    out += functionSignature(f);
    out += '{';
    out.addBlock(f.body(), Code::defaultIndentation());
    out += '}';
    out.newLine();
  }

  // Classes
  for (const auto &c : classes) {
    std::string str = d->classImplementation(c);
    if (!str.empty()) out += d->classImplementation(c);
  }

  if (!file.nameSpace().empty()) {
    out += "}";
    out.newLine();
  }

  // Print to file
  std::string filename = file.filenameImplementation();

  if (!d->mOutputDirectory.empty())
    filename.insert(0, d->mOutputDirectory + '/');

  std::ofstream implementation;
  implementation.open(filename, std::ios::out | std::ios::trunc);
  if (!implementation.is_open()) {
    std::cerr << "Can't open '" << filename << "' for writing." << std::endl;
    return;
  }

  implementation << out.text();

  implementation.close();
}

#if 0  // TODO: port to cmake
void Printer::printAutoMakefile(const AutoMakefile &am) {
  std::string filename = "Makefile.am";

  if (!d->mOutputDirectory.empty()) filename.prepend(d->mOutputDirectory + '/');

  //  KSaveFile::simpleBackupFile( filename, std::string(), ".backup" );

  QFile file(filename);
  if (!file.open(QIODevice::WriteOnly)) {
    qWarning("Can't open '%s' for writing.", qPrintable(filename));
    return;
  }

  QTextStream ts(&file);

  ts << am.text();
}
#endif
