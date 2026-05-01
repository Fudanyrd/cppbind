#include <cppbind.h>

namespace cppbind {

/* NOLINTBEGIN(bugprone-macro-parentheses) */
/* Because of the C++ type argument.*/

#define test_and_return(cpp_except, py_except)                                 \
  if (dynamic_cast<cpp_except *>(&except) != nullptr) {                        \
    PyErr_SetString((py_except), except.what());                               \
    return;                                                                    \
  }

/* NOLINTEND(bugprone-macro-parentheses) */

void PyErr_from_cpp_exception(std::exception &except) noexcept {
  map_except_cpp_py(test_and_return);
  PyErr_SetString(PyExc_Exception, except.what());
}

} /* namespace cppbind */
