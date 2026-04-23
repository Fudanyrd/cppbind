#include <cassert>

#include <cppbind.h>

using cppbind::List;
using cppbind::Tuple;
using cppbind::Long;
using cppbind::Object;

extern "C" PyObject *myadd(PyObject *self, PyObject *args) {
  __static_assert(cppbind::CFunction_flags<decltype(&myadd)>());
  assert(PyTuple_Check(args));
  Tuple list{Object{args}};
  assert(list.size() == 2);
  Long a(list[0].ptr), b(list[1].ptr);
  a += b;
  return a.object().unwrap();
}

extern "C" PyObject *mysum(PyObject *self, PyObject *args) {
  __static_assert(cppbind::CFunction_flags<decltype(&mysum)>());
  assert(PyTuple_Check(args));
  Tuple list{Object{args}};
  Long sum(0);
  for (Py_ssize_t i = 0; i < list.size(); i++) {
    Long a(list[i].ptr);
    sum += a;
  }
  return sum.object().unwrap();
}


gen_modinit_fn_from_fns(/* name */ myabc, nullptr, nullptr, nullptr,
                        gen_PyMethodDef_doc(myadd,
                                            ":returns: sum of two integers"),
                        gen_PyMethodDef_doc(mysum,
                                            ":returns: sum of all integers"));
