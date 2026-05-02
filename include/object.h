#ifndef __OBJECT_H__
#define __OBJECT_H__ (1)

#include "common.h"

namespace cppbind {

/**
 * Manage Python Object's reference counting via C++'s RAII.
 */
struct Object {
private:
public:
  /**
   * The pointer to the Python object.
   * It should never be initialized to nullptr.
   */
  PyObject *ptr; /* assumes not null */

  /**
   * Initialize from `PyObject *`. It will not increment
   * the reference count of <pre>pt</pre>.
   */
  explicit Object(PyObject *pt) : ptr(pt) {}

  /**
   * Copy constructor.
   */
  Object(const Object &other) {
    other.inc_ref();
    ptr = other.ptr;
  }

  /**
   * Move constructor. After move, the source object will not
   * hold a reference to the Python object.
   */
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

  /**
   * Copy assignment operator. It will increment the reference count of the new
   * object and decrement the reference count of the old object.
   */
  Object &operator=(const Object &other) {
    if (&other != this) {
      other.inc_ref();
      this->dec_ref();
    }
    ptr = other.ptr;
    return *this;
  }

  /**
   * Move assignment operator. After move, the source object will not hold a
   * reference to the Python object.
   */
  Object &operator=(Object &&other) noexcept {
    if (&other == this) {
      return *this;
    }
    dec_ref();
    ptr = other.ptr;
    other.ptr = nullptr;
    return *this;
  }

  /**
   * Decrement the reference count of the Python object. It will <b>not</b>
   * check if the pointer is nullptr, so it should only be called when the
   * object holds a reference to a Python object.
   */
  void dec_ref(void) const { Py_DECREF(this->ptr); }

  /**
   * Increment the reference count of the Python object. It will <b>not</b>
   * check if the pointer is nullptr, so it should only be called when the
   * object holds a reference to a Python object.
   */
  void inc_ref(void) const { Py_INCREF(this->ptr); }

  /**
   * Cast the pointer to type <pre>_Tp</pre>.
   */
  template <typename _Tp> _Tp *cast(void) const {
    return reinterpret_cast<_Tp *>(ptr);
  }

  /**
   * Convert the object to Python string.
   */
  PyObject *__str__() const { return PyObject_Str(ptr); }

  /**
   * Returns the object's python representation.
   */
  PyObject *__repr__() const { return PyObject_Repr(ptr); }

  /**
   * It does <b>not</b> check whether current object is null.
   *
   * @return the reference count to current object.
   */
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

  /**
   * Equivalent to python code:
   * <blockquote><pre>thisObject **= other
   * </pre></blockquote>
   */
  void inplace_num_power(PyObject *other, PyObject *modulo) {
    PyObject *ret = PyNumber_InPlacePower(ptr, other, modulo);
    if (ret != ptr) {
      dec_ref();
    }
    ptr = ret;
  }

  /**
   * Equivalent to python code:
   * <blockquote><pre>thisObject **= other
   * </pre></blockquote>
   */
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

  /**
   * Transfer ownership back to a PyObject pointer. It is
   * useful when a <pre>PyObject *</pre> should be returned.
   * For example:
   * <blockquote><pre>PyObject *someOp(PyObject *self, PyObject *args) {
   *   Object ret = { ... };
   *   return ret.unwrap();
   * }
   * </pre></blockquote>
   */
  PyObject *unwrap(void) {
    auto *ret = this->ptr;
    ptr = nullptr;
    return ret;
  }

  /**
   * @return the reference to current object.
   */
  Object &object() { return *this; }

  /**
   * @return the reference to current object.
   */
  const Object &object() const { return *this; }

  /**
   * Check if the object holds a reference to a Python object.
   */
  operator bool() const { return ptr != nullptr; }
};

} /* namespace cppbind */

#endif /* __OBJECT_H__ */
