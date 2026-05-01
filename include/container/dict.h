#ifndef __CONTAINER_DICT_H__
#define __CONTAINER_DICT_H__ (1)

#include "object.h"

namespace cppbind {

/**
 * Python's `dict` class.
 *
 * <p>It supports item assignment and size:
 * <blockquote><pre>
 * Dict d;
 * Object some_key, some_value;
 *
 * d[some_key] = some_value; // OK
 * d.size(); // 1
 * some_value.ptr == d[some_key].ptr; // true
 * </pre></blockquote>
 * </p>
 *
 * @see {@link Dict::ObjectRef} for misuses of `[]` operator.
 */
struct Dict {
private:
  static void setitem(PyObject *dict, PyObject *key, const Object &value) {
    int ret = PyDict_SetItem(dict, key, value.ptr);
    cppbind_check_internal(ret == 0 && "Failed to set item in dict.");
  }

  static Object getitem(PyObject *dict, PyObject *key,
                        PyObject *default_value) {
    auto ret = Object(PyDict_GetItem(dict, key));
    if (ret.ptr == nullptr) {
      ret.ptr = default_value;
    }
    ret.inc_ref(); /* give the object a reference.  */
    return ret;
  }

  Object obj;

public:
  /**
   * Constructs an empty {@link Dict}.
   */
  Dict() : obj(PyDict_New()) {}

  /**
   * Construct from an {@link Object} holding a python dict.
   */
  Dict(const Object &ob) : obj(ob) {
    cppbind_check_internal(PyDict_Check(ob.ptr));
  }
  ~Dict() = default;

  /**
   * Construct a tuple from python method  key-value argument
   * `kwargs`.
   */
  static Dict from_kwargs(PyObject *kwargs) {
    Object obj{kwargs};
    obj.inc_ref(); /* neutralize the effect of ~Dict. */
    return Dict(obj);
  }

  /**
   * A class that acts as reference to value in the dict (a {@link Object}).
   * It exists because there is no way to get a reference to a value in a
   * {@link Dict}.
   *
   * <p>
   * <b>Note</b> that the following listings are incorrect:
   * <blockquote><pre>
   * Dict dict;
   * Object &object_ref = dict[some_key]; // wrong
   * auto &object_ref = dict[some_key]; // wrong
   * </pre></blockquote>
   * </p>
   *
   * <p>
   * Also <b>Note</b> that the {@link Dict} object should live longer than
   * {@link Dict::ObjectRef}, because the latter only holds weak reference
   * to the former. For example, the following listing is problematic:
   * <blockquote><pre>
   * Dict::ObjectRef objref;
   * {
   *    Dict d;
   *    d[some_key] = some_value;
   *    objref = d[some_key];
   * } // d is destroyed.
   * objref = Object();
   * </pre></blockquote>
   * </p>
   */
  struct ObjectRef {
    /**
     * A weak reference to the python dict.
     */
    PyObject *dict;

    /**
     * A weak reference to the key object.
     */
    PyObject *key;

    /**
     * Construct a value reference from dict and key.
     */
    ObjectRef(PyObject *dict, PyObject *key) : dict(dict), key(key) {}

    /**
     * Assigns an {@link Object} to `dict[key]`.
     */
    ObjectRef &operator=(const Object &value) {
      Dict::setitem(dict, key, value);
      return *this;
    }

    /**
     * Get the value object, or `None` if key does not exist.
     * It is equivalent to python code `dict.get(key, None)`.
     */
    operator Object() const { return Dict::getitem(dict, key, Py_None); }
  };

  /**
   * @return an {@link Object} (holding a strong reference)
   * to the value of key, or `Py_None` if not exists.
   *
   * The returned {@link Object} will be usable even if the
   * dict dies early; however, any modifications to the returned
   * object has no effect on the dict.
   */
  Object operator[](const Object &key) const {
    return getitem(obj.ptr, key.ptr, Py_None);
  }

  /**
   * @return {@link Dict::ObjectRef} to the value of key.
   * It does <b>not</b> check whether the key exists in the dict.
   */
  ObjectRef operator[](const Object &key) {
    return ObjectRef(obj.ptr, key.ptr);
  }

  /**
   * Set or overwrite value for key.
   */
  void __setitem__(const Object &key, const Object &value) {
    setitem(obj.ptr, key.ptr, value);
  }

  /**
   * @return size of the dict.
   */
  Py_ssize_t size(void) const { return PyDict_Size(obj.ptr); }

  /**
   * @return size of the dict.
   */
  Py_ssize_t __len__(void) const { return size(); }

  /**
   * Remove all key-value pairs in the dict.
   */
  void clear(void) { PyDict_Clear(obj.ptr); }

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

#endif /* __CONTAINER_DICT_H__ */
