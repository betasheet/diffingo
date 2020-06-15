/*
 * util.cc
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

#include "util/util.h"

#include <sys/time.h>
#include <sys/stat.h>
#include <execinfo.h>
#include <string.h>
#include <utf8proc.h>
#include <utf8proc.c>

#include <algorithm>
#include <string>

#if 0
std::string util::fmt(const char* fmt, ...) {
    char buffer[1024];

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, ap);
    buffer[sizeof(buffer)-1] = '\0';
    va_end(ap);

    return std::string(buffer);
}
#endif

std::list<std::string> util::strsplit(std::string s, std::string delim) {
  std::list<std::string> l;

  while (true) {
    size_t p = s.find(delim);

    if (p == std::string::npos) break;

    l.push_back(s.substr(0, p));

    // FIXME: Don't understand why directly assigning to s doesn't work.
    std::string t = s.substr(p + delim.size(), std::string::npos);
    s = t;
  }

  l.push_back(s);
  return l;
}

std::string util::strreplace(const std::string& s, const std::string& o,
                             const std::string& n) {
  std::string r = s;

  while (true) {
    auto i = r.find(o);

    if (i == std::string::npos) break;

    r.replace(i, o.size(), n);
  }

  return r;
}

std::string util::strtolower(const std::string& s) {
  std::string t = s;
  std::transform(t.begin(), t.end(), t.begin(), ::tolower);
  return t;
}

std::string util::strtoupper(const std::string& s) {
  std::string t = s;
  std::transform(t.begin(), t.end(), t.begin(), ::toupper);
  return t;
}

std::string util::strtrim(const std::string& s) {
  auto t = s;
  t.erase(t.begin(),
          std::find_if(t.begin(), t.end(),
                       std::not1(std::ptr_fun<int, int>(std::isspace))));
  t.erase(std::find_if(t.rbegin(), t.rend(),
                       std::not1(std::ptr_fun<int, int>(std::isspace))).base(),
          t.end());
  return t;
}

uint64_t util::hash(const std::string& str) {
  return util::hash(str.data(), str.size());
}

uint64_t util::hash(const char* data, size_t len) {
  uint64_t h = 0;

  while (len--) h = (h << 5) - h + (uint64_t)*data++;

  return h;
}

std::string util::uitoa_n(uint64_t value, int base, int n) {
  static char dig[] =
      "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

  assert(base <= strlen(dig));

  std::string s;

  do {
    s.append(1, dig[value % base]);
    value /= base;
  } while (value && (n < 0 || s.size() < n - 1));

  return s;
}

bool util::pathExists(const std::string& path) {
  struct stat st;
  return ::stat(path.c_str(), &st) == 0;
}

bool util::pathIsFile(const std::string& path) {
  struct stat st;
  if (::stat(path.c_str(), &st) < 0) return false;

  return S_ISREG(st.st_mode);
}

bool util::pathIsDir(const std::string& path) {
  struct stat st;
  if (::stat(path.c_str(), &st) < 0) return false;

  return S_ISDIR(st.st_mode);
}

bool util::endsWith(const std::string& s, const std::string& suffix) {
  size_t i = s.rfind(suffix);

  if (i == std::string::npos) return false;

  return (i == (s.length() - suffix.size()));
}

std::string util::expandEscapes(const std::string& s) {
  std::string d;

  for (auto c = s.begin(); c != s.end();) {
    if (*c != '\\') {
      d += *c++;
      continue;
    }

    ++c;

    if (c == s.end()) throw std::runtime_error("broken escape sequence");

    switch (*c++) {
      case '\\':
        d += '\\';
        break;

      case '"':
        d += '"';
        break;

      case 'n':
        d += '\n';
        break;

      case 'r':
        d += '\r';
        break;

      case 't':
        d += '\t';
        break;

      case 'u': {
        int32_t val;
        auto end = c + 4;
        c = atoi_n(c, end, 16, &val);

        if (c != end) throw std::runtime_error("cannot decode character");

        uint8_t tmp[4];
        int len = utf8proc_encode_char(val, tmp);

        if (!len) throw std::runtime_error("cannot encode unicode code point");

        d.append(reinterpret_cast<char*>(tmp), len);
        break;
      }

      case 'U': {
        int32_t val;
        auto end = c + 8;
        c = atoi_n(c, end, 16, &val);

        if (c != end) throw std::runtime_error("cannot decode character");

        uint8_t tmp[4];
        int len = utf8proc_encode_char(val, tmp);

        if (!len) throw std::runtime_error("cannot encode unicode code point");

        d.append(reinterpret_cast<char*>(tmp), len);
        break;
      }

      case 'x': {
        char val;
        auto end = c + 2;
        c = atoi_n(c, end, 16, &val);

        if (c != end) throw std::runtime_error("cannot decode character");

        d.append(&val, 1);
        break;
      }

      default:
        throw std::runtime_error("unknown escape sequence");
    }
  }

  return d;
}

std::string util::escapeUTF8(const std::string& s) {
  const unsigned char* p = (const unsigned char*)s.data();
  const unsigned char* e = p + s.size();

  std::string esc;

  while (p < e) {
    int32_t cp;

    ssize_t n = utf8proc_iterate((const uint8_t*)p, e - p, &cp);

    if (n < 0) {
      esc += "<illegal UTF8 sequence>";
      break;
    }

    if (cp == '\\')
      esc += "\\\\";

    else if (cp == '"')
      esc += "\\\"";

    else if (cp == '\n')
      esc += "\\n";

    else if (cp == '\r')
      esc += "\\r";

    else if (cp == '\t')
      esc += "\\t";

    else if (cp >= 65536)
      esc += fmt("\\U%08x", cp);

    else if (!isprint(static_cast<char>(cp)))
      esc += fmt("\\u%04x", cp);

    else
      esc += static_cast<char>(cp);

    p += n;
  }

  return esc;
}

std::string util::escapeBytes(const std::string& s) {
  const unsigned char* p = (const unsigned char*)s.data();
  const unsigned char* e = p + s.size();

  std::string esc;

  while (p < e) {
    if (*p == '\\')
      esc += "\\\\";

    else if (*p == '"')
      esc += "\\\"";

    else if (*p == '\n')
      esc += "\\n";

    else if (*p == '\r')
      esc += "\\r";

    else if (*p == '\t')
      esc += "\\t";

    else if (isprint(*p))
      esc += *p;

    else
      esc += fmt("\\x%02x", *p);

    ++p;
  }

  return esc;
}

std::string util::pathJoin(const std::string& p1, const std::string& p2) {
  if (startsWith(p2, "/")) return p2;

  std::string p = p1;

  while (endsWith(p, "/")) p = p.substr(0, p.size() - 1);

  return p + "/" + p2;
}

std::string util::dirname(const std::string& path) {
  size_t i = path.rfind("/");

  if (i == std::string::npos) return "";

  std::string dir = path.substr(0, i);

  std::string p = path;

  while (endsWith(p, "/")) p = p.substr(0, p.size() - 1);

  return p;
}

std::string util::basename(const std::string& path) {
  size_t i = path.rfind("/");

  if (i == std::string::npos) return path;

  return path.substr(i + 1, std::string::npos);
}

bool util::makeDir(const std::string& path) {
  return mkdir(path.c_str(), 0700) == 0;
}

std::string util::findInPaths(const std::string& file, const path_list& paths) {
  if (startsWith(file, "/")) return pathIsFile(file) ? file : "";

  for (auto d : paths) {
    std::string path = pathJoin(d, file);
    if (pathIsFile(path)) return path;
  }

  return "";
}

void util::abort_with_backtrace() {
  fputs("\n--- Aborting\n", stderr);
  void* callstack[128];
  int frames = backtrace(callstack, 128);
  backtrace_symbols_fd(callstack, frames, 2);
  abort();
}

double util::currentTime() {
  struct timeval tv;
  gettimeofday(&tv, 0);
  return static_cast<double>(tv.tv_sec) + static_cast<double>(tv.tv_usec) / 1e6;
}

std::string util::toIdentifier(const std::string& s, bool ensure_non_keyword) {
  static char const* const hex = "0123456789abcdef";

  if (s.empty()) return s;

  std::string normalized = s;

  normalized = ::util::strreplace(normalized, "::", "_");
  normalized = ::util::strreplace(normalized, "<", "_");
  normalized = ::util::strreplace(normalized, ">", "_");
  normalized = ::util::strreplace(normalized, ",", "_");
  normalized = ::util::strreplace(normalized, ".", "_");
  normalized = ::util::strreplace(normalized, " ", "_");
  normalized = ::util::strreplace(normalized, "-", "_");
  normalized = ::util::strreplace(normalized, "'", "_");
  normalized = ::util::strreplace(normalized, "\"", "_");
  normalized = ::util::strreplace(normalized, "__", "_");

  while (::util::endsWith(normalized, "_"))
    normalized = normalized.substr(0, normalized.size() - 1);

  std::string ns;

  for (auto c : normalized) {
    if (isalnum(c) || c == '_') {
      ns += c;
      continue;
    }

    ns += "x";
    ns += hex[c >> 4];
    ns += hex[c % 0x0f];
  }

  ns = ::util::strreplace(ns, "__", "_");

  if (isdigit(ns[0])) ns = "_" + ns;

  if (ensure_non_keyword) ns += "_";

  return ns;
}
