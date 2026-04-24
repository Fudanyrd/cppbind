#ifndef __PACKAGE_INVOKE_H__
#define __PACKAGE_INVOKE_H__ (1)

#include <methodobject.h>

#include <common.h>
#include <container/list.h>
#include <numeric/long.h>

extern "C" {
typedef PyObject (*PyCArgsFunction)(PyObject *self, PyObject *const *args,
                                    Py_ssize_t nargs);

typedef PyObject *(*PyCArgsFunctionWithKeywords)(PyObject *self,
                                                 PyObject *const *args,
                                                 Py_ssize_t nargs,
                                                 PyObject *kwargs);

/* vector call. */
typedef PyObject *(*PyCFunctionVec)(PyObject *, PyObject *const *, Py_ssize_t);

typedef PyObject *(*PyCFunctionVecWithKeywords)(PyObject *, PyObject *const *,
                                                Py_ssize_t, PyObject *);
}

namespace cppbind {

struct Dict; /* To be implemented */

/* Invoke `def fn(...)` */
template <typename... _Args> Object invoke(PyCFunction fn, _Args &...args) {
  List largs{args...};
  return Object(fn(nullptr, largs.object().ptr));
}

template <typename _Tp> PyObject *get_pyobject_ptr(_Tp &obj) {
  return obj.object().ptr;
}

/* Invoke `def fn(..., *args)` */
template <typename... _Args>
Object invoke(PyCArgsFunction fn, const List &args, _Args &...rargs) {
  PyObject *arr[] = {get_pyobject_ptr(rargs)...};
  return Object(fn(nullptr, arr, sizeof...(rargs)));
}

/* Invoke `def fn(..., **kwargs)` */
template <typename... _Args> Object invoke(const Dict &kwargs, _Args...);

/* Invoke `def fn(..., *args, **kwargs)` */
template <typename... _Args>
Object invoke(const List &args, const Dict &kwargs, _Args...);

} /* namespace cppbind */

#endif /* __PACKAGE_INVOKE_H__ */
