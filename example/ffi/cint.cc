/**
 * CInt implementation.
 */
#include "ffi.h"

using cppbind::Long;
using example::CInt;

static PyObject *CInt_Init(PyObject *self) {
  return ::cppbind::Type<CInt>::New(self);
}

extern "C" {

PyObject *CInt_New(PyObject *self) { return CInt_Init(self); }

PyObject *CInt_FromInt(PyObject *self, PyObject *arg) {
  auto *ret = CInt_Init(self);
  reinterpret_cast<CInt *>(ret)->num = (long)Long(arg);
  return ret;
}

} /* extern "C" */
