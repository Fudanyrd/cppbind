#include <cppbind.h>

namespace cppbind {

#define test_and_return(cpp_except, py_except)                                 \
  if (dynamic_cast<cpp_except *>(&ex) != nullptr) {                            \
    PyErr_SetString(py_except, ex.what());                                     \
    return;                                                                    \
  }

void PyErr_from_cpp_exception(std::exception &ex) noexcept {
  map_except_cpp_py(test_and_return);
  PyErr_SetString(PyExc_Exception, ex.what());
  return;
}

} /* namespace cppbind */
