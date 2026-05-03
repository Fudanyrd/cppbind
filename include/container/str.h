#ifndef __CONTAINER_STR_H__
#define __CONTAINER_STR_H__ (1)

#include "common.h"
#include "object.h"

namespace cppbind {

/**
 * Python string type. Like python `str`, it does not
 * support item assignment, `==`, `!=`, `+` (concatenate),
 * and `+=`.
 *
 * <h2>Convertion to and from std::string</h2>
 * We see that using {@link Str::data} or {@link Str::c_str}
 * to convert to std::string is incorrect.
 *
 * <blockquote><pre>
 *  std::string cpp_utf8_str("abc😃");
 *  cpp_utf8_str.size(); // 7
 *  cpp_utf8_str[3]; // 0xf0, first char in 😃
 *
 *  // convert to python str.
 *  cppbind::Str py_str(cpp_utf8_str.c_str());
 *  py_str[3];  // Str("😃")
 *  py_str.size(); // 4
 *
 *  // convert back to std::string.
 *  cppbind::stringify(py_str) == cpp_utf8_str; // true
 *  std::string(py_str.data()) == cpp_utf8_str; // probably false
 * </pre></blockquote>
 */
struct Str {
public:
private:
  Object obj;

public:
  /**
   * Construct from C-like null-terminated string.
   */
  Str(const char *data) : obj(PyUnicode_FromString(data)) {}

#define Str_from_string_and_size(TP)                                           \
  Str(const TP *data, Py_ssize_t len)                                          \
      : obj(PyUnicode_FromStringAndSize(reinterpret_cast<const char *>(data),  \
                                        len * sizeof(TP))) {                   \
    __static_assert(is_char_ty<TP>());                                         \
  }

  char_types(Str_from_string_and_size)
#undef Str_from_string_and_size

      /**
       * Construct an empty string.
       */
      Str(void)
      : obj(PyUnicode_FromString("")) {
  }

  /**
   * Construct from an {@link Object} guarding a python str.
   */
  Str(const Object &object) : obj(object) {
    cppbind_check_internal(PyUnicode_Check(obj.ptr));
  }

  /**
   * Append the content `other` to the back of this string.
   * Equivalent to python `self += other`.
   */
  Str &operator+=(const Str &other) {
    PyObject *ptr = obj.unwrap();
    PyUnicode_Append(&ptr, other.obj.ptr);
    obj.ptr = ptr;
    return *this;
  }

  /**
   * @return the reference to current object.
   */
  Object &object() { return this->obj; }
  /**
   * @return the reference to current object.
   */
  const Object &object() const { return this->obj; }

  /**
   * @return the raw unicode buffer. Using this for a
   * C null-terminated string is <b>incorrect</b>.
   */
  const char *data() const {
    return reinterpret_cast<const char *>(PyUnicode_DATA(obj.ptr));
  }

  /**
   * @return the raw unicode buffer. Using this for a
   * C null-terminated string is <b>incorrect</b>.
   */
  const char *c_str() const { return data(); }

  /**
   * @return true if content of self is equal to that of other.
   */
  bool operator==(const Str &other) const {
    return PyUnicode_Compare(obj.ptr, other.obj.ptr) == 0;
  }

  /**
   * @return true if content of self is not equal to that of other.
   */
  bool operator!=(const Str &other) const { return !(*this == other); }

  /**
   * Note that the behavior is not same as `std::string`.
   *
   * @return the "character" at index `index`, equal to
   * python code `self[index]`.
   */
  Str operator[](Py_ssize_t index) const {
    PyObject *ptr = PyUnicode_Substring(obj.ptr, index, index + 1);
    return Str{Object{ptr}};
  }

  /**
   * @return a copy of this string starting from `start` and
   * ending at `end` (exlude). Equal to `self[start:end]`.
   */
  Str substr(Py_ssize_t start, Py_ssize_t end) const {
    PyObject *ptr = PyUnicode_Substring(obj.ptr, start, end);
    return Str{Object{ptr}};
  }

  /**
   * @return length of the string, equal to `len(self)`.
   */
  Py_ssize_t size(void) const { return PyUnicode_GetLength(obj.ptr); }
};

/**
 * {@link Str} concatenation.
 */
inline Str operator+(const Str &lhs, const Str &rhs) {
  PyObject *ptr = PyUnicode_Concat(lhs.object().ptr, rhs.object().ptr);
  return Str{Object(ptr)};
}

} /* namespace cppbind */

#endif /* __CONTAINER_STR_H__ */
