#ifndef __CAST_H__
#define __CAST_H__ (1)

#include <type_traits>

#include "common.h"
#include "numeric/long.h"
#include "object.h"

/**
 * PyObject * => C++ types via this template function.
 */
namespace cppbind {

/**
 * Convert to C++ types via this template function.
 */
template <typename T> inline T from(PyObject *obj) {
  cppbind_check_internal(obj != nullptr);
  /**
   * For a custom CPython class layout, the
   * head is a `PyObject`, the rest are the
   * non-static members of C++ type `T`.
   *
   * Therefore, `obj + 1` is the pointer to the non-static members, which can be
   * cast to `T *`.
   */
  return *reinterpret_cast<T *>(obj + 1);
}

/**
 * Template specialization for `PyObject *`.
 */
template <> inline PyObject *from<PyObject *>(PyObject *obj) { return obj; }

#define cppbind_from_on_type_mismatch                                          \
  throw std::invalid_argument("type mismatch when converting to C++ type")

/**
 * Template specialization for signed integer types.
 */
#define cppbind_from_specialize_signed_integer(cpp_ty)                         \
  template <> inline cpp_ty from<cpp_ty>(PyObject * obj) {                     \
    if (PyLong_Check(obj)) {                                                   \
      long long int value = PyLong_AsLongLong(obj);                            \
      return static_cast<cpp_ty>(value);                                       \
    }                                                                          \
    if (PyFloat_Check(obj)) {                                                  \
      double value = PyFloat_AsDouble(obj);                                    \
      return static_cast<cpp_ty>(value);                                       \
    }                                                                          \
    cppbind_from_on_type_mismatch;                                             \
  }

signed_integer_types(cppbind_from_specialize_signed_integer);
#undef cppbind_from_specialize_signed_integer

/**
 * Template specialization for unsigned integer types.
 */
#define cppbind_from_specialize_unsigned_integer(cpp_ty)                       \
  template <> inline cpp_ty from<cpp_ty>(PyObject * obj) {                     \
    if (PyLong_Check(obj)) {                                                   \
      unsigned long value = PyLong_AsUnsignedLong(obj);                        \
      return static_cast<cpp_ty>(value);                                       \
    }                                                                          \
    if (PyFloat_Check(obj)) {                                                  \
      double value = PyFloat_AsDouble(obj);                                    \
      return static_cast<cpp_ty>(value);                                       \
    }                                                                          \
    cppbind_from_on_type_mismatch;                                             \
  }

unsigned_integer_types(cppbind_from_specialize_unsigned_integer);
#undef cppbind_from_specialize_unsigned_integer

/**
 * Template specialization for floating point types.
 */
#define cppbind_from_specialize_fp(cpp_ty)                                     \
  template <> inline cpp_ty from<cpp_ty>(PyObject * obj) {                     \
    if (PyFloat_Check(obj)) {                                                  \
      double value = PyFloat_AsDouble(obj);                                    \
      return static_cast<cpp_ty>(value);                                       \
    }                                                                          \
    if (PyLong_Check(obj)) {                                                   \
      long long int value = PyLong_AsLongLong(obj);                            \
      return static_cast<cpp_ty>(value);                                       \
    }                                                                          \
    cppbind_from_on_type_mismatch;                                             \
  }

floating_point_types(cppbind_from_specialize_fp);
#undef cppbind_from_specialize_fp

} /* namespace cppbind */

namespace cppbind {

/**
 * Convert C++ integer types to Python objects.
 */
template <typename _Tp,
          std::__enable_if_t<!is_integer_ty<_Tp>() && !is_fp_ty<_Tp>() &&
                                 !is_object_ty<_Tp>() && !is_pair_ty<_Tp>(),
                             bool> = true>
inline Object into(_Tp value) {
  /* For internal testing, trigger an assertion failure. */
  cppbind_check_internal(0 &&
                         "unsupported type for conversion to Python object");
  /* For external use, set TypeError, and return Object with nullptr. */
  PyErr_SetString(PyExc_TypeError,
                  "unsupported type for conversion to Python object");
  return Object{nullptr};
}

template <typename _Tp, std::__enable_if_t<is_object_ty<_Tp>(), bool> = true>
inline Object into(_Tp value) {
  return value.object();
}

/**
 * Specialization for integer types. It will convert the integer to
 * a Python long object.
 */
template <typename _Tp, std::__enable_if_t<is_integer_ty<_Tp>(), bool> = true>
inline Object into(_Tp value) {
  __static_assert(is_integer_ty<_Tp>());
  return Long(value).object();
}

/**
 * Specialization for floating point types. It will convert the floating point
 * number to a Python float object.
 */
template <typename _Tp, std::__enable_if_t<is_fp_ty<_Tp>(), bool> = true>
inline Object into(_Tp value) {
  __static_assert(is_fp_ty<_Tp>());
  return Float(value).object();
}

/**
 * Build {@link cppbind::Object} from <pre>PyObject *</pre>.
 */
template <> inline Object into<PyObject *>(PyObject *pt) { return Object{pt}; }

/**
 * Convert C++ pair to tuple of size 2.
 */
template <typename _Tp, std::__enable_if_t<is_pair_ty<_Tp>(), bool> = true>
inline Object into(_Tp value) {
  Object first_obj = into<decltype(value.first)>(value.first);
  Object second_obj = into<decltype(value.second)>(value.second);
  return Tuple(first_obj, second_obj).object();
}

} /* namespace cppbind */

#endif /* __CAST_H__ */
