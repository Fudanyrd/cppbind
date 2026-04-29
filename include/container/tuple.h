#ifndef __CONTAINER_TUPLE_H__
#define __CONTAINER_TUPLE_H__ (1)

#include "object.h"

namespace cppbind {

/**
 * Python tuple object (immutable) which supports `operator []`.
 */
struct Tuple {

private:
  template <typename _Tp> static Object *get_ptr(_Tp &obj) {
    return &obj.object();
  }

  Object obj;

public:
  /**
   * Construct an empty tuple.
   */
  Tuple() : obj(PyTuple_New(0)) {}

  /**
   * Convert a {@link Object} guarding a tuple to {@link Tuple}.
   */
  Tuple(const Object &other) : obj(other) {
    cppbind_check_internal(PyTuple_Check(other.ptr) &&
                           "not an instance of tuple");
  }

  /**
   * Construct a tuple from several objects (can be
   * {@link Object} or {@link List}, {@link Dict}, etc.).
   *
   * Example:
   * <blockquote><pre>
   * Dict foo();
   * Bytes bar();
   * Object none(Py_None);
   * tuple tuple(foo, bar, none);
   * tuple.size(); // 3
   * tuple[0]; // (dict) {}
   * tuple[1]; // (bytes) b""
   * tuple[2]; // None
   * </pre></blockquote>
   */
  template <typename... _Objects>
  Tuple(_Objects &...objs) : obj(PyTuple_New(sizeof...(_Objects))) {
    Object *arr[] = {get_ptr(objs)...};

    auto *pyobj = this->obj.ptr;
    for (size_t i = 0; i < (sizeof...(_Objects)); i++) {
      PyTuple_SetItem(pyobj, i, arr[i]->ptr);
    }
  }

  /**
   * Borrows a reference from `self[index]` and returns it, which
   * can be used even if self is destroyed.
   */
  Object operator[](Py_ssize_t index) const {
    auto ret = Object(PyTuple_GetItem(obj.ptr, index));
    cppbind_check_internal(ret.ptr != nullptr);
    ret.inc_ref(); /* give the object a reference.  */
    return ret;
  }

  /**
   * @return size of the tuple, equating to `len(self)`.
   */
  Py_ssize_t size(void) const { return PyTuple_Size(obj.ptr); }

  /**
   * @return size of the tuple, equating to `len(self)`.
   */
  Py_ssize_t __len__(void) const { return size(); }

  /**
   * @return the reference to current object.
   */
  Object &object() { return this->obj; }

  /**
   * @return the reference to current object.
   */
  const Object &object() const { return this->obj; }
};

} /* namespace cppbind */

#endif /* __CONTAINER_TUPLE_H__ */
