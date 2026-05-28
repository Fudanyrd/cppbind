#ifndef __PACKAGE_MODOBJ_H__
#define __PACKAGE_MODOBJ_H__ (1)

#include "object.h"
#include "package/mod.h"

#include "package/bind.h"

namespace cppbind {

/**
 * A wrapper of Python module object.
 *
 * User should not create a `Module` object directly. Instead,
 * the macro `gen_modinit_fn_from_fns` registers the module
 * initialization function.
 */
struct Module {
private:
  PyObject *obj;

public:
  Module() = delete;
  /**
   * Initialize the module object with a `PyModuleDef` struct.
   * It is a wrapper of `PyModule_Create`.
   */
  Module(PyModuleDef *def) : obj(PyModule_Create(def)) {}
  ~Module() = default;

  /**
   * Add an integer constant to the module. It is a wrapper of
   * `PyModule_AddIntConstant`.
   *
   * @return -1 with an exception set on error, 0 on success.
   */
  int add_int_const(const char *name, int value) {
    return PyModule_AddIntConstant(obj, name, value);
  }

  /**
   * Add an object to the module. It is a wrapper of `PyModule_AddObject`.
   *
   * @return 0 on success. On error, raise an exception and return -1.
   */
  int add_object(const char *name, PyObject *value) {
    /**
     * Use AddObjectRef so that the reference is borrowed
     * instead of stealed.
     */
    return PyModule_AddObjectRef(obj, name, value);
  }

  /**
   * @return the guarded pointer to the module object.
   */
  PyObject *ptr() const { return obj; }

/**
 * Register a C++ class in the module object.
 *
 * @param modobj a {@link Module} object.
 * @param name (const char *) the name of the type in Python.
 * @param cpp_type the C++ type to be registered.
 */
#define add_cpp_type(modobj, cpp_type, name)                                   \
  do {                                                                         \
    PyObject *_the_obj =                                                       \
        reinterpret_cast<PyObject *>(cpp_get_type_object(cpp_type));           \
    modobj.add_object(name, _the_obj);                                         \
  } while (0)
};

} /* namespace cppbind */

#endif /* __PACKAGE_MODOBJ_H__ */
