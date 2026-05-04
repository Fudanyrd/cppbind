#ifndef __PACKAGE_FUNC_H__
#define __PACKAGE_FUNC_H__ (1)

#include <cstddef>
#include <cstdlib>
#include <cstring>

#include <cast.h>
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

// #pragma GCC diagnostic ignored "-Wpedantic"
#define gen_PyMethodDef(fn)                                                    \
  (PyMethodDef) {                                                              \
    #fn, (PyCFunction)fn, ::cppbind::CFunction_flags<decltype(&fn)>(), nullptr \
  }
#define gen_PyMethodDef_doc(fn, doc)                                           \
  (PyMethodDef) {                                                              \
    #fn, (PyCFunction)fn, ::cppbind::CFunction_flags<decltype(&fn)>(), doc     \
  }

/**
 * A wrapper for method function. It is a callable object that
 * wraps the actual method, and can be called by python.
 *
 * Example of this:
 * <blockquote><pre>
 * struct CBytes {
 *   PyObject pyobj; // required for all python classes
 *   const char *buf;
 *   // cast *this to Python3 bytes.
 *   PyObject *bytes() const;
 * };
 *
 * // the most common method wrapper (not accessible in Python):
 * PyObject *CBytes_bytes (PyObject *self,
 *              PyObject *args, // Tuple, Unused
 *              PyObject *kwargs) { // Dict, Unused
 *   // cast to CBytes.
 *   CBytes *cppobj = reinterpret_cast<CBytes *>(self);
 *   // call the actual method.
 *   return cppobj->bytes();
 * }
 *
 * // a CBytes Instance:
 * CBytes mybytes;
 *
 * // create a Method wrapper (accessible in python):
 * MethodWrapper *methodObject =
 * MethodWrapper::create_instance(&mybytes, &CBytes_bytes, "_mypackage");
 * </pre></blockquote>
 */
template <typename Callable> struct MethodWrapper {

  /**
   * Create an {@link MethodWrapper}.
   *
   * @param self: the `PyObject` owning the method.
   * @param callable: a function pointer whose first
   * argument is `self`.
   */
  MethodWrapper(PyObject *self, Callable callable)
      : self(self), callable(callable) {
    Py_INCREF(self);
  }
  ~MethodWrapper() { Py_DECREF(self); }

  /**
   * Invoke the callable with args and kwargs.
   */
  PyObject *operator()(PyObject *self, PyObject *args, PyObject *kwargs) const {
    auto *objref = this->self;
    return ((ternaryfunc)callable)(objref, args, kwargs);
  }

  /**
   * Explicitly invoke the destructor of {@link MethodWrapper}.
   */
  static void __del__(PyObject *self) {
    MethodWrapper *wrapper = reinterpret_cast<MethodWrapper *>(self);
    wrapper->~MethodWrapper();
  }

  /**
   * Invoke the callable with args and kwargs.
   */
  static PyObject *call(PyObject *self, PyObject *args, PyObject *kwargs) {
    MethodWrapper *wrapper = reinterpret_cast<MethodWrapper *>(self);
    return (*wrapper)(self, args, kwargs);
  }

  /**
   * Constructs a `PyTypeObject` in `type`.
   *
   * @param type: pointer to the `PyTypeObject` to be initialized.
   * @param name: the name of the type, in format "<package>.<name>".
   */
  static void initType(PyTypeObject *type, const char *name) {
    auto offset = offsetof(PyTypeObject, tp_name);
    memset(&(type->tp_name), 0, sizeof(*type) - offset);
    type->tp_name = name;
    type->tp_dealloc = __del__;
    type->tp_flags = Py_TPFLAGS_DEFAULT;
    type->tp_call = (ternaryfunc)call;
    type->tp_basicsize = sizeof(MethodWrapper);
  }

  /**
   * @param objref: the `self` argument to the `callable` method.
   * @param callable: the actual method wrapper, as a function pointer.
   *
   * @return an instance of {@link MethodWrapper}, cast as `PyObject`.
   */
  static PyObject *create_instance(PyObject *objref, Callable callable);

private:
  PyObject pyobj;
  PyObject *self;
  Callable callable;

public:
  /**
   * Python type for {@link MethodWrapper}.
   */
  static PyTypeObject *method_type;

  /**
   * Initialize {@link MethodWrapper::method_type}.
   *
   * @param name: string literal of format "<package>.<class>"
   */
  static void init_method_type(const char *name) {
    if (method_type == nullptr) {
      method_type = (PyTypeObject *)_PyObject_New((PyTypeObject *)&PyType_Type);
      initType(method_type, name);
    }
  }

  /**
   * Free {@link MethodWrapper::method_type}.
   * This will set {@link MethodWrapper::method_type} to nullptr,
   * so only use this for module unload.
   */
  static void fini_method_type(void) {
    if (method_type == nullptr) {
      return;
    }
    auto count = Py_REFCNT(method_type);
    for (decltype(count) i = 0; i < count; i++) {
      Py_DECREF(method_type);
    }
    method_type = nullptr;
  }

/**
 * @param package_name: the name of the package (string literal).
 * @param callable_type: the type of the method wrapper, e.g., `PyCFunction`.
 */
#define MethodWrapper_init(package_name, callable_type)                        \
  do {                                                                         \
    ::cppbind::MethodWrapper<callable_type>::init_method_type(                 \
        package_name ".MethodWrapper");                                        \
    if (::cppbind::MethodWrapper<callable_type>::method_type == nullptr) {     \
      PyErr_SetString(PyExc_RuntimeError,                                      \
                      "failed to create method wrapper type");                 \
      return (1);                                                              \
    }                                                                          \
  } while (0)

#define method_wrapper_static_members(callable_type)                           \
  template <>                                                                  \
  PyTypeObject * ::cppbind::MethodWrapper<callable_type>::method_type =        \
      nullptr
};

template <typename Callable>
PyObject *MethodWrapper<Callable>::create_instance(PyObject *objref,
                                                   Callable callable) {
  PyTypeObject *type =
      MethodWrapper<Callable>::method_type; /* initialized in package init
                                               function */

  PyObject *obj = _PyObject_New((PyTypeObject *)type);
  new (obj) MethodWrapper<Callable>(objref, callable);
  return obj;
}

} /* namespace cppbind */

#endif /* __PACKAGE_FUNC_H__ */
