#ifndef __CONTAINER_TUPLE_H__
#define __CONTAINER_TUPLE_H__ (1)

#include "object.h"

namespace cppbind {

struct Tuple {

private:
  template <typename _Tp> static Object *get_ptr(_Tp &obj) {
    return &obj.object();
  }

public:
  Object obj;
  Tuple() : obj(PyTuple_New(0)) {}
  Tuple(const Object &other) : obj(other) {}

  template <typename... _Objects>
  Tuple(_Objects &...objs) : obj(PyTuple_New(sizeof...(_Objects))) {
    Object *arr[] = {get_ptr(objs)...};

    auto *pyobj = this->obj.ptr;
    for (size_t i = 0; i < (sizeof...(_Objects)); i++) {
      PyTuple_SetItem(pyobj, i, arr[i]->ptr);
    }
  }

  Object operator[](Py_ssize_t index) const {
    auto ret = Object(PyTuple_GetItem(obj.ptr, index));
    cppbind_assert(ret.ptr != nullptr);
    ret.inc_ref(); /* give the object a reference.  */
    return ret;
  }

  Py_ssize_t size(void) const { return PyTuple_Size(obj.ptr); }
  Py_ssize_t __len__(void) const { return size(); }

  Object &object() { return this->obj; }
  const Object &object() const { return this->obj; }
};

} /* namespace cppbind */

#endif /* __CONTAINER_TUPLE_H__ */
