#include <cppbind.h>
#include <gtest/gtest.h>

namespace cppbind {

TEST(Exception, Conversion) {
  try {
    PyErr_Clear();
    throw std::invalid_argument("invalid argument");
  } catch (std::exception &ex) {
    PyErr_from_cpp_exception(ex);
  }

  PyObject *exc_type = PyErr_Occurred();
  PyErr_Clear();
  ASSERT_EQ(exc_type, PyExc_TypeError);
}

} /* namespace cppbind */
