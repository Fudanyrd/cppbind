#ifndef __PACKAGE_INVOKE_H__
#define __PACKAGE_INVOKE_H__ (1)

#include <functional> /* std::function */
#include <methodobject.h>

#include <cast.h> /* cppbind::from and cppbind::into */
#include <common.h>
#include <container/dict.h>
#include <container/list.h>
#include <container/tuple.h>
#include <numeric/long.h>

extern "C" {
typedef PyObject *(*PyCArgsFunctionWithKeywords)(PyObject *self,
                                                 PyObject *const *args,
                                                 Py_ssize_t nargs,
                                                 PyObject *kwargs);

/* vector call. */
typedef PyObject *(*PyCFunctionVec)(PyObject *self, PyObject *const *args,
                                    Py_ssize_t nargs);

typedef PyObject *(*PyCFunctionVecWithKeywords)(PyObject *self,
                                                PyObject *const *args,
                                                Py_ssize_t nargs,
                                                PyObject *kwnames);
}

namespace cppbind {

/**
 * Unpack arguments and convert them to C++ types, then call the function.
 *
 * @param RetType the return type of the function to call.
 * @param Elements the argument types of the function to call.
 */
template <size_t Idx, typename RetType, typename... Elements> struct PyArgs;

/**
 * Unpack arguments and convert them to C++ types, then call the function.
 */
template <size_t Idx, typename RetType, typename... Elements>
struct PyVecCallArgs;

/**
 * Base of the ending the recursion of inheritance.
 */
template <size_t Idx, typename RetType, typename Head>
struct PyArgs<Idx, RetType, Head> {
  PyArgs(const Tuple &tp) : tuple(tp) {}

  RetType call(std::function<RetType(Head)> fn) {
    PyObject *obj = tuple[Idx] /* cppbind::Object */.ptr;
    return fn(from<Head>(obj));
  }

  /**
   * Expected argument count.
   */
  virtual size_t size() const { return Idx + 1; }

protected:
  virtual const Tuple &get_args() const { return tuple; }

  const Tuple &tuple;
};

/**
 * Base of the ending the recursion of inheritance.
 */
template <size_t Idx, typename RetType, typename Head>
struct PyVecCallArgs<Idx, RetType, Head> {
  PyVecCallArgs(PyObject *const *args, Py_ssize_t nargs)
      : args(args), nargs(nargs) {}

  RetType call(std::function<RetType(Head)> fn) {
    cppbind_check_internal(Idx < static_cast<size_t>(nargs));
    PyObject *obj = args[Idx];
    return fn(from<Head>(obj));
  }

  virtual size_t size() const { return Idx + 1; }

protected:
  PyObject *const *args;
  Py_ssize_t nargs;
};

/**
 * Recursively unpack arguments and convert them to C++ types.
 */
template <size_t Idx, typename RetType, typename Head, typename... RestElements>
struct PyArgs<Idx, RetType, Head, RestElements...>
    : public PyArgs<Idx + 1, RetType, RestElements...> {

  PyArgs<Idx, RetType, Head, RestElements...>(const Tuple &tp)
      : PyArgs<Idx + 1, RetType, RestElements...>(tp) {}

  RetType call(std::function<RetType(Head, RestElements...)> fn) {
    auto lower_fn = [&](RestElements... args) {
      const auto &tuple = get_args();
      return fn(from<Head>(tuple[Idx].ptr), args...);
    };

    return PyArgs<Idx + 1, RetType, RestElements...>::call(lower_fn);
  }

  size_t size() const override {
    return PyArgs<Idx + 1, RetType, RestElements...>::size();
  }

  const Tuple &get_args() const override {
    return PyArgs<Idx + 1, RetType, RestElements...>::get_args();
  }
};

/**
 * Recursively unpack arguments and convert them to C++ types.
 */
template <size_t Idx, typename RetType, typename Head, typename... RestElements>
struct PyVecCallArgs<Idx, RetType, Head, RestElements...>
    : public PyVecCallArgs<Idx + 1, RetType, RestElements...> {

  PyVecCallArgs<Idx, RetType, Head, RestElements...>(PyObject *const *args,
                                                     Py_ssize_t nargs)
      : PyVecCallArgs<Idx + 1, RetType, RestElements...>(args, nargs) {}

  RetType call(std::function<RetType(Head, RestElements...)> fn) {
    auto lower_fn = [&](RestElements... args) {
      return fn(from<Head>(this->args[Idx]), args...);
    };

    return PyVecCallArgs<Idx + 1, RetType, RestElements...>::call(lower_fn);
  }

  size_t size() const override {
    return PyVecCallArgs<Idx + 1, RetType, RestElements...>::size();
  }
};

} /* namespace cppbind */

#endif /* __PACKAGE_INVOKE_H__ */
