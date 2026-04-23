#ifndef __CONTAINER_DICT_H__
#define __CONTAINER_DICT_H__ (1)

#include "object.h"

namespace cppbind {

struct Dict {
  Object obj;
  Dict() : obj(PyDict_New()) {}
  ~Dict() = default;

  Object operator[](const Object &key) const {
    auto ret = Object(PyDict_GetItem(obj.ptr, key.ptr));
    if (ret.ptr == nullptr) {
      ret.ptr = Py_None;
    }
    ret.inc_ref(); /* give the object a reference.  */
    return ret;
  }

  void __setitem__(const Object &key, const Object &value) {
    int ret = PyDict_SetItem(obj.ptr, key.ptr, value.ptr);
    cppbind_assert(ret == 0 && "Failed to set item in dict.");
  }

  Py_ssize_t size(void) const { return PyDict_Size(obj.ptr); }
  Py_ssize_t __len__(void) const { return size(); }

  void clear(void) { PyDict_Clear(obj.ptr); }
  Object &object() { return this->obj; }
  const Object &object() const { return this->obj; }

};

} /* namespace cppbind */

#endif /* __CONTAINER_DICT_H__ */
