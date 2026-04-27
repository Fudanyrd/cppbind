#ifndef __PACKAGE_INVOKE_H__
#define __PACKAGE_INVOKE_H__ (1)

#include <methodobject.h>

#include <common.h>
#include <container/dict.h>
#include <container/list.h>
#include <container/tuple.h>
#include <numeric/long.h>

extern "C" {
typedef PyObject *(*PyCArgsFunctionWithKeywords)(PyObject *self,
                                                 PyObject *const *args,
                                                 Py_ssize_t nargs,
                                                 PyObject *kwargs);

/* vector call. */
typedef PyObject *(*PyCFunctionVec)(PyObject *self, PyObject *const *args,
                                    Py_ssize_t nargs);

typedef PyObject *(*PyCFunctionVecWithKeywords)(PyObject *self,
                                                PyObject *const *args,
                                                Py_ssize_t nargs,
                                                PyObject *kwnames);
}

namespace cppbind {

inline Object invoke(PyObject *callable) {
  return Object{PyObject_CallNoArgs(callable)};
}

inline Object invoke(PyObject *callable, Object arg) {
  return Object{PyObject_CallOneArg(callable, arg.unwrap())};
}

template <typename... Args>
inline Object invoke(PyObject *callable, Args... args) {
  Tuple tuple(args...);
  PyObject *pyargs = tuple.object().ptr;
  return Object{PyObject_Call(callable, pyargs, nullptr)};
}

} /* namespace cppbind */

#endif /* __PACKAGE_INVOKE_H__ */
