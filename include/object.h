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
  Object(Object &&other) noexcept {
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

  Object &operator=(Object &&other) noexcept {
    if (&other == this) {
      return *this;
    }
    dec_ref();
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

  Py_ssize_t ref_count(void) const { return Py_REFCNT(ptr); }

#define num_inplace_op(op)                                                     \
  void CONCAT(inplace_num_, op)(PyObject * other) {                            \
    PyObject *ret = CONCAT(PyNumber_InPlace, op)(ptr, other);                  \
    if (ret != ptr) {                                                          \
      dec_ref();                                                               \
    }                                                                          \
    ptr = ret;                                                                 \
  }                                                                            \
  void CONCAT(inplace_num_, op)(const Object &other) {                         \
    CONCAT(inplace_num_, op)(other.ptr);                                       \
  }

  void inplace_num_power(PyObject *other, PyObject *modulo) {
    PyObject *ret = PyNumber_InPlacePower(ptr, other, modulo);
    if (ret != ptr) {
      dec_ref();
    }
    ptr = ret;
  }
  void inplace_num_power(Object &other, Object &modulo) {
    inplace_num_power(other.ptr, modulo.ptr);
  }

#define FOREACH(X)                                                             \
  X(Add)                                                                       \
  X(Subtract)                                                                  \
  X(Multiply)                                                                  \
  X(FloorDivide)                                                               \
  X(TrueDivide)                                                                \
  X(Remainder)                                                                 \
  X(Lshift)                                                                    \
  X(Rshift)                                                                    \
  X(And)                                                                       \
  X(Xor)                                                                       \
  X(Or)

  /* Python number in-place operations. */
  FOREACH(num_inplace_op)
#undef FOREACH
#undef num_inplace_op

  /* Transfer ownership back to a PyObject pointer. */
  PyObject *unwrap(void) {
    auto *ret = this->ptr;
    ptr = nullptr;
    return ret;
  }

  Object &object() { return *this; }
  const Object &object() const { return *this; }
};

} /* namespace cppbind */

#endif /* __OBJECT_H__ */
