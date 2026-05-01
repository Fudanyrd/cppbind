/**
 * CInt implementation.
 */
#include "ffi.h"

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
