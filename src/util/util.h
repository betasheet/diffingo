/*
 * util.h
 *
 * TODO adopted from from Binpac++, add their copyright.
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

#ifndef SRC_UTIL_UTIL_H_
#define SRC_UTIL_UTIL_H_

#include <cxxabi.h>
#include <stddef.h>
#include <tinyformat.h>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <functional>
#include <iterator>
#include <list>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <utility>

namespace util {

typedef std::list<std::string> path_list;

template <typename... Args>
std::string fmt(const char* fmt, const Args&... args) {
  return tfm::format(fmt, args...);
}

extern std::list<std::string> strsplit(std::string s, std::string delim = " ");

template <typename T>
std::string strjoin(const std::list<T>& l, std::string delim = "") {
  std::string result;
  bool first = true;

  for (typename std::list<T>::const_iterator i = l.begin(); i != l.end(); i++) {
    if (!first) result += delim;
    result += std::string(*i);
    first = false;
  }

  return result;
}

template <typename T>
std::string strjoin(const std::set<T>& l, std::string delim = "") {
  std::string result;
  bool first = true;

  for (typename std::set<T>::const_iterator i = l.begin(); i != l.end(); i++) {
    if (!first) result += delim;
    result += std::string(*i);
    first = false;
  }

  return result;
}

template <typename iterator>
std::string strjoin(const iterator& begin, const iterator& end,
                    std::string delim = "") {
  std::string result;
  bool first = true;

  for (iterator i = begin; i != end; i++) {
    if (!first) result += delim;
    result += std::string(*i);
    first = false;
  }

  return result;
}

extern std::string strreplace(const std::string& s, const std::string& o,
                              const std::string& n);
extern std::string strtolower(const std::string& s);
extern std::string strtoupper(const std::string& s);
extern std::string strtrim(const std::string& s);

inline bool startsWith(const std::string& s, const std::string& prefix) {
  return s.find(prefix) == 0;
}
extern bool endsWith(const std::string& s, const std::string& suffix);

/// Returns a simple (non-crypto) hash value of a std::string.
extern uint64_t hash(const std::string& str);

/// Returns a simple (non-crypto) hash value of a memory block.
extern uint64_t hash(const char* data, size_t len);

/// Converts an integer into a std::string relative to a given base.
///
/// value: The value.
///
/// base: The base.
///
/// n: The max number of characters to include. If the final std::string would
/// longer than this, it's cut off. If smaller than zero, includes all.
///
/// Returns: The std::string.
extern std::string uitoa_n(uint64_t value, int base, int n = -1);

/// Expands escape sequences. The following escape sequences are supported:
///
///    ============   ============================
///    Escape         Result
///    ============   ============================
///    \\             Backslash
///    \\n            Line feed
///    \\r            Carriage return
///    \\t            Tabulator
///    \\uXXXX        16-bit Unicode codepoint
///    \\UXXXXXXXX    32-bit Unicode codepoint
///    \\xXX          8-bit hex value
///    ============   ============================
///
/// str: std::string - The std::string to expand.
///
/// Returns: A UTF8 std::string with escape sequences expanded.
///
/// Raises: std::runtime_error - Raised when an illegal sequence was found.
extern std::string expandEscapes(const std::string& s);

/// Escapes non-printable and control characters in an UTF8 std::string. This
/// produces a std::string that can be reverted by expandEscapes().
///
/// str: std::string - The std::string to escape.
///
/// Returns: The escaped std::string.
extern std::string escapeUTF8(const std::string& s);

/// Escapes non-printable characters in a raw std::string. This produces a
/// std::string
/// that can be reverted by expandEscapes().
///
/// str: std::string - The std::string to escape.
///
/// Returns: The escaped std::string.
extern std::string escapeBytes(const std::string& s);

/// Turns an arbitrary std::string into something that can be used as C-level
/// identifier.
///
/// s: The std::string to convert.
///
/// ensure_non_keyword: If true, the returned ID will be expanded to make
/// sure it won't accidentdally match a compiler keyword.
extern std::string toIdentifier(const std::string& s,
                                bool ensure_non_keyword = false);

/// Returns the curren time in seconds since the epoch.
extern double currentTime();

extern bool pathExists(const std::string& path);
extern bool pathIsFile(const std::string& path);
extern bool pathIsDir(const std::string& path);
extern std::string pathJoin(const std::string& p1, const std::string& p2);
extern std::string findInPaths(const std::string& file, const path_list& paths);
extern std::string dirname(const std::string& path);
extern std::string basename(const std::string& path);
extern bool makeDir(const std::string& path);

extern void abort_with_backtrace();

template <class T>
std::string::const_iterator atoi_n(std::string::const_iterator s,
                                   std::string::const_iterator e, int base,
                                   T* result) {
  T n = 0;
  int neg = 0;

  if (s != e && *s == '-') {
    neg = 1;
    ++s;
  }

  bool first = true;

  for (; s != e; s++) {
    auto c = *s;
    unsigned int d;

    if (isdigit(c))
      d = c - '0';

    else if (c >= 'a' && c < 'a' - 10 + base)
      d = c - 'a' + 10;

    else if (c >= 'A' && c < 'A' - 10 + base)
      d = c - 'A' + 10;

    else if (!first)
      break;

    else
      throw std::runtime_error("cannot decode number");

    n = n * base + d;
    first = false;
  }

  if (neg)
    *result = -n;
  else
    *result = n;

  return s;
}

// From http://stackoverflow.com/questions/10420380/c-zip-variadic-templates.
template <typename A, typename B>
std::list<std::pair<A, B>> zip2(const std::list<A>& lhs,
                                const std::list<B>& rhs) {
  std::list<std::pair<A, B>> result;
  for (std::pair<typename std::list<A>::const_iterator,
                 typename std::list<B>::const_iterator>
           iter = std::pair<typename std::list<A>::const_iterator,
                            typename std::list<B>::const_iterator>(
               lhs.cbegin(), rhs.cbegin());
       iter.first != lhs.end() && iter.second != rhs.end();
       ++iter.first, ++iter.second)
    result.push_back(std::pair<A, B>(*iter.first, *iter.second));
  return result;
}

// Returns the keys of a map as a set.
template <typename A, typename B>
std::set<A> map_keys(const std::map<A, B> m) {
  std::set<A> l;

  for (auto i : m) l.insert(i.first);

  return l;
}

// Returns the values of a map as a set.
template <typename A, typename B>
std::set<A> map_values(const std::map<A, B> m) {
  std::set<A> l;

  for (auto i : m) l.insert(i.second);

  return l;
}

// Returns the difference of two sets. This is a convience wrapper around
// std::set_difference.
template <typename A, typename Compare = std::less<A>>
std::set<A, Compare> set_difference(std::set<A, Compare> a,
                                    std::set<A, Compare> b) {
  std::set<A, Compare> r;
  std::set_difference(a.begin(), a.end(), b.begin(), b.end(),
                      std::inserter(r, r.end()), Compare());
  return r;
}

// Returns the intersection of two sets. This is a convience wrapper around
// std::set_intersection.
template <typename A, typename Compare = std::less<A>>
std::set<A, Compare> set_intersection(std::set<A, Compare> a,
                                      std::set<A, Compare> b) {
  std::set<A, Compare> r;
  std::set_intersection(a.begin(), a.end(), b.begin(), b.end(),
                        std::inserter(r, r.end()), Compare());
  return r;
}

// Returns the union of two sets. This is a convience wrapper around
// std::set_union.
template <typename A, typename Compare = std::less<A>>
std::set<A, Compare> set_union(std::set<A, Compare> a, std::set<A, Compare> b) {
  std::set<A, Compare> r;
  std::set_union(a.begin(), a.end(), b.begin(), b.end(),
                 std::inserter(r, r.end()), Compare());
  return r;
}

template <typename A>
std::string type_name(const A& a) {
  int status;
  char* name = abi::__cxa_demangle(typeid(a).name(), nullptr, nullptr, &status);
  return name;
}

template <typename A>
std::string type_name() {
  int status;
  char* name = abi::__cxa_demangle(typeid(A).name(), nullptr, nullptr, &status);
  return name;
}

}  // namespace util

#endif  // SRC_UTIL_UTIL_H_
