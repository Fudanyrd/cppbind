#ifndef __EXAMPLE_FFI_H__
#define __EXAMPLE_FFI_H__ (1)

#include <map>

#include <cppbind.h>

extern "C" {
/* CInt python built-in functions */

/**
 * Constructs a {@link CInt} object and initalize with value 0.
 */
PyObject *CInt_New(PyObject *self);

/**
 * Constructs a {@link CInt} object and initalize with
 * value in `arg[0]`.
 */
PyObject *CInt_FromInt(PyObject *self, PyObject *arg);

/* CppMap python built-in functions */

/**
 * Construct an empty {@link CppMap} object with
 * the provided compare function.
 */
PyObject *CppMap_New(PyObject *self, PyObject *compare_fn);
} /* extern "C" */

namespace example {

/**
 * An integer used interchangeably in C++ and Python.
 *
 * This is possible because it:
 * <ul>
 *   <li>has first non-static member (i.e. at offset 0) is `PyObject`</li>
 *   <li>has initialized its type in python via `type_static_members` and
 *    `type_init`.</li>
 * </ul>
 */
struct CInt {
  CInt() : num(0) {}

  /**
   * @return the integer value of this `CInt` object.
   */
  int getvalue(void) const { return num; }

  /**
   * Copy assignment operator. It will only copy the `num` member.
   */
  CInt &operator=(const CInt &other) {
    /* Only copies the num member. */
    num = other.num;
    return *this;
  }

  /**
   * Convertion to `long long` for use.
   */
  operator long long() const { return static_cast<long long>(num); }

  /**
   * Forward an {@link cppbind::Object} to {@link CInt} if possible. It will
   * check that the argument is either a {@link CInt} object or a python
   * integer, and convert it to {@link CInt} if possible. Otherwise, it will set
   * a `ValueError` and return an null {@link cppbind::Object}.
   */
  static ::cppbind::Object forward_or_convert(const ::cppbind::Object &arg) {
    if (arg.ptr == nullptr) {
      PyErr_SetString(PyExc_ValueError, "argument cannot be null");
      return ::cppbind::Object(nullptr);
    }
    if (PyObject_TypeCheck(arg.ptr, cppbind::Type<CInt>::instance)) {
      return arg;
    } else if (PyLong_Check(arg.ptr)) {
      PyObject *ret = _PyObject_New(cppbind::Type<CInt>::instance);
      if (ret != nullptr) {
        new (ret) CInt();
        reinterpret_cast<CInt *>(ret)->num = PyLong_AsLong(arg.ptr);
      }
      return ::cppbind::Object(ret);
    }
    return ::cppbind::Object(nullptr);
  }

#define CInt_inplace_operator(Operator)                                        \
  CInt &operator Operator(const CInt & other) {                                \
    num Operator other.num;                                                    \
    return *this;                                                              \
  }

  /**
   * {@link CInt} inplace integer operators.
   */
  type_integer_inplace_ops(CInt_inplace_operator);
#undef CInt_inplace_operator

private:
  PyObject pyobj;

public:
  /**
   * Value of this object.
   */
  int num;
};

/**
 * Use `std::map` in Python.
 */
struct CppMap {
  /**
   * Comparator for `std::map`. It will call the provided Python callable to
   * compare two Python objects (keys in the map).
   */
  struct Compare {
  private:
    ::cppbind::Object compare;

  public:
    /**
     * @param comparefn: a Python callable that takes two arguments and
     * returns either `True` (first argument is less than the second argument)
     * or `False`.
     */
    Compare(PyObject *comparefn) : compare(comparefn) {
      /* Borrow a reference of this compare function. */
      compare.inc_ref();
    }
    ~Compare() = default;

    /**
     * @return true if `compare(a, b)` returns `True`.
     */
    bool operator()(PyObject *a, PyObject *b) const {
      auto *ret = PyObject_CallFunctionObjArgs(compare.ptr, a, b, nullptr);
      assert(ret != nullptr);
      return PyObject_IsTrue(ret);
    }

    /**
     * If `a` and `b` cannot be compared by the provided compare function, it
     * will set an python exception and return false. Otherwise, it will return
     * true.
     */
    bool comparable(PyObject *a, PyObject *b) const {
      auto *ret = PyObject_CallFunctionObjArgs(compare.ptr, a, b, nullptr);
      if (ret == nullptr) {
        return false;
      }
      Py_DECREF(ret);
      return true;
    }
  };

  /**
   * @param compare_fn: a Python callable that takes two arguments and
   * returns either `True` (first argument is less than the second argument)
   * or `False`.
   */
  CppMap(PyObject *compare_fn) : table(Compare(compare_fn)) {}
  ~CppMap() = default;

  /**
   * Check whether `obj` can be compared with existing keys in the map using the
   * comparator before `get` or `put`. If not, python exception is set
   * accordingly.
   *
   * @return true if `obj` can be compared with existing keys
   * in the map.
   */
  bool comparable(PyObject *obj) const {
    PyObject *other = table.empty() ? obj : (table.begin()->first);
    auto comparator = table.key_comp();
    return comparator.comparable(obj, other);
  }

  /**
   * @return the value associated with key, or `None` if key is not present.
   */
  PyObject *get(PyObject *key) const {
    if (!comparable(key)) {
      return nullptr;
    }
    auto it = table.find(key);
    if (it == table.end()) {
      return Py_None;
    }
    return it->second;
  }

  /**
   * @param key: a tuple of length 1, with the query key;
   * or length 2 with the query key and default value.
   */
  PyObject *get(const ::cppbind::Tuple &key) const;

  /**
   * @param args: a tuple of length 2, key and value.
   */
  PyObject *put(const ::cppbind::Tuple &args);

  /**
   * Set value associated with key to `value`.
   * If key already exists, previous value will be overwrite.
   */
  PyObject *put(PyObject *key, PyObject *value) {
    if (!comparable(key)) {
      return nullptr;
    }
    table[key] = value;
    Py_INCREF(value);
    return Py_None;
  }

  /**
   * This is required for a cppbind's mapping class.
   *
   * @return test result of key's presence.
   */
  bool contains(PyObject *key) const {
    if (!comparable(key)) {
      /* comparable sets an expection; clears it. */
      PyErr_Clear();
      return false;
    }
    return table.find(key) != table.end();
  }

  /**
   * This is required for a cppbind's mapping class.
   *
   * @return size of the map.
   */
  size_t size() const { return table.size(); }

  /**
   * Test whether `arg` is an instance of {@link CppMap}.
   *  Required for cppbind classes.
   */
  static ::cppbind::Object forward_or_convert(const ::cppbind::Object &arg) {
    if (arg.ptr == nullptr) {
      PyErr_SetString(PyExc_ValueError, "argument cannot be null");
      return ::cppbind::Object(nullptr);
    }
    if (PyObject_TypeCheck(arg.ptr, cppbind::Type<CppMap>::instance)) {
      return arg;
    }
    return ::cppbind::Object(nullptr);
  }

private:
  PyObject pyobj;
  ::std::map<PyObject *, PyObject *, Compare> table;
};

} /* namespace example */

#define CInt_binary_operator_decl(Operator)                                    \
  example::CInt operator Operator(const example::CInt &a,                      \
                                  const example::CInt &b);

type_integer_binary_ops(CInt_binary_operator_decl);

#define CInt_unary_operator_decl(Operator)                                     \
  example::CInt operator Operator(const example::CInt &a);

type_integer_unary_ops(CInt_unary_operator_decl);

#undef CInt_binary_operator_decl
#undef CInt_unary_operator_decl
#endif /* __EXAMPLE_FFI_H__ */
