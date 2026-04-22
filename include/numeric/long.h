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

  Long(PyObject *obj) : obj(PyLong_Check(obj) ? obj : PyNumber_Long(obj)) {}
  Long(const Object &obj)
      : obj(PyLong_Check(obj.ptr) ? obj.ptr : PyNumber_Long(obj.ptr)) {}
  Long(const Long &other) : obj(other.obj) {}
  ~Long() = default;

#define build_from(cpp_ty, converter)                                          \
  Long(cpp_ty val) : obj(CONCAT(PyLong_From, converter)(val)) {}

  build_from(long, Long) build_from(unsigned long, UnsignedLong)
      build_from(long long, LongLong)
          build_from(unsigned long long, UnsignedLongLong)

#undef build_from
#define build_from_signed_int(cpp_sint_ty)                                     \
  Long(cpp_sint_ty val)                                                        \
      : obj(PyLong_FromLongLong(static_cast<long long>(val))) {}
#define build_from_unsigned_int(cpp_uint_ty)                                   \
  Long(cpp_uint_ty val)                                                        \
      : obj(PyLong_FromUnsignedLongLong(                                       \
            static_cast<unsigned long long>(val))) {}

#define FOREACH(X) X(signed char) X(short) X(int)
              FOREACH(build_from_signed_int)
#undef FOREACH
#define FOREACH(X) X(unsigned char) X(unsigned short) X(unsigned int)
                  FOREACH(build_from_unsigned_int)

#undef FOREACH
#undef build_from_signed_int
#undef build_from_unsigned_int

#define convert_to(cpp_ty, converter)                                          \
  operator cpp_ty() const { return CONCAT(PyLong_As, converter)(obj.ptr); }

                      convert_to(long, Long) convert_to(long long, LongLong)
                          convert_to(unsigned long, UnsignedLong)
                              convert_to(unsigned long long, UnsignedLongLong)

#undef convert_to

      /* Get the underlying Object */
      Object &object() {
    return obj;
  }
  const Object &object() const { return obj; }

  Long &operator+=(const Long &other) {
    obj.ptr = PyNumber_InPlaceAdd(obj.ptr, other.obj.ptr);
    return *this;
  }
};

using Integer = Long;

} /* namespace cppbind */

#undef __TO_STR
#undef STR
#undef __CONCAT
#undef CONCAT
#endif /* __NUMERIC_LONG_H__ */
