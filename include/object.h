#ifndef __OBJECT_H__
#define __OBJECT_H__ (1)

#include "common.h"

#include <stdexcept>
#include <unordered_set>

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

  /**
   * Create an `Object` that holds a borrowed reference to `Py_None`.
   */
  static Object none() { return Object(Py_NewRef(Py_None)); }

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

#define rich_compare_ops(X)                                                    \
  X(<, Py_LT)                                                                  \
  X(>, Py_GT)                                                                  \
  X(==, Py_EQ)                                                                 \
  X(!=, Py_NE)                                                                 \
  X(<=, Py_LE)                                                                 \
  X(>=, Py_GE)

#define gen_compare_operator(op, op_code)                                      \
  bool operator op(const Object &other) const {                                \
    int ret = PyObject_RichCompareBool(ptr, other.ptr, op_code);               \
    if (ret == -1) {                                                           \
      /* Type Error */                                                         \
      throw std::invalid_argument("Failed to compare objects.");               \
    }                                                                          \
    return static_cast<bool>(ret);                                             \
  }

  /**
   * Compare two objects, without exception (returns -1 on failure).
   *
   * @param other: other object to compare with.
   * @param op_code: one of the following values:
   * `Py_LT`, `Py_LE`, `Py_EQ`, `Py_NE`, `Py_GT`, `Py_GE`.
   */
  int rich_compare(const Object &other, int op_code) const noexcept {
    int ret = PyObject_RichCompareBool(ptr, other.ptr, op_code);
    return ret;
  }

  /**
   * Compare two Python objects with the given operator. It will throw
   * `std::invalid_argument` if the comparison is not supported by the
   * Python objects.
   */
  rich_compare_ops(gen_compare_operator);
#undef rich_compare_ops
#undef gen_compare_operator

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

namespace std {

/**
 * Specialization of std::hash for cppbind::Object.
 */
template <> struct hash<cppbind::Object> {
  /**
   * Hash the Python object. A more silent version (does not throw exception)
   * could be relying on `id()` when not hashable:
   * <blockquote><pre>
   * size_t operator()(const cppbind::Object &obj) const noexcept {
   *   auto ret = PyObject_Hash(obj.ptr);
   *   if (ret == -1) { return std::hash<PyObject *>{}(obj.ptr); }
   *   return static_cast<size_t>(ret);
   * }
   * </pre></blockquote>
   *
   * @throw std::invalid_argument if the object is not hashable.
   */
  size_t operator()(const cppbind::Object &obj) const {
    auto ret = PyObject_Hash(obj.ptr);
    if (ret == -1) {
      throw std::invalid_argument("Failed to hash the object.");
    }
    return static_cast<size_t>(ret);
  }
};

} /* namespace std */

#endif /* __OBJECT_H__ */
