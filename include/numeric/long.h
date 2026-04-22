#ifndef __NUMERIC_LONG_H__
#define __NUMERIC_LONG_H__ (1)

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

#include "common.h"
#include "object.h"

#define __TO_STR(x) #x
#define STR(x) __TO_STR(x)
#if !defined __CONCAT
#define __CONCAT(a, b) a##b
#endif /* !defined __CONCAT */
#define CONCAT(a, b) __CONCAT(a, b)

namespace cppbind {

struct Long {
  Object obj;

#define build_from(cpp_ty, converter)                                          \
  Long(cpp_ty val) : obj(CONCAT(PyLong_From, converter)(val)) {}

  build_from(long, Long) build_from(unsigned long, UnsignedLong)
      build_from(long long, LongLong)
          build_from(unsigned long long, UnsignedLongLong)

#undef build_from
#define convert_to(cpp_ty, converter)                                          \
  operator cpp_ty() const { return CONCAT(PyLong_As, converter)(obj.ptr); }

              convert_to(long, Long)

#undef convert_to

                  Object &object() {
    return obj;
  }
  const Object &object() const { return obj; }
};

using Integer = Long;

} /* namespace cppbind */

#undef __TO_STR
#undef STR
#undef __CONCAT
#undef CONCAT
#endif /* __NUMERIC_LONG_H__ */
