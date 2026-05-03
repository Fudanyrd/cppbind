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

/**
 * A function pointer type for module initialization functions.
 * It should return `0` on succcess; set python `ImportError`
 * and return non-zero on failure.
 */
typedef int (*RestInitFn)(void);

template <typename FnPtr> constexpr bool is_restini_fn(void) { return false; }
template <> constexpr bool is_restini_fn<RestInitFn>(void) { return true; }
template <> constexpr bool is_restini_fn<decltype(nullptr)>(void) {
  return true;
}

/*
 * Generates a `PyMODINIT_FUNC` that initializes a python module
 * named `name`. Each arguments in `...` should be an instance
 * of `PyMethodDef`.
 */
#define gen_modinit_fn_from_fns(name, rest_init_fn, traverse_fn, free_fn,      \
                                clear_fn, ...)                                 \
  PyMODINIT_FUNC CONCAT(PyInit_, name)(void) {                                 \
    static PyMethodDef defs[] = {__VA_ARGS__, {nullptr, nullptr, 0, nullptr}}; \
    static PyModuleDef moddef = {                                              \
        PyModuleDef_HEAD_INIT, #name,      "",        0, defs, 0,              \
        (traverse_fn),         (clear_fn), (free_fn),                          \
    };                                                                         \
    if (rest_init_fn != nullptr) {                                             \
      __static_assert(::cppbind::is_restini_fn<decltype(rest_init_fn)>());     \
      if ((*(::cppbind::RestInitFn)(rest_init_fn))()) {                        \
        return nullptr; /* ImportError */                                      \
      }                                                                        \
    }                                                                          \
    PyObject *mod = PyModule_Create(&moddef);                                  \
    return mod;                                                                \
  }

} /* namespace cppbind */

#endif /* __PACKAGE_MOD_H__ */
