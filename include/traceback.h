#ifndef __TRACEBACK_H__
#define __TRACEBACK_H__ (1)

#include <stdexcept>

#include <pyerrors.h>

#include <common.h>

/* Exception handling */
namespace cppbind {
#define cppbind_raise_with_message(exc_object, message)                        \
  do {                                                                         \
    PyErr_SetString((exc_object), reinterpret_cast<const char *>(message));    \
    return nullptr;                                                            \
  } while (0)

#define cppbind_raise(exc_object)                                              \
  do {                                                                         \
    PyErr_SetString((exc_object), Py_None);                                    \
    return nullptr;                                                            \
  } while (0)

#define map_except_cpp_py(X)                                                   \
  X(std::invalid_argument, PyExc_TypeError)                                    \
  X(std::logic_error, PyExc_ValueError)                                        \
  X(std::runtime_error, PyExc_RuntimeError)                                    \
  X(std::out_of_range, PyExc_IndexError)                                       \
  X(std::length_error, PyExc_IndexError)                                       \
  X(std::overflow_error, PyExc_OverflowError)                                  \
  X(std::underflow_error, PyExc_OverflowError)                                 \
  X(std::range_error, PyExc_OverflowError)                                     \
  X(std::domain_error, PyExc_ValueError)

/**
 * Set python exception from an `std::exception`.
 */
void PyErr_from_cpp_exception(std::exception &ex) noexcept;

} /* namespace cppbind */

#endif /* __TRACEBACK_H__ */
