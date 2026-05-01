#ifndef __STL_H__
#define __STL_H__ (1)

#include <cstdarg>
#include <iostream>
#include <stdexcept>
#include <string>

#include "object.h"

/* Conversion of Objects to STL string */
namespace cppbind {

std::string stringify(PyObject *obj);

inline std::string stringify(const Object &obj) { return stringify(obj.ptr); }

template <typename _Object_Ty>
inline std::string stringify(const _Object_Ty &obj) {
  return stringify(obj.object());
}

} /* namespace cppbind */

/* Print to STL output streams */
namespace cppbind {

std::ostream &operator<<(std::ostream &stream, PyObject *obj);

inline std::ostream &operator<<(std::ostream &stream, const Object &obj) {
  return stream << obj.ptr;
}

template <typename _Object_Ty>
inline std::ostream &operator<<(std::ostream &stream, const _Object_Ty &obj) {
  return stream << obj.object();
}

} /* namespace cppbind */

#endif /* __STL_H__ */
