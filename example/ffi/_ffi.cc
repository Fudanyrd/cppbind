#include <cstdio>
#include <map>

#include "ffi.h"
#include <cppbind.h>

using cppbind::MethodWrapper;
using example::CInt;
using example::CppMap;

type_static_members(CInt);
type_static_members(CppMap);

/**
 * Package finalization function. It will free the type objects of {@link CInt}
 * and and {@link CppMap}.
 */
static int _ffi_clear(PyObject *) {
  ::cppbind::Type<CInt>::module_free();
  ::cppbind::Type<CppMap>::module_free();
  return 0;
}

/**
 * Package initialization function. It will initialize the
 * type objects and method tables of {@link CInt} and {@link CppMap}.
 */
static int _ffi_rest_init(void) {
  type_init("_ffi", CInt, "CInt",
            MethodTableEntry_build_noarg("_ffi", CInt, getvalue));
  type_init("_ffi", CppMap, "CppMap",
            MethodTableEntry_build_noarg("_ffi", CppMap, size),
            MethodTableEntry_build_args("_ffi", CppMap, get),
            MethodTableEntry_build_args("_ffi", CppMap, put));
  type_init_integer_ops("_ffi", CInt, "CInt");
  type_init_mapping("_ffi", CppMap, "CppMap");
  return 0;
}

gen_modinit_fn_from_fns(
    _ffi, &_ffi_rest_init, nullptr, nullptr, &_ffi_clear,
    gen_PyMethodDef_doc(CInt_New, ":returns: a new CInt object"),
    (PyMethodDef){"CInt_FromInt", (PyCFunction)CInt_FromInt, METH_O,
                  "Creates a new CInt object from an integer."},
    (PyMethodDef){
        "CppMap_New", (PyCFunction)CppMap_New, METH_O,
        "Creates a new CppMap object with the given compare function."})
