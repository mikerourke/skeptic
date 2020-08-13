#ifndef SKEPTIC_COMMON_H
#define SKEPTIC_COMMON_H

#include <cassert>
#include <climits>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <string>
#include "twain.h"

/** Solution for unused arguments */
#define UNUSEDARG(x) (void)x

#define kPATH_SEPARATOR '/'

inline int Printf(char *targetString, size_t n, const char *format, ...) {
  va_list vaList;
  va_start(vaList, format);
  int result = vsnprintf(targetString, n, format, vaList);
  va_end(vaList);

  return result;
}

#endif //SKEPTIC_COMMON_H
