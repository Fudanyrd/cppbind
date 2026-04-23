#include <cassert>

#include <cppbind.h>

using cppbind::List;
using cppbind::Long;
using cppbind::Object;

extern "C" PyObject *myadd(PyObject *self, PyObject *args) {
  __static_assert(cppbind::CFunction_flags<decltype(&myadd)>());
  assert(PyList_Check(args));
  List list{Object(args)};
  assert(list.size() == 2);
  Long a(list[0].ptr), b(list[1].ptr);
  a += b;
  return a.object().unwrap();
}

gen_modinit_fn_from_fns(/* name */ myabc, nullptr, nullptr, nullptr,
                        gen_PyMethodDef_doc(myadd,
                                            ":returns: sum of two integers"))
