#include <Python.h>

/**
 * Initialize the Python interpreter before gtest starts.
 */
__attribute__((constructor)) void _ZN7cppbind4initEv(void) {
  Py_Initialize();
}

/**
 * Finalize the Python interpreter after gtest ends.
 */
__attribute__((destructor)) void _ZN7cppbind4finiEv(void) {
  Py_Finalize();
}
