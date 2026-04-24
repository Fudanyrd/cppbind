#ifndef __CONTAINER_STR_H__
#define __CONTAINER_STR_H__ (1)

#include "common.h"
#include "object.h"

namespace cppbind {

struct Str {
public:
  Object obj;

  Str(const char *data) : obj(PyUnicode_FromString(data)) {}

#define Str_from_string_and_size(TP)                                           \
  Str(const TP *data, Py_ssize_t len)                                          \
      : obj(PyUnicode_FromStringAndSize(reinterpret_cast<const char *>(data),  \
                                        len * sizeof(TP))) {                   \
    __static_assert(is_char_ty<TP>());                                         \
  }

  char_types(Str_from_string_and_size)
#undef Str_from_string_and_size

      Str(void)
      : obj(PyUnicode_FromString("")) {
  }
  Str(const Object &object) : obj(object) {
    cppbind_assert(PyUnicode_Check(obj.ptr));
  }

  Str &operator+=(const Str &other) {
    PyObject *ptr = obj.unwrap();
    PyUnicode_Append(&ptr, other.obj.ptr);
    obj.ptr = ptr;
    return *this;
  }

  Object &object() { return this->obj; }
  const Object &object() const { return this->obj; }

  const char *data() const {
    return reinterpret_cast<const char *>(PyUnicode_DATA(obj.ptr));
  }
  const char *c_str() const { return data(); }

  bool operator==(const Str &other) const {
    return PyUnicode_Compare(obj.ptr, other.obj.ptr) == 0;
  }
  bool operator!=(const Str &other) const { return !(*this == other); }

  Str operator[](Py_ssize_t index) const {
    PyObject *ptr = PyUnicode_Substring(obj.ptr, index, index + 1);
    return Str{Object{ptr}};
  }

  Str substr(Py_ssize_t start, Py_ssize_t end) const {
    PyObject *ptr = PyUnicode_Substring(obj.ptr, start, end);
    return Str{Object{ptr}};
  }

  Py_ssize_t size(void) const { return PyUnicode_GetLength(obj.ptr); }
};

inline Str operator+(const Str &lhs, const Str &rhs) {
  PyObject *ptr = PyUnicode_Concat(lhs.obj.ptr, rhs.obj.ptr);
  return Str{Object(ptr)};
}

} /* namespace cppbind */

#endif /* __CONTAINER_STR_H__ */
