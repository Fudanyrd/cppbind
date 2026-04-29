#ifndef __CONTAINER_LIST_H__
#define __CONTAINER_LIST_H__ (1)

#include "object.h"

namespace cppbind {

/**
 * Python list object. It supports item assignment and size:
 * <blockquote><pre>
 *   List list;
 *   list.size(); // 0
 *   Object obj{Py_None};
 *   list.append(obj); // OK
 *   list.size(); // 1
 *   list[0].ptr == Py_None; // true
 *   list[0] = Bytes("😃").object(); // item assignment
 *   list[1] = obj; // IndexError cleared.
 * </pre></blockquote>
 *
 * @see {@link List::ObjectRef} for reference to list items.
 */
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
      cppbind_check_internal(0 && "failed to setitem");
      PyErr_Clear();
    }
  }

public:
  /**
   * a class that acts like reference to list items ({@link Object}s)
   * to make C++'s `[]` operator work for {@link List}. It stores
   * a <b>weak reference</b> to an item in the list.
   *
   * <p>
   * because of {@link List::ObjectRef}, the following listings are
   * incorrect:
   * <blockquote><pre>
   * List list;
   * list.append(Bytes().object());
   * Object &obj = list[0]; // compile error
   * auto &obj = list[0]; // compile error
   * </pre></blockquote>
   * </p>
   *
   * <p>
   * <b>Note</b> also that the struct only holds weak reference,
   * so the user must ensure that its underlying {@link List} lives
   * long enough.
   * </p>
   */
  struct ObjectRef {
  private:
    PyObject *list;
    Py_ssize_t idx;

  public:
    /**
     * Construct a reference via list and index.
     */
    ObjectRef(PyObject *list, Py_ssize_t index) : list(list), idx(index) {}

    /**
     * Assigns a new {@link Object}, equvalent to python code
     * `list[idx] = obj`.
     */
    ObjectRef &operator=(const Object &obj) {
      List::setitem(list, idx, obj);
      return *this;
    }

    /**
     * @return the object being referenced.
     */
    operator Object() const { return List::getitem(list, idx); }
  };

private:
  Object obj;

public:
  /**
   * Default constructor: an empty list.
   */
  List(void) : obj(PyList_New(0)) {}

  /**
   * Borrow a strong reference to python list other.
   * It will check that `other` holds a python list
   * only when internal assertion is enabled.
   */
  List(const Object &other) : obj(other) {
    cppbind_check_internal(PyList_Check(other.ptr));
  }

  /**
   * Construct a list from several objects (can be
   * {@link Object} or {@link List}, {@link Dict}, etc.).
   *
   * Example:
   * <blockquote><pre>
   * Dict foo();
   * Bytes bar();
   * Object none(Py_None);
   * List list(foo, bar, none);
   * list.size(); // 3
   * list[0]; // (dict) {}
   * list[1]; // (bytes) b""
   * list[2]; // None
   * </pre></blockquote>
   */
  template <typename... _Objects>
  List(_Objects &...objs) : obj(PyList_New(sizeof...(_Objects))) {
    Object *arr[] = {get_ptr(objs)...};

    auto *pyobj = this->obj.ptr;
    for (size_t i = 0; i < (sizeof...(_Objects)); i++) {
      PyList_SetItem(pyobj, i, arr[i]->ptr);
    }
  }

  /**
   * Equivalent to python code `self.append(object)`.
   */
  void append(const Object &object) { PyList_Append(obj.ptr, object.ptr); }

  /**
   * Equivalent to python code `self.append(object)`.
   */
  template <typename _OBject_Ty> void append(const _OBject_Ty &object) {
    append(object.object());
  }

  /**
   * @return size of list; equivalent to `len(self)` in Python.
   */
  Py_ssize_t size(void) const { return PyList_Size(obj.ptr); }

  /**
   * @return size of list; equivalent to `len(self)` in Python.
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

  /**
   * Borrows a strong reference from `self[index]` and returns it.
   * It will live longer than `self`, but modifications to it
   * does not change the underlying {@link List}.
   */
  Object operator[](Py_ssize_t index) const { return getitem(obj.ptr, index); }

  /**
   * The user have to make sure that `index` is not out of bounds.
   *
   * @return a {@link List::ObjectRef} to the item at index `index`.
   */
  ObjectRef operator[](Py_ssize_t index) { return ObjectRef(obj.ptr, index); }
};

} /* namespace cppbind */

#endif /* __CONTAINER_LIST_H__ */
