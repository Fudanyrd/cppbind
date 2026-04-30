#ifndef __NUMERIC_LONG_H__
#define __NUMERIC_LONG_H__ (1)

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

#include "common.h"
#include "numeric/macro.h"
#include "object.h"

namespace cppbind {

/**
 * Python integer type, which supports numeric operations (`+`, `+=`, etc.)
 *
 * <h2>Initialization</h2>
 * {@link Long} is constructible from all C++ integer types. All signed
 * integer types are converted to `long long int`, and all unsigned integer
 * types are converted to `unsigned long long` before actual construction.
 *
 * <h2>Convertion to C++ integers</h2>
 * Precision loss is prevented by:
 * <ul>
 *   <li>Before convertion to a signed integer, it is first converted to long
 * long.</li> <li>Before convertion to an unsigned integer, it is first
 * converted to unsigned long long.</li>
 * </ul>
 */
struct Long {
private:
  Object obj;

public:
  /**
   * Construct from a python integer.
   */
  Long(PyObject *obj) : obj(PyLong_Check(obj) ? obj : PyNumber_Long(obj)) {
    if (this->obj.ptr == nullptr) {
      /* possibly PyNumber_Long failure. */
      cppbind_check_internal(0 && "failed to convert to long");
      /* if assertion is disabled, construct from a default value. */
      PyErr_Clear();
      this->obj.ptr = PyLong_FromLongLong(0ll);
    }
  }

  /**
   * Construct from an {@link Object} guarding a python integer.
   */
  Long(const Object &obj)
      : obj(PyLong_Check(obj.ptr) ? obj.ptr : PyNumber_Long(obj.ptr)) {}

  /**
   * Copy constructor.
   */
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

                      convert_to(long long, LongLong) convert_to(long, Long)
                          convert_to(unsigned long long, UnsignedLongLong)
                              convert_to(unsigned long, UnsignedLong)

#undef convert_to

      /**
       * @return the reference to current object.
       */
      Object &object() {
    return obj;
  }

  /**
   * @return the reference to current object.
   */
  const Object &object() const { return obj; }

  /**
   * Equivalent to python code `self //= other`.
   */
  Long &operator/=(const Long &other) {
    PyObject *result = PyNumber_FloorDivide(obj.ptr, other.obj.ptr);
    obj = Object(result);
    return *this;
  }

  gen_inplace_op_impl(Long, +=, inplace_num_Add);
  gen_inplace_op_impl(Long, -=, inplace_num_Subtract);
  gen_inplace_op_impl(Long, *=, inplace_num_Multiply);
  gen_inplace_op_impl(Long, %=, inplace_num_Remainder);
  gen_inplace_op_impl(Long, <<=, inplace_num_Lshift);
  gen_inplace_op_impl(Long, >>=, inplace_num_Rshift);
  gen_inplace_op_impl(Long, &=, inplace_num_And);
  gen_inplace_op_impl(Long, ^=, inplace_num_Xor);
  gen_inplace_op_impl(Long, |=, inplace_num_Or);
};

/**
 * Python integer type; an alias for {@link Long}.
 */
using Integer = Long;

} /* namespace cppbind */

#endif /* __NUMERIC_LONG_H__ */
