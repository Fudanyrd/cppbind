#include <Python.h>

__attribute__((constructor)) void _ZN7cppbind4initEv(void) {
  Py_Initialize();
}

__attribute__((destructor)) void _ZN7cppbind4finiEv(void) {
  Py_Finalize();
}
