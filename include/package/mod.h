#ifndef __PACKAGE_MOD_H__
#define __PACKAGE_MOD_H__ (1)

#include <common.h>

namespace cppbind {

struct Module;

#define gen_modinit_fn(name, method_def_array, traverse_fn, free_fn, clear_fn) \
  PyMODINIT_FUNC CONCAT(PyInit_, name)(void) {                                 \
    PyObject *mod;                                                             \
    PyModuleDef moddef = {                                                     \
        PyModuleDef_HEAD_INIT, #name,     "",         0, method_def_array, 0,  \
        (traverse_fn),         (free_fn), (clear_fn),                          \
    };                                                                         \
    mod = PyModule_Create(&moddef);                                            \
    return mod;                                                                \
  }

#define sizeof_array(a) ((sizeof(a)) / sizeof(a[0]))

#define gen_modinit_fn_from_fns(name, traverse_fn, free_fn, clear_fn, ...)     \
  PyMODINIT_FUNC CONCAT(PyInit_, name)(void) {                                 \
    static PyMethodDef defs[] = {__VA_ARGS__, {nullptr, nullptr, 0, nullptr}}; \
    static PyModuleDef moddef = {                                              \
        PyModuleDef_HEAD_INIT, #name,     "",         0, defs, 0,              \
        (traverse_fn),         (free_fn), (clear_fn),                          \
    };                                                                         \
    PyObject *mod = PyModule_Create(&moddef);                                  \
    return mod;                                                                \
  }

} /* namespace cppbind */

#endif /* __PACKAGE_MOD_H__ */
