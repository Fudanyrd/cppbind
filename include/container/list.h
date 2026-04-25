#ifndef __CONTAINER_LIST_H__
#define __CONTAINER_LIST_H__ (1)

#include "object.h"

namespace cppbind {

struct List {
private:
  template <typename _Tp> static Object *get_ptr(_Tp &obj) {
    return &obj.object();
  }

  static Object getitem(PyObject *list, Py_ssize_t index) {
    auto ret = Object(PyList_GetItem(list, index));
    cppbind_check_internal(ret.ptr != nullptr);
    ret.inc_ref(); /* give the object a reference.  */
    return ret;
  }

  static void setitem(PyObject *list, Py_ssize_t index, const Object &obj) {
    int ret = PyList_SetItem(list, index, obj.ptr);
    if (ret != 0) {
      /* Clear `IndexError` */
      PyErr_Clear();
    }
  }

public:
  /* Surrogate class for (Object &). */
  struct ObjectRef {
    PyObject *list;
    Py_ssize_t idx;

    ObjectRef(PyObject *list, Py_ssize_t index) : list(list), idx(index) {}

    ObjectRef &operator=(const Object &obj) {
      List::setitem(list, idx, obj);
      return *this;
    }

    operator Object() const { return List::getitem(list, idx); }
  };

  Object obj;

  List(void) : obj(PyList_New(0)) {}
  List(const Object &other) : obj(other) {}

  template <typename... _Objects>
  List(_Objects &...objs) : obj(PyList_New(sizeof...(_Objects))) {
    Object *arr[] = {get_ptr(objs)...};

    auto *pyobj = this->obj.ptr;
    for (size_t i = 0; i < (sizeof...(_Objects)); i++) {
      PyList_SetItem(pyobj, i, arr[i]->ptr);
    }
  }

  void append(const Object &object) { PyList_Append(obj.ptr, object.ptr); }
  template <typename _OBject_Ty> void append(const _OBject_Ty &object) {
    append(object.object());
  }

  Py_ssize_t size(void) const { return PyList_Size(obj.ptr); }
  Py_ssize_t __len__(void) const { return size(); }

  Object &object() { return this->obj; }
  const Object &object() const { return this->obj; }

  Object operator[](Py_ssize_t index) const { return getitem(obj.ptr, index); }

  ObjectRef operator[](Py_ssize_t index) { return ObjectRef(obj.ptr, index); }
};

} /* namespace cppbind */

#endif /* __CONTAINER_LIST_H__ */
