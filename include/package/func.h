#ifndef __PACKAGE_FUNC_H__
#define __PACKAGE_FUNC_H__ (1)

#include <cstddef>
#include <cstring>

#include <object.h>
#include <package/invoke.h>

namespace cppbind {

template <typename _Fn> constexpr int CFunction_flags(void) { return 0; }

template <> constexpr int CFunction_flags<PyObject *(*)(void)>(void) {
  return METH_NOARGS;
}
template <> constexpr int CFunction_flags<PyCFunction>(void) {
  return METH_VARARGS;
}
template <> constexpr int CFunction_flags<PyCArgsFunction>(void) {
  return METH_VARARGS;
}
template <> constexpr int CFunction_flags<PyObject *(*)(PyObject *)>(void) {
  return METH_NOARGS;
}
template <> constexpr int CFunction_flags<PyCMethod>(void) {
  return METH_METHOD | METH_VARARGS | METH_KEYWORDS;
}

template <> constexpr int CFunction_flags<PyCFunctionWithKeywords>(void) {
  return METH_KEYWORDS | METH_VARARGS;
}

template <> constexpr int CFunction_flags<PyCFunctionVec>(void) {
  return METH_FASTCALL;
}

template <> constexpr int CFunction_flags<PyCFunctionVecWithKeywords>(void) {
  return METH_FASTCALL | METH_KEYWORDS;
}

#pragma GCC diagnostic ignored "-Wpedantic"
#define gen_PyMethodDef(fn)                                                    \
  (PyMethodDef) {                                                              \
    #fn, (PyCFunction)fn, cppbind::CFunction_flags<decltype(&fn)>(), nullptr   \
  }
#define gen_PyMethodDef_doc(fn, doc)                                           \
  (PyMethodDef) {                                                              \
    #fn, (PyCFunction)fn, cppbind::CFunction_flags<decltype(&fn)>(), doc       \
  }

template <typename Callable> struct MethodWrapper {

  MethodWrapper(PyObject *self, Callable callable)
      : self(self), callable(callable) {
    Py_INCREF(self);
  }
  ~MethodWrapper() { Py_DECREF(self); }

  PyObject *operator()(PyObject *self, PyObject *args, PyObject *kwargs) const {
    auto *objref = this->self;
    return ((ternaryfunc)callable)(objref, args, kwargs);
  }

  static void __del__(PyObject *self) {
    MethodWrapper *wrapper = reinterpret_cast<MethodWrapper *>(self);
    wrapper->~MethodWrapper();
  }

  static PyObject *call(PyObject *self, PyObject *args, PyObject *kwargs) {
    MethodWrapper *wrapper = reinterpret_cast<MethodWrapper *>(self);
    return (*wrapper)(self, args, kwargs);
  }

  static void initType(PyTypeObject *type) {
    auto offset = offsetof(PyTypeObject, tp_name);
    memset(&(type->tp_name), 0, sizeof(*type) - offset);
    type->tp_name = "_ffi.MethodWrapper";
    type->tp_dealloc = __del__;
    type->tp_flags |= Py_TPFLAGS_DEFAULT;
    type->tp_call = (ternaryfunc)call;
    type->tp_basicsize = sizeof(MethodWrapper);
  }

  static PyObject *createInstance(PyObject *objref, Callable callable) {
    PyTypeObject *type =
        (PyTypeObject *)_PyObject_New((PyTypeObject *)&PyType_Type);
    initType(type);

    PyObject *obj = _PyObject_New((PyTypeObject *)type);
    new (obj) MethodWrapper(objref, callable);
    return obj;
  }

private:
  PyObject pyobj;
  PyObject *self;
  Callable callable;
};

} /* namespace cppbind */

#endif /* __PACKAGE_FUNC_H__ */
