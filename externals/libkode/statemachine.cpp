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

#include "statemachine.h"

#include <boost/algorithm/string/join.hpp>
#include <list>
#include <map>
#include <string>
#include <utility>

#include "code.h"

using namespace KODE;

class StateMachine::Private {
 public:
  std::map<std::string, Code> mStateMap;
  std::string mInitialState;
};

StateMachine::StateMachine() : d(new Private) {}

StateMachine::StateMachine(const StateMachine &other) : d(new Private) {
  *d = *other.d;
}

StateMachine::~StateMachine() { delete d; }

StateMachine &StateMachine::operator=(const StateMachine &other) {
  if (this == &other) return *this;

  *d = *other.d;

  return *this;
}

void StateMachine::setState(const std::string &state, const Code &code) {
  d->mStateMap[state] = code;

  if (d->mInitialState.empty()) d->mInitialState = state;
}

void StateMachine::setInitialState(const std::string &state) {
  d->mInitialState = state;
}

Code StateMachine::stateDefinition() const {
  Code code;

  std::list<std::string> states;
  for (const auto &elem : d->mStateMap) {
    states.push_back(elem.first);
  }

  code += "enum State { " + boost::algorithm::join(states, ", ") + " };";
  code += "State state = " + d->mInitialState + ';';

  return code;
}

Code StateMachine::transitionLogic() const {
  Code code;

  code += "switch( state ) {";
  code.indent();

  for (const auto &elem : d->mStateMap) {
    code += "case " + elem.first + ':';
    code.indent();
    code.addBlock(elem.second);
    code += "break;";
    code.unindent();
  }

  code += "default:";
  code.indent();
  code += "break;";
  code.unindent();

  code.unindent();
  code += '}';

  return code;
}
