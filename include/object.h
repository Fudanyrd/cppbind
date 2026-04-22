#ifndef __OBJECT_H__
#define __OBJECT_H__ (1)

#include "common.h"

namespace cppbind {

/*
 * Manage Python Object's reference counting via C++'s RAII.
 */
struct Object {
private:
public:
  PyObject *ptr; /* assumes not null */

  explicit Object(PyObject *pt) : ptr(pt) {}
  Object(const Object &other) {
    other.inc_ref();
    ptr = other.ptr;
  }
  Object(Object &&other) {
    ptr = other.ptr;
    other.ptr = nullptr;
  }

  ~Object() {
    /*
     * After move, it might be nullptr.
     * However, we do not use `Py_XDECREF` (which can accept nullptr)
     * to avoid one function call.
     */
    if (ptr != nullptr) {
      Py_DECREF(ptr);
    }
  }

  Object &operator=(const Object &other) {
    if (&other != this) {
      other.inc_ref();
      this->dec_ref();
    }
    ptr = other.ptr;
    return *this;
  }

  Object &operator=(Object &&other) {
    if (&other != this) {
      this->dec_ref();
    }
    ptr = other.ptr;
    other.ptr = nullptr;
    return *this;
  }

  void dec_ref(void) const { Py_DECREF(this->ptr); }

  void inc_ref(void) const { Py_INCREF(this->ptr); }

  template <typename _Tp> _Tp *cast(void) const {
    return reinterpret_cast<_Tp *>(ptr);
  }

  PyObject *__str__() const { return PyObject_Str(ptr); }
  PyObject *__repr__() const { return PyObject_Repr(ptr); }
};

} /* namespace cppbind */

#endif /* __OBJECT_H__ */
