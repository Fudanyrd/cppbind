/**
 * CInt implementation.
 */
#include "ffi.h"

/**
 * <h3>bugprone-easily-swappable-parameters</h3>
 * Make the `type_init_mapping` macro work.
 *
 * <h3>readability-identifier-naming</h3>
 * `CInt_New` is a CPython function.
 *
 * <h3>readability-identifier-length</h3>
 * For macros like `type_init_integer_ops`, using longer
 * identifiers will not improve readability.
 */
/**
 * NOLINTBEGIN(bugprone-easily-swappable-parameters,
 *             readability-identifier-naming,
 *             readability-identifier-length)
 */

type_static_members_declare(example::CInt);

#define CInt_binary_operator(Operator)                                         \
  example::CInt operator Operator(const example::CInt &a,                      \
                                  const example::CInt &b) {                    \
    example::CInt ret;                                                         \
    ret.num = a.num Operator b.num;                                            \
    return ret;                                                                \
  }

type_integer_binary_ops(CInt_binary_operator);
#undef CInt_binary_operator

#define CInt_unary_operator(Operator)                                          \
  example::CInt operator Operator(const example::CInt &a) {                    \
    example::CInt ret;                                                         \
    ret.num = Operator(a.num);                                                 \
    return ret;                                                                \
  }
type_integer_unary_ops(CInt_unary_operator);
#undef CInt_unary_operator

using cppbind::Long;
using example::CInt;

static PyObject *CInt_Init(PyObject *self) {
  return ::cppbind::Type<CInt>::New(self);
}

::cppbind::Object CInt::forward_or_convert(const ::cppbind::Object &arg) {
  if (arg.ptr == nullptr) {
    PyErr_SetString(PyExc_ValueError, "argument cannot be null");
    return ::cppbind::Object(nullptr);
  }
  if (PyObject_TypeCheck(arg.ptr, cppbind::Type<CInt>::instance)) {
    return arg;
  } else if (PyLong_Check(arg.ptr)) {
    PyObject *ret = _PyObject_New(cppbind::Type<CInt>::instance);
    if (ret != nullptr) {
      new (ret) CInt();
      reinterpret_cast<CInt *>(ret)->num = PyLong_AsLong(arg.ptr);
    }
    return ::cppbind::Object(ret);
  }
  return ::cppbind::Object(nullptr);
}

extern "C" {

PyObject *CInt_New(PyObject *self) { return CInt_Init(self); }

PyObject *CInt_FromInt(PyObject *self, PyObject *arg) {
  auto *ret = CInt_Init(self);
  reinterpret_cast<CInt *>(ret)->num = (long)Long(arg);
  return ret;
}

} /* extern "C" */

/**
 * NOLINTEND(bugprone-easily-swappable-parameters,
 *           readability-identifier-naming,
 *           readability-identifier-length)
 */