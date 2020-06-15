/*
 * node_types.h
 *
 * Distributed under the MIT License (MIT).
 *
 * Copyright (c) 2015 Eric Seckler
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef SRC_SPEC_AST_NODE_TYPES_H_
#define SRC_SPEC_AST_NODE_TYPES_H_

namespace diffingo {
namespace spec {
namespace ast {

class Attribute;
class AttributeMap;
class Function;
class ID;
class Module;
class Location;
class Scope;
class Transform;
namespace constant { class Bitset; }
namespace constant { class Bool; }
namespace constant { class Constant; }
namespace constant { class Double; }
namespace constant { class Enum; }
namespace constant { class Integer; }
namespace constant { class None; }
namespace constant { class String; }
namespace constant { class Tuple; }
namespace ctor { class Bytes; }
namespace ctor { class Ctor; }
namespace ctor { class RegExp; }
namespace declaration { class Constant; }
namespace declaration { class Declaration; }
namespace declaration { class Function; }
namespace declaration { class Transform; }
namespace declaration { class Type; }
namespace declaration { namespace unit_instantiation { class Instantiation; } }
namespace declaration { namespace unit_instantiation { class Item; } }
namespace declaration { namespace unit_instantiation { class Field; } }
namespace declaration { namespace unit_instantiation { class Property; } }
namespace expression { class Assign; }
namespace expression { class Conditional; }
namespace expression { class Constant; }
namespace expression { class Ctor; }
namespace expression { class Expression; }
namespace expression { class Find; }
namespace expression { class Function; }
namespace expression { class ID; }
namespace expression { class Lambda; }
namespace expression { class ListComprehension; }
namespace expression { class MemberAttribute; }
namespace expression { class Operator; }
namespace expression { class ParserState; }
namespace expression { class Transform; }
namespace expression { class Type; }
namespace expression { class Variable; }
namespace type { class Any; }
namespace type { class Bitset; }
namespace type { class Bool; }
namespace type { class Bytes; }
namespace type { class CAddr; }
namespace type { class Container; }
namespace type { class Double; }
namespace type { class Enum; }
namespace type { class Integer; }
namespace type { class List; }
namespace type { class Map; }
namespace type { class MemberAttribute; }
namespace type { class RegExp; }
namespace type { class Set; }
namespace type { class Sink; }
namespace type { class String; }
namespace type { class Tuple; }
namespace type { class Type; }
namespace type { class TypeType; }
namespace type { class Unknown; }
namespace type { class UnknownElementType; }
namespace type { class Vector; }
namespace type { class Void; }
namespace type { namespace bitfield { class Bitfield; } }
namespace type { namespace bitfield { class Bits; } }
namespace type { namespace function { class Function; } }
namespace type { namespace function { class Parameter; } }
namespace type { namespace function { class Result; } }
namespace type { namespace unit { class Unit; } }
namespace type { namespace unit { namespace item {class Item; } } }
namespace type { namespace unit { namespace item { class Property; } } }
namespace type { namespace unit { namespace item { class Variable; } } }
namespace type { namespace unit { namespace item { namespace field { class AtomicType; } } } }  // NOLINT
namespace type { namespace unit { namespace item { namespace field { class Constant; } } } }  // NOLINT
namespace type { namespace unit { namespace item { namespace field { class Ctor; } } } }  // NOLINT
namespace type { namespace unit { namespace item { namespace field { class Field; } } } }  // NOLINT
namespace type { namespace unit { namespace item { namespace field { class Unit; } } } }  // NOLINT
namespace type { namespace unit { namespace item { namespace field { class Unknown; } } } }  // NOLINT
namespace type { namespace unit { namespace item { namespace field { namespace container { class Container; } } } } }  // NOLINT
namespace type { namespace unit { namespace item { namespace field { namespace container { class List; } } } } }  // NOLINT
namespace type { namespace unit { namespace item { namespace field { namespace container { class Vector; } } } } }  // NOLINT
namespace type { namespace unit { namespace item { namespace field { namespace switch_ { class Switch; } } } } }  // NOLINT
namespace type { namespace unit { namespace item { namespace field { namespace switch_ { class Case; } } } } }  // NOLINT
namespace variable { class Variable; }
namespace variable { class Local; }

}  // namespace ast
}  // namespace spec
}  // namespace diffingo

#endif  // SRC_SPEC_AST_NODE_TYPES_H_
