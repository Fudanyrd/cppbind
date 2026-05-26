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

/**
 * Naming conventions for a tester.
 */
#define invocable_tester(name) InvocableTester

/**
 * Naming conventions for a default argument handler.
 */
#define default_arg_handler(name) DefaultArgHandlerF##name

/**
 * For cppbind internal use only.
 *
 * Given a C++ function, e.g.
 * <blockquote><pre>
 *   long add(long a, long b = 2);
 *   // works with 2 arguments.
 *   static_assert(invocable_tester(add)::get<long, long>(), "");
 *   // works with 1 argument, using default value for the second one.
 *   static_assert(invocable_tester(add)::get<long>(), "");
 * </pre></blockquote>
 * this class detects whether arguments `Args` is compatible with the function.
 * When compatible, a wrapper function `call_impl` is provided to call the
 * function with arguments of type `Args`. When not compatible, `call_impl`
 * will throw an exception.
 */
#define make_invocable_tester(function, RetType)                               \
  struct invocable_tester(function) {                                          \
  private:                                                                     \
    template <typename... Args> static constexpr bool get_impl(...) {          \
      return false;                                                            \
    }                                                                          \
    template <typename... Args>                                                \
    static constexpr auto get_impl(                                            \
        int) -> decltype(function(::std::declval<Args>()...), false) {         \
      return true;                                                             \
    }                                                                          \
                                                                               \
  public:                                                                      \
    template <typename... Args> static constexpr bool get() {                  \
      return get_impl<Args...>(0);                                             \
    }                                                                          \
    template <typename... Args,                                                \
              ::std::__enable_if_t<get_impl<Args...>(0), bool> = true>         \
    static inline auto call_impl(                                              \
        Args... args) -> decltype(function(::std::declval<Args>()...)) {       \
      return function(args...);                                                \
    }                                                                          \
    template <typename... Args,                                                \
              ::std::__enable_if_t<!get_impl<Args...>(0), bool> = true>        \
    static inline RetType call_impl(...) {                                     \
      /* bad: not invocable with the given arguments. */                       \
      throw ::std::invalid_argument("argument count mismatch");                \
    }                                                                          \
  }

/**
 * For cppbind internal use only.
 *
 * It generates definition for a template class `TypePack`. It is used
 * to truncate a template argument pack (e.g. &lt;long, long, long&gt;
 * to &lt;long&gt;.
 *
 * Caveats: instead of writing TypePack&lt;void&gt;, one should
 * use TypePack&lt;&gt; to represent an empty type pack.
 */
#define _make_type_pack(function, ThisRetType)                                 \
  template <typename... Args> struct TypePack;                                 \
  template <typename... Args1, typename... Args2>                              \
  TypePack<Args1..., Args2...> static type_pack_cat(TypePack<Args1...>,        \
                                                    TypePack<Args2...>);       \
  template <typename T> static TypePack<T> make_type_pack(T);                  \
  template <typename... Args> struct TypePack {                                \
    using ArgumentPack = ::cppbind::ArgumentPack;                              \
    TypePack(void);                                                            \
    using RetType = ThisRetType;                                               \
    using CallableType = RetType (*)(Args...);                                 \
    static constexpr bool invocable() {                                        \
      return invocable_tester(function)::get<Args...>();                       \
    }                                                                          \
    static auto call(Args... args) -> RetType {                                \
      return invocable_tester(function)::call_impl(args...);                   \
    }                                                                          \
    static auto call_packed(ArgumentPack &pack) -> RetType {                   \
      using CallImpl =                                                         \
          ::cppbind::NativeCallImpl<0, decltype(&call), RetType, Args...>;     \
      CallImpl impl(&call, pack);                                              \
      return impl.call();                                                      \
    }                                                                          \
    static constexpr int size() { return sizeof...(Args); }                    \
  };

/**
 * For cppbind internal use only.
 *
 * It generates a TypePackSlice template class. Its function is to implement
 * truncating template pack. Its arguments:
 * <ul>
 *  <li><i>Idx</i>: it is always 0.</li>
 *  <li><i>LastIdx</i>: the index of the last element to keep. It should be
 *      less than or equal to the size of the original template pack. </li>
 * <li><i>Args</i>: the original template pack.</li>
 * </ul>
 */
#define make_type_pack_slice(function, RetType, ...)                           \
  template <int Idx, int LastIdx, typename... Args> struct TypePackSlice;      \
  template <int Idx, int LastIdx, typename Head, typename... Args>             \
  struct TypePackSlice<Idx, LastIdx, Head, Args...>                            \
      : public TypePackSlice<Idx + 1, LastIdx, Args...> {                      \
    static_assert(LastIdx <= sizeof...(Args) + Idx,                            \
                  "LastIdx should be less than sizeof...(Args)");              \
    using SubTy = TypePackSlice<Idx + 1, LastIdx, Args...>;                    \
    constexpr TypePackSlice() : SubTy() {}                                     \
    using NormRetType =                                                        \
        decltype(type_pack_cat(make_type_pack<Head>(std::declval<Head>()),     \
                               std::declval<SubTy>().norm()));                 \
    constexpr NormRetType norm() const;                                        \
  };                                                                           \
  template <int LastIdx, typename Head, typename... Args>                      \
  struct TypePackSlice<LastIdx, LastIdx, Head, Args...> {                      \
    static_assert(LastIdx <= sizeof...(Args) + LastIdx,                        \
                  "LastIdx should be less than sizeof...(Args)");              \
    constexpr TypePackSlice() {}                                               \
    constexpr TypePack<Head> norm() const;                                     \
    using NormRetType = TypePack<Head>;                                        \
  }

/**
 * For cppbind internal use only.
 *
 * C++ 11 does not allow us to use a loop variable to instantiate template
 * class `TypePackSlice`. We instead use `Range` to
 * instantiate `TypePackSlice`s, and then  forward a argument
 * pack based on its size.
 */
#define make_range(function, ThisRetType, ...)                                 \
  template <int Idx, int End, typename... Args> struct Range;                  \
  template <int Idx, int End, typename Head, typename... Args>                 \
  struct Range<Idx, End, Head, Args...> {                                      \
    using TypePackType =                                                       \
        decltype(std::declval<TypePackSlice<0, Idx, ##__VA_ARGS__>>().norm()); \
    ThisRetType operator()(::cppbind::ArgumentPack &pack) const {              \
      if (Idx + 1 == pack.size()) {                                            \
        return TypePackType::call_packed(pack);                                \
      }                                                                        \
      Range<Idx + 1, End, Args...> next_rng;                                   \
      return next_rng(pack);                                                   \
    }                                                                          \
  };                                                                           \
  template <int End> struct Range<End, End> {                                  \
    ThisRetType operator()(::cppbind::ArgumentPack &pack) const {              \
      throw std::invalid_argument("argument count mismatch");                  \
    }                                                                          \
  }

/**
 * It is used to do preparation for binding a C++ function, by generating
 * a helper class named `make_default_arg_handler(function)`.
 *
 * {@link NativeCallImpl} does not handle default argument(s).
 *
 * @param function: the C++ function to bind.
 * @param ThisRetType: the return type of the function.
 * @param ...: the parameter list of the function. Default arguments should be
 *             put at the end of the parameter list.
 *
 * Note: this should be used in the same namespace as the C++ function.
 * i.e. the following code does not work:
 * <blockquote><pre>
 *   namespace mypkg { long add2(long a, long b = 42); }
 *   // incorrectly expanded to struct DefaultArgHandlerFmypkg::add2
 *   make_default_arg_handler(mypkg::add2, long, long, long);
 *
 *  // "no function named add2 in the global namespace" error.
 *   make_default_arg_handler(add2, long, long, long);
 * </pre></blockquote>
 */
#define make_default_arg_handler(function, ThisRetType, ...)                   \
  struct default_arg_handler(function) {                                       \
  private:                                                                     \
    template <typename... Args> struct Config {                                \
      static constexpr int begin() { return 0; }                               \
      static constexpr int end() { return sizeof...(Args); }                   \
    };                                                                         \
    make_invocable_tester(function, ThisRetType);                              \
    _make_type_pack(function, ThisRetType);                                    \
    make_type_pack_slice(function, ThisRetType, ##__VA_ARGS__);                \
    make_range(function, ThisRetType, ##__VA_ARGS__);                          \
                                                                               \
    using ArgumentPackType = ::cppbind::ArgumentPack;                          \
    using ConfigType = Config<__VA_ARGS__>;                                    \
                                                                               \
  public:                                                                      \
    static ThisRetType call(ArgumentPackType &pack) {                          \
      if (pack.size() == 0) {                                                  \
        return TypePack<>::call_packed(pack);                                  \
      }                                                                        \
      Range<ConfigType::begin(), ConfigType::end(), ##__VA_ARGS__> rng;        \
      return rng(pack);                                                        \
    }                                                                          \
  }

#define gen_default_arg_builtin_function(cpp_function, RetType, ...)           \
  [](PyObject *self, PyObject *const *args, Py_ssize_t nargs) -> PyObject * {  \
    try {                                                                      \
      return ::cppbind::fastcall_and_into<RetType>(                            \
          [](PyObject *, PyObject *const *args, Py_ssize_t nargs) -> RetType { \
            default_arg_handler(cpp_function) handler;                         \
            auto pack = ::cppbind::PyVecCallArgPack(args, nargs);              \
            return handler.call(pack);                                         \
          },                                                                   \
          args, nargs);                                                        \
    } catch (::std::exception & ex) {                                          \
      ::cppbind::PyErr_from_cpp_exception(ex);                                 \
      return nullptr;                                                          \
    }                                                                          \
  }

/**
 * Generate a `PyMethodDef` for a C++ function with default arguments.
 *
 * <h3>Note of dealing with namespaces</h3>
 * Consider the function `mypkg::add` below:
 * <blockquote><pre>
 *   namespace mypkg { long add2(long a, long b = 42); }
 * </pre></blockquote>
 *
 * To make it callable as `def add2(a: int, b: int = 42)` in python3,
 * one should do:
 * <blockquote><pre>
 *   namespace mypkg { make_default_arg_handler(add2, long, long, long); }
 *
 *   // make the helper class "escape" from the namespace.
 *   using mypkg :: default_arg_handler(add2);
 *   gen_default_arg_builtin_function_def(add2,
 *       "add two numbers", long, long, long);
 * </pre></blockquote>
 */
#define gen_default_arg_builtin_function_def(cpp_function, func_doc, RetType,  \
                                             ...)                              \
  {                                                                            \
    #cpp_function,                                                             \
        (PyCFunction)(PyCFunctionVec)(gen_default_arg_builtin_function(        \
            cpp_function, RetType, ##__VA_ARGS__)),                            \
        METH_FASTCALL, func_doc                                                \
  }

} /* namespace cppbind */

#endif /* __PACKAGE_INVOKE_H__ */
