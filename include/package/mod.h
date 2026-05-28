#ifndef __PACKAGE_MOD_H__
#define __PACKAGE_MOD_H__ (1)

#include "common.h"

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

/**
 * A function pointer type for module initialization functions that
 * need to access the module object. It should return `0` on succcess; set
 * python `ImportError` and return non-zero on failure.
 */
typedef int (*RestInitFnWithMod)(Module &mod);

/**
 * Invoke `RestInitFn` or `RestInitFnWithMod` with the given module
 * object.
 */
template <typename FnPtr> struct RestInitFnInvoke {};

/**
 * Specialization for `RestInitFn`.
 */
template <> struct RestInitFnInvoke<RestInitFn> {
  static int invoke(RestInitFn fn, Module &) { return fn(); }
};

/**
 * Specialization for `RestInitFnWithMod`.
 */
template <> struct RestInitFnInvoke<RestInitFnWithMod> {
  static int invoke(RestInitFnWithMod fn, Module &mod) { return fn(mod); }
};

/**
 * Specialization for `nullptr`. It does nothing and returns 0.
 */
template <> struct RestInitFnInvoke<decltype(nullptr)> {
  static int invoke(decltype(nullptr), Module &) { return 0; }
};

/**
 * Test if a function pointer type is `RestInitFn` or `nullptr`.
 */
template <typename FnPtr> constexpr bool is_restini_fn(void) { return false; }

/**
 * Specialization for `RestInitFn`
 */
template <> constexpr bool is_restini_fn<RestInitFn>(void) { return true; }

/**
 * Specialization for `RestInitFnWithMod`.
 */
template <> constexpr bool is_restini_fn<RestInitFnWithMod>(void) {
  return true;
}

/**
 * Specialization for `nullptr`.
 */
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
    static ::cppbind::Module modobj(&moddef);                                  \
    int __result =                                                             \
        ::cppbind::RestInitFnInvoke<decltype(rest_init_fn)>::invoke(           \
            rest_init_fn, modobj);                                             \
    if (__result != 0) {                                                       \
      /* Module initialization failed. By spec, user has ImportError set. */   \
      return nullptr;                                                          \
    }                                                                          \
    return modobj.ptr();                                                       \
  }

} /* namespace cppbind */

#endif /* __PACKAGE_MOD_H__ */
