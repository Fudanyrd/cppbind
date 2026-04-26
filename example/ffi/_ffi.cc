#include <cstdio>

#include <cppbind.h>

using cppbind::Long;
using cppbind::MethodWrapper;
using cppbind::Object;

struct CInt {
  CInt() : num(0) {}

  int getvalue(void) const { return num; }

private:
  PyObject pyobj;

public:
  int num;
};

static inline CInt *from_pyobj(PyObject *obj) {
  return reinterpret_cast<CInt *>(obj);
}

extern "C" void CInt_destructor(PyObject *self) {
  CInt *cint = from_pyobj(self);
  /* Explicitly calls the destructor of CInt. */
  cint->~CInt();
}

extern "C" PyObject *CInt_getvalue(PyObject *self) {
  return Long(from_pyobj(self)->getvalue()).object().unwrap();
}

extern "C" PyObject *CInt_getattr(PyObject *self, char *name) {
  if (strcmp(name, "num") == 0) {
    return CInt_getvalue(self);
  } else if (strcmp(name, "getvalue") == 0) {
    return MethodWrapper<decltype(&CInt_getvalue)>::createInstance(
        self, CInt_getvalue);
  }
  PyErr_SetString(PyExc_AttributeError, "attribute not found");
  return nullptr;
}

extern "C" PyObject *CInt_str(PyObject *self) {
  char buf[16];
  sprintf(buf, "0x%x", from_pyobj(self)->getvalue());
  return cppbind::Str(buf).object().unwrap();
}

static PyMethodDef CInt_methods[] = {
    (PyMethodDef){"getvalue", (PyCFunction)CInt_getvalue,
                  METH_METHOD | METH_NOARGS, nullptr},
    (PyMethodDef){nullptr, nullptr, 0, nullptr} /* Sentinel */
};

extern "C" PyObject *CInt_Init(PyObject *self) {
  PyTypeObject *CInt_type =
      (PyTypeObject *)_PyObject_New((PyTypeObject *)&PyType_Type);
  auto offset = offsetof(PyTypeObject, tp_name);
  memset(&(CInt_type->tp_name), 0, sizeof(*CInt_type) - offset);
  CInt_type->tp_name = "_ffi.CInt";
  CInt_type->tp_basicsize = sizeof(CInt);
  CInt_type->tp_dealloc = CInt_destructor;
  CInt_type->tp_getattr = CInt_getattr;
  CInt_type->tp_flags = Py_TPFLAGS_DEFAULT;
  CInt_type->tp_methods = CInt_methods;
  CInt_type->tp_repr = CInt_str;
  PyObject *ret = _PyObject_New((PyTypeObject *)CInt_type);

  new (from_pyobj(ret)) CInt();
  return ret;
}

extern "C" PyObject *CInt_New(PyObject *self) { return CInt_Init(self); }
extern "C" PyObject *CInt_FromInt(PyObject *self, PyObject *arg) {
  auto *ret = CInt_Init(self);
  from_pyobj(ret)->num = (long)Long(arg);
  return ret;
}

gen_modinit_fn_from_fns(
    _ffi, nullptr, nullptr, nullptr,
    gen_PyMethodDef_doc(CInt_New, ":returns: a new CInt object"),
    (PyMethodDef){"CInt_FromInt", (PyCFunction)CInt_FromInt, METH_O,
                  "Creates a new CInt object from an integer."});
