#ifndef __PACKAGE_FUNC_H__
#define __PACKAGE_FUNC_H__ (1)

#include <object.h>
#include <package/invoke.h>

namespace cppbind {

template <typename _Fn> constexpr int CFunction_flags(void) { return 0; }

template <> constexpr int CFunction_flags<PyObject (*)(void)>(void) {
  return METH_NOARGS;
}
template <> constexpr int CFunction_flags<PyCFunction>(void) {
  return METH_VARARGS;
}
template <> constexpr int CFunction_flags<PyCArgsFunction>(void) {
  return METH_VARARGS;
}

template <> constexpr int CFunction_flags<PyCFunctionWithKeywords>(void) {
  return METH_KEYWORDS;
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

} /* namespace cppbind */

#endif /* __PACKAGE_FUNC_H__ */
