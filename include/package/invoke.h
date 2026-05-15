#ifndef __PACKAGE_INVOKE_H__
#define __PACKAGE_INVOKE_H__ (1)

#include <functional> /* std::function */
#include <methodobject.h>

#include "cast.h" /* cppbind::from and cppbind::into */
#include "common.h"
#include "container/dict.h"
#include "container/list.h"
#include "container/tuple.h"
#include "numeric/long.h"

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
 * Base class for argument packs. It provides an interface to get the expected
 * argument count and get the argument at a specific index.
 */
/* abstract */ struct ArgumentPack {
  virtual ~ArgumentPack() = default;

  /**
   * @return expected argument count.
   */
  virtual size_t size() const = 0;

  /**
   * @return reference to the argument tuple.
   */
  virtual Object get(size_t idx) const = 0;
};

/**
 * Argument pack for `PyCFunctionVec`.
 */
struct PyVecCallArgPack final : public ArgumentPack {
  /**
   * Constructor.
   */
  PyVecCallArgPack(PyObject *const *args, Py_ssize_t nargs)
      : args(args), nargs(nargs) {}
  ~PyVecCallArgPack() = default;

  /**
   * @return expected argument count.
   */
  size_t size() const override { return static_cast<size_t>(nargs); }

  /**
   * @return reference to the argument tuple.
   */
  Object get(size_t idx) const override {
    cppbind_check_internal(idx < static_cast<size_t>(nargs));
    /* Must increment the refcnt in order to borrow it. */
    auto *ptr = const_cast<PyObject *>(args[idx]);
    Py_INCREF(ptr);
    return Object(ptr);
  }

private:
  const PyObject *const *args;
  const Py_ssize_t nargs;
};

/**
 * Argument pack for `PyCFunction`.
 */
struct PyCallArgPack final : public ArgumentPack {
  /**
   * Constructor.
   */
  PyCallArgPack(const Tuple &tp) : tuple(tp) {}
  ~PyCallArgPack() = default;

  /**
   * @return expected argument count.
   */
  size_t size() const override { return tuple.size(); }

  /**
   * @return reference to the argument tuple.
   */
  Object get(size_t idx) const override {
    cppbind_check_internal(idx < static_cast<size_t>(tuple.size()));
    /**
     * operator[] already incremented the reference count of
     * the returned object, so we can return it directly.
     */
    return tuple[idx];
  }

private:
  const Tuple &tuple;
};

/**
 * Implementation of unpacking arguments and calling the function.
 */
template <size_t Idx, typename Callable, typename RetType,
          typename... RestElements>
struct NativeCallImpl;

/**
 * Unpack one argument, and forward the rest to the next level of recursion.
 */
template <size_t Idx, typename Callable, typename RetType, typename Head,
          typename... RestElements>
struct NativeCallImpl<Idx, Callable, RetType, Head, RestElements...> {
  /**
   * Constructor.
   */
  NativeCallImpl(Callable callable, ArgumentPack &arg_pack)
      : callable(callable), arg_pack(arg_pack) {
    /*
     * Like previous version of Py(Vec)Args,
     * we check number of arguments inside the constructor.
     */
    if (unlikely(arg_pack.size() != sizeof...(RestElements) + 1 + Idx)) {
      throw std::invalid_argument("argument count mismatch");
    }
  }
  ~NativeCallImpl() = default;

  /**
   * Unpack the argument at index `Idx`, convert it to C++ type `Head`, and
   * call the next level of recursion with the rest of the arguments.
   */
  RetType call(void) {
    Object arg = arg_pack.get(Idx);
    auto lower_fn = [this, &arg](RestElements... args) {
      return this->callable(from<Head>(arg.ptr), args...);
    };
    NativeCallImpl<Idx + 1, decltype(lower_fn), RetType, RestElements...>
        lower_impl(lower_fn, arg_pack);
    return lower_impl.call();
  }

private:
  Callable callable;
  ArgumentPack &arg_pack;
};

/**
 * Base of the ending the recursion of inheritance.
 */
template <size_t Idx, typename Callable, typename RetType, typename Arg>
struct NativeCallImpl<Idx, Callable, RetType, Arg> {
  /**
   * Constructor.
   */
  NativeCallImpl(Callable callable, ArgumentPack &arg_pack)
      : callable(callable), arg_pack(arg_pack) {
    if (unlikely(arg_pack.size() != Idx + 1)) {
      throw std::invalid_argument("argument count mismatch");
    }
  }
  ~NativeCallImpl() = default;

  /**
   * Forward the last argument to the callable.
   */
  RetType call(void) {
    Object arg = arg_pack.get(Idx);
    return callable(from<Arg>(arg.ptr));
  }

private:
  Callable callable;
  ArgumentPack &arg_pack;
};

/**
 * Specialization for no argument functions.
 */
template <size_t Idx, typename Callable, typename RetType>
struct NativeCallImpl<Idx, Callable, RetType> {
  /**
   * Constructor.
   */
  NativeCallImpl(Callable callable, ArgumentPack &arg_pack)
      : callable(callable), arg_pack(arg_pack) {
    if (arg_pack.size() != 0) {
      throw std::invalid_argument("argument count mismatch");
    }
  }
  ~NativeCallImpl() = default;

  /**
   * Simply invoke the callable without unpacking any argument.
   */
  RetType call(void) { return callable(); }

private:
  Callable callable;
  ArgumentPack &arg_pack;
};

/**
 * Unpack arguments and convert them to C++ types, then call the function.
 *
 * @param RetType the return type of the function to call.
 * @param Elements the argument types of the function to call.
 */
template <size_t Idx, typename RetType, typename... Elements> struct PyArgs {
  /**
   * Initialization from {@link Tuple}.
   */
  PyArgs(const Tuple &tp) : arg_pack(tp) {}

  /**
   * Forward the arguments using {@link NativeCallImpl}, and get return value.
   */
  RetType call(std::function<RetType(Elements...)> fn) {
    NativeCallImpl<0, decltype(fn), RetType, Elements...> impl(fn, arg_pack);
    return impl.call();
  }

private:
  PyCallArgPack arg_pack;
};

/**
 * Unpack arguments and convert them to C++ types, then call the function.
 */
template <size_t Idx, typename RetType, typename... Elements>
struct PyVecCallArgs {
  /**
   * Initialization from fastcall arguments.
   */
  PyVecCallArgs(PyObject *const *args, Py_ssize_t nargs)
      : arg_pack(args, nargs) {}

  /**
   * Forward the arguments using {@link NativeCallImpl}, and get return value.
   */
  RetType call(std::function<RetType(Elements...)> fn) {
    NativeCallImpl<0, decltype(fn), RetType, Elements...> impl(fn, arg_pack);
    return impl.call();
  }

private:
  PyVecCallArgPack arg_pack;
};

/**
 * Call a fast call function and convert the return value to `PyObject *`.
 * This is specialized for those returning `void` to return `Py_None`.
 */
template <typename RetTy, std::__enable_if_t<is_void_ty<RetTy>(), bool> = true>
inline PyObject *fastcall_and_into(void (*func)(PyObject *, PyObject *const *,
                                                Py_ssize_t),
                                   PyObject *const *args, Py_ssize_t nargs) {
  func(nullptr, args, nargs);
  Py_RETURN_NONE;
}

/**
 * Call a fast call function and convert the return value to `PyObject *`.
 */
template <typename RetTy,
          std::__enable_if_t<(!is_void_ty<RetTy>()) && is_copyable_ty<RetTy>(),
                             bool> = true>
inline PyObject *fastcall_and_into(RetTy (*func)(PyObject *, PyObject *const *,
                                                 Py_ssize_t),
                                   PyObject *const *args, Py_ssize_t nargs) {
  RetTy ret = func(nullptr, args, nargs);
  return into<RetTy>(ret).unwrap();
}

/**
 * `fastcall_and_into` for non-copyable return types, use
 * `std::move` to move the return value.
 */
template <typename RetTy, std::__enable_if_t<(!is_void_ty<RetTy>()) &&
                                                 (!is_copyable_ty<RetTy>()),
                                             bool> = true>
inline PyObject *fastcall_and_into(RetTy (*func)(PyObject *, PyObject *const *,
                                                 Py_ssize_t),
                                   PyObject *const *args, Py_ssize_t nargs) {
  RetTy ret = func(nullptr, args, nargs);
  return into<RetTy &&>(std::move(ret)).unwrap();
}

#define gen_builtin_function(cpp_function, RetType, ...)                       \
  [](PyObject *self, PyObject *const *args, Py_ssize_t nargs) -> PyObject * {  \
    RetType (*func)(PyObject *, PyObject *const *, Py_ssize_t);                \
    func = (RetType(*)(PyObject *, PyObject *const *,                          \
                       Py_ssize_t))([](PyObject *self, PyObject *const *args,  \
                                       Py_ssize_t nargs) -> RetType {          \
      return ::cppbind::PyVecCallArgs<0, RetType, ##__VA_ARGS__>(args, nargs)  \
          .call(cpp_function);                                                 \
    });                                                                        \
    try {                                                                      \
      return ::cppbind::fastcall_and_into<RetType>(func, args, nargs);         \
    } catch (::std::exception & ex) {                                          \
      ::cppbind::PyErr_from_cpp_exception(ex);                                 \
      return nullptr;                                                          \
    }                                                                          \
  }

#define gen_builtin_function_def(cpp_function, func_doc, RetType, ...)         \
  {                                                                            \
    #cpp_function,                                                             \
        (PyCFunction)(PyCFunctionVec)(gen_builtin_function(                    \
            cpp_function, RetType, ##__VA_ARGS__)),                            \
        METH_FASTCALL, func_doc                                                \
  }

} /* namespace cppbind */

#endif /* __PACKAGE_INVOKE_H__ */
