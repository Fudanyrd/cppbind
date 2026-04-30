#include <cppbind.h>

extern "C" PyObject *myadd(PyObject *self, PyObject *args) {
  __static_assert(cppbind::CFunction_flags<decltype(&myadd)>());
  assert(args && PyTuple_Check(args));
  cppbind::Tuple tuple = cppbind::Tuple::from_args(args);
  if (tuple.size() != 2) {
    PyErr_SetString(PyExc_TypeError, "Two arguments expected");
    return nullptr;
  }
  cppbind::Long a(tuple[0].ptr), b(tuple[1].ptr);
  a += b;
  return a.object().unwrap();
}

gen_modinit_fn_from_fns(
    /* name */ _capi, nullptr, nullptr, nullptr, nullptr,
    gen_PyMethodDef_doc(myadd, ":returns: sum of two integers"));
