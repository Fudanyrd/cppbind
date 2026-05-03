#ifndef __STL_H__
#define __STL_H__ (1)

#include <cstdarg>
#include <iostream>
#include <stdexcept>
#include <string>

#include "object.h"

/* Conversion of Objects to STL string */
namespace cppbind {

/**
 * Convert a Python object to an STL string.
 */
std::string stringify(PyObject *obj);

/**
 * Convert a Python object to an STL string.
 */
inline std::string stringify(const Object &obj) { return stringify(obj.ptr); }

/**
 * Convert a `HasObject` type (e.g. {@link List}, {@link Long}) to string.
 */
template <typename _Object_Ty>
inline std::string stringify(const _Object_Ty &obj) {
  return stringify(obj.object());
}

} /* namespace cppbind */

/* Print to STL output streams */
namespace cppbind {

/**
 * Writes the representation of a Python object to an STL output stream.
 */
std::ostream &operator<<(std::ostream &stream, PyObject *obj);

/**
 * Writes the representation of the Python object held by
 * {@link Object} to an STL output stream.
 */
inline std::ostream &operator<<(std::ostream &stream, const Object &obj) {
  return stream << obj.ptr;
}

/**
 * Writes the representation of the Python object held by a `HasObject` type
 * (e.g. {@link List}, {@link Long}) to an STL output stream.
 */
template <typename _Object_Ty>
inline std::ostream &operator<<(std::ostream &stream, const _Object_Ty &obj) {
  return stream << obj.object();
}

} /* namespace cppbind */

#endif /* __STL_H__ */
