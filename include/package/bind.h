#ifndef __PACKAGE_BIND_H__
#define __PACKAGE_BIND_H__ (1)

#include <functional>
#include <sstream>
#include <type_traits>

#include "common.h"
#include "package/func.h"
#include "package/mod.h"
#include "package/type.h"

/**
 * Synthesize bindings for C++ classes
 * without modifying their definitions
 *
 * (EXPERIMENTAL since 1bc529c).
 */
namespace cppbind {

/**
 * Construct a class to be exported to Python, without modifying the class
 * definition.
 */
template <typename CppClass> struct CppObject {
  /**
   * Type of the payload -- the C++ class to be wrapped.
   */
  using payload_t = CppClass;

  /**
   * Illustration of the layout of the Python object for a C++ class `CppClass`.
   */
  using layout_t = struct {
    /**
     * Head: a PyObject.
     */
    PyObject pyobj;

    /**
     * Rest: the C++ object, a.k.a. the payload.
     */
    payload_t payload;
  };

  /**
   * @return the pointer to the payload of the Python object.
   */
  static inline payload_t *get_payload(PyObject *obj) {
    return &(reinterpret_cast<layout_t *>(obj)->payload);
  }

  /**
   * @return the Python type object for the C++ class. It is initialized in
   * `cpp_type_init`.
   */
  static PyTypeObject *py_type() {
    return cppbind::Type<CppObject<CppClass>>::instance;
  }
};

/**
 * Specialization for void type.
 */
template <> struct CppObject<void> {

  /**
   * Type of the payload -- the C++ class to be wrapped.
   */
  using payload_t = void;

  /**
   * Illustration of the layout of the Python object for a C++ class `CppClass`.
   */
  using layout_t = struct {
    /**
     * Head: a PyObject.
     */
    PyObject pyobj;
  };

  /**
   * @return a non-null pointer but should not be read/written to,
   * since the payload type is `void` (zero-sized).
   */
  static inline payload_t *get_payload(PyObject *obj) {
    return reinterpret_cast<payload_t *>(obj + 1);
  }

  /**
   * @return None type in python.
   */
  static PyTypeObject *py_type() {
    return reinterpret_cast<PyTypeObject *>(Py_NewRef(Py_None->ob_type));
  }
};

/**
 * Staticize a method of a C++ class. It will generate a wrapper function that
 * can be called from Python. We use a `py_method` to create alias,
 * and handle C++ method overloading by generating different staticized methods
 * for different overloads.
 *
 * @param cpp_class The C++ class that the method belongs to.
 * @param method The name of the method.
 * @param py_method method name in Python (identifier).
 * @param ret_type The return type of the method.
 * @param ... The argument types of the method.
 */
#define staticize_method(cpp_class, method, py_method, ret_type, ...)          \
  template <typename CppTy, typename RetTy, typename... Args>                  \
  static ::std::function<RetTy(Args...)> synthesize_##py_method(               \
      CppTy &instance) {                                                       \
    return [&instance](Args... args) -> RetTy {                                \
      return instance.method(args...);                                         \
    };                                                                         \
  }                                                                            \
  static ret_type method_raw_##py_method(PyObject *self, PyObject *args,       \
                                         PyObject *kwargs) {                   \
    cpp_class &payload = *get_payload(self);                                   \
    ::std::function<ret_type(__VA_ARGS__)> fn =                                \
        synthesize_##py_method<cpp_class, ret_type, ##__VA_ARGS__>(payload);   \
    ::cppbind::Tuple tuple = ::cppbind::Tuple::from_args(args);                \
    ::cppbind::PyArgs<0, ret_type, ##__VA_ARGS__> pyargs(tuple);               \
    return pyargs.call(fn);                                                    \
  }                                                                            \
  static PyObject *method_##py_method(PyObject *self, PyObject *args,          \
                                      PyObject *kwargs) {                      \
    auto *ret = call_method_and_into<ret_type>(self, args, kwargs,             \
                                               &method_raw_##py_method);       \
    return ret;                                                                \
  }

/**
 * Staticize a destructor of a C++ class. It will generate a wrapper function
 * that can be called from Python.
 */
#define staticize_destructor(cpp_class)                                        \
  static void del(PyObject *self) {                                            \
    cpp_class *cppobj = get_payload(self);                                     \
    cppobj->~cpp_class();                                                      \
  }

/**
 * Generate a staticized method.
 *
 * @param method The name of the method.
 * @param py_method method name in Python (identifier).
 * @param ret_type The return type of the method.
 * @param ... The argument types of the method.
 */
#define cpp_class_wrapper_impl(method, py_method, ret_type, ...)               \
  staticize_method(payload_t, method, py_method, ret_type, ##__VA_ARGS__)

/**
 * Generate a method table entry for a method of a C++ class.
 *
 * @param method The name of the method.
 * @param py_method method name in Python (identifier).
 * @param ret_type The return type of the method.
 * @param ... The argument types of the method.
 */
#define cpp_class_create_method_tbl_entry(method, py_method, ret_type, ...)    \
  ::cppbind::MethodTableEntry(                                                 \
      #py_method,                                                              \
      [](PyObject *self) -> ::cppbind::MethodWrapper<                          \
                             ::cppbind::MethodTableEntry::method_t> * {        \
        auto *ret =                                                            \
            ::cppbind::MethodWrapper<::cppbind::MethodTableEntry::method_t>::  \
                create_instance(self, ::cppbind::CppObject<payload_t>::CONCAT( \
                                          method_, py_method));                \
        return reinterpret_cast<::cppbind::MethodWrapper<                      \
            ::cppbind::MethodTableEntry::method_t> *>(ret);                    \
      }),

/**
 * Common member types and functions for `CppObject<cpp_class>`.
 * Starting from these, user can simply use `cpp_class_wrapper`, or
 * define custom member functions.
 *
 * @see staticize_method, staticize_destructor, cpp_class_wrapper_impl
 */
#define cpp_class_wrapper_common(cpp_class)                                    \
  using payload_t = cpp_class;                                                 \
  using layout_t = struct {                                                    \
    PyObject pyobj;                                                            \
    cpp_class payload;                                                         \
  };                                                                           \
  static inline payload_t *get_payload(PyObject *obj) {                        \
    return &(reinterpret_cast<layout_t *>(obj)->payload);                      \
  }                                                                            \
  template <typename RetTy, ::std::__enable_if_t<(!is_void_ty<RetTy>()) &&     \
                                                     is_copyable_ty<RetTy>(),  \
                                                 bool> = true>                 \
  static inline PyObject *call_method_and_into(                                \
      PyObject *self, PyObject *args, PyObject *kwargs,                        \
      RetTy (*fn)(PyObject *, PyObject *, PyObject *)) {                       \
    auto ret = fn(self, args, kwargs);                                         \
    return into<RetTy>(ret).unwrap();                                          \
  }                                                                            \
  template <typename RetTy,                                                    \
            ::std::__enable_if_t<                                              \
                (!is_void_ty<RetTy>()) && (!is_copyable_ty<RetTy>()), bool> =  \
                true>                                                          \
  static inline PyObject *call_method_and_into(                                \
      PyObject *self, PyObject *args, PyObject *kwargs,                        \
      RetTy (*fn)(PyObject *, PyObject *, PyObject *)) {                       \
    auto ret = fn(self, args, kwargs);                                         \
    return into<RetTy &&>(::std::move(ret)).unwrap();                          \
  }                                                                            \
  template <typename RetTy,                                                    \
            ::std::__enable_if_t<is_void_ty<RetTy>(), bool> = true>            \
  static inline PyObject *call_method_and_into(                                \
      PyObject *self, PyObject *args, PyObject *kwargs,                        \
      void (*fn)(PyObject *, PyObject *, PyObject *)) {                        \
    fn(self, args, kwargs);                                                    \
    Py_RETURN_NONE;                                                            \
  }

/**
 * Specialization of CppObject for `cpp_class`.
 *
 * This can:
 * <ul>
 *   <li>Generate a unique name for C++ class wrapper
 * (CppObject<cpp_class>);</li> <li>Generate a unique name for a staticized
 * method, e.g. std::vector&lt;int&gt; and std::vector&lt;long&gt; both possess
 * the method <i>push_back</i>. The staticized methods for them are
 * CppObject&lt;std::vector&lt;int&gt;&gt;::method_push_back and
 *     CppObject&lt;std::vector&lt;long&gt;&gt;::method_push_back,
 * respectively;</li>
 * </ul>
 */
#define cpp_class_wrapper(cpp_class, foreach_method)                           \
  template <> struct CppObject<cpp_class> {                                    \
    cpp_class_wrapper_common(cpp_class) staticize_destructor(payload_t)        \
        foreach_method(cpp_class_wrapper_impl)                                 \
  }

/**
 * Synthesize a constructor of a C++ class. It will generate a wrapper function.
 */
template <typename CppClass, typename... Args>
::std::function<void(Args...)> synthesize_constructor(PyObject *buf) {
  auto *ptr = CppObject<CppClass>::get_payload(buf);
  return [ptr](Args... args) -> void { new (ptr) CppClass(args...); };
}

/**
 * Staticize a constructor of a C++ class. It will generate a wrapper function
 * that can be called from Python. We use fastcall `METH_FASTCALL` to avoid the
 * overhead of creating a tuple for arguments.
 */
template <typename CppClass, typename... Args>
PyObject *staticize_constructor(PyObject *self, PyObject *const *args,
                                Py_ssize_t nargs) {
  if (nargs != sizeof...(Args)) {
    PyErr_SetString(PyExc_TypeError, "invalid number of arguments");
    return nullptr;
  }
  PyObject *obj = _PyObject_New(Type<CppObject<CppClass>>::instance);
  if (obj == nullptr) {
    PyErr_SetString(PyExc_RuntimeError, "failed to create object");
    return nullptr;
  }
  auto static_constructor = synthesize_constructor<CppClass, Args...>(obj);

  PyVecCallArgs<0, void, Args...> pyargs(args, nargs);
  pyargs.call(static_constructor);
  return obj;
}

/**
 * Check implementation of
 * <blockquote><pre>
 * std::ostream &operator<<(std::ostream &, const Ty &);
 * </pre></blockquote>
 */
template <typename _Tp> constexpr bool has_repr_impl(...) { return false; }

/**
 * Check implementation of
 * <blockquote><pre>
 * std::ostream &operator<<(std::ostream &, const Ty &);
 * </pre></blockquote>
 */
template <typename _Tp>
constexpr auto
has_repr_impl(int) -> decltype(std::declval<std::ostringstream &>()
                                   << std::declval<const _Tp &>(),
                               true) {
  return true;
}

/**
 * Check implementation of
 * <blockquote><pre>
 * std::ostream &operator<<(std::ostream &, const Ty &);
 * </pre></blockquote>
 */
template <typename _Tp> constexpr bool has_repr() {
  return has_repr_impl<_Tp>(0);
}

/**
 * @return synthesized python repr function.
 */
template <typename _Tp, std::__enable_if_t<!has_repr<_Tp>(), bool> = true>
inline constexpr unaryfunc synthesize_repr(void) {
  return nullptr;
}

/**
 * @return synthesized python repr function.
 */
template <typename _Tp, std::__enable_if_t<has_repr<_Tp>(), bool> = true>
inline constexpr unaryfunc synthesize_repr(void) {
  return [](PyObject *self) -> PyObject * {
    const _Tp &payload = *CppObject<_Tp>::get_payload(self);
    std::string repr_str;
    {
      std::ostringstream oss;
      oss << payload;
      repr_str = oss.str();
    }
    auto *ret = PyUnicode_FromString(repr_str.c_str());
    if (ret == nullptr && !PyErr_Occurred()) {
      PyErr_SetString(PyExc_RuntimeError, "failed to create string object");
    }
    return ret;
  };
}

#define has_cast_to(CppTy)                                                     \
  template <typename _Tp> constexpr bool has_cast_to_##CppTy##_impl(...) {     \
    return false;                                                              \
  }                                                                            \
  template <typename _Tp>                                                      \
  constexpr auto has_cast_to_##CppTy##_impl(int)                               \
      ->decltype(static_cast<CppTy>(std::declval<_Tp>()), true) {              \
    return true;                                                               \
  }                                                                            \
  template <typename _Tp> constexpr bool has_cast_to_##CppTy() {               \
    return has_cast_to_##CppTy##_impl<_Tp>(0);                                 \
  }

has_cast_to(long) has_cast_to(double)

#define synthesize_cast_to(CppTy)                                              \
  template <typename _Tp,                                                      \
            std::__enable_if_t<!has_cast_to_##CppTy<_Tp>(), bool> = true>      \
  inline constexpr unaryfunc synthesize_cast_to_##CppTy(void) {                \
    return nullptr;                                                            \
  }                                                                            \
  template <typename _Tp,                                                      \
            std::__enable_if_t<has_cast_to_##CppTy<_Tp>(), bool> = true>       \
  inline constexpr unaryfunc synthesize_cast_to_##CppTy(void) {                \
    return [](PyObject *a) -> PyObject * {                                     \
      _Tp &payload = *CppObject<_Tp>::get_payload(a);                          \
      CppTy result = static_cast<CppTy>(payload);                              \
      return into(result).unwrap();                                            \
    };                                                                         \
  }

    synthesize_cast_to(long) synthesize_cast_to(double)
#undef has_cast_to
#undef synthesize_cast_to

#define has_binary_operator(op, opname)                                        \
  template <typename _Tp> constexpr bool has_##opname##_impl(...) {            \
    return false;                                                              \
  }                                                                            \
  template <typename _Tp>                                                      \
  constexpr auto has_##opname##_impl(int)                                      \
      ->decltype(::std::declval<_Tp>() op ::std::declval<_Tp>(), true) {       \
    return true;                                                               \
  }                                                                            \
  template <typename _Tp> constexpr bool has_##opname() {                      \
    return has_##opname##_impl<_Tp>(0);                                        \
  }

/**
 * Enumeration of all binary operators in C++.
 */
#define cpp_all_binary_operators(X)                                            \
  X(+, add)                                                                    \
  X(-, subtract)                                                               \
  X(*, multiply)                                                               \
  X(/, div)                                                                    \
  X(%, remainder)                                                              \
  X(^, xor) X(&, and) X(|, or) X(>>, rshift) X(<<, lshift) X(==, eq) X(!=, ne) \
      X(<, lt) X(<=, le) X(>, gt) X(>=, ge)

        cpp_all_binary_operators(has_binary_operator);
#undef cpp_all_binary_operators

template <
    typename _Tp,
    std::__enable_if_t<!has_eq<_Tp>() || (!is_copyable_ty<_Tp>()), bool> = true>
inline constexpr richcmpfunc synthesize_richcmp(void) {
  return nullptr;
}

/**
 * For types that has equality but not partial order.
 */
template <typename _Tp, std::__enable_if_t<has_eq<_Tp>() && (!has_lt<_Tp>()) &&
                                               is_copyable_ty<_Tp>(),
                                           bool> = true>
inline constexpr richcmpfunc synthesize_richcmp(void) {
  return [](PyObject *a, PyObject *b, int op) -> PyObject * {
    if (op != Py_EQ && op != Py_NE) {
      PyErr_SetString(PyExc_TypeError, "unorderable types: " STR(_Tp) "()");
      return nullptr;
    }
    const _Tp &lhs = *CppObject<_Tp>::get_payload(a);
    bool is_eq;
    if (b->ob_type == a->ob_type) {
      const _Tp &rhs = *CppObject<_Tp>::get_payload(b);
      is_eq = (lhs == rhs);
    } else {
      _Tp rhs = from<_Tp>(b);
      is_eq = (lhs == rhs);
    }
    bool res = (op == Py_EQ) ? is_eq : (!is_eq);
    return Py_NewRef(res ? Py_True : Py_False); /* return a new reference. */
  };
}

/**
 * For types that has partial order.
 */
template <typename _Tp, std::__enable_if_t<has_eq<_Tp>() && has_lt<_Tp>() &&
                                               is_copyable_ty<_Tp>(),
                                           bool> = true>
inline constexpr richcmpfunc synthesize_richcmp(void) {
  return [](PyObject *a, PyObject *b, int op) -> PyObject * {
    const _Tp &lhs = *CppObject<_Tp>::get_payload(a);
    bool res;
    if (b->ob_type == a->ob_type) {
      const _Tp &rhs = *CppObject<_Tp>::get_payload(b);
#define perform_richcmp                                                        \
  do {                                                                         \
    switch (op) {                                                              \
    case Py_EQ: {                                                              \
      res = (lhs == rhs);                                                      \
      break;                                                                   \
    }                                                                          \
    case Py_NE: {                                                              \
      res = (lhs != rhs);                                                      \
      break;                                                                   \
    }                                                                          \
    case Py_LT: {                                                              \
      res = (lhs < rhs);                                                       \
      break;                                                                   \
    }                                                                          \
    case Py_LE: {                                                              \
      res = (lhs <= rhs);                                                      \
      break;                                                                   \
    }                                                                          \
    case Py_GT: {                                                              \
      res = (lhs > rhs);                                                       \
      break;                                                                   \
    }                                                                          \
    case Py_GE: {                                                              \
      res = (lhs >= rhs);                                                      \
      break;                                                                   \
    }                                                                          \
    default: {                                                                 \
      PyErr_SetString(PyExc_TypeError, "invalid comparison operator");         \
      return nullptr;                                                          \
    }                                                                          \
    }                                                                          \
  } while (0)

      perform_richcmp;
    } else {
      _Tp rhs = from<_Tp>(b);
      perform_richcmp;
    }

    return Py_NewRef(res ? Py_True : Py_False); /* return a new reference. */
  };
#undef perform_richcmp
}

/**
 * In C++, we do not differentiate true divide and floor divide.
 * The user should set them correctly. Normally,
 * integer types have both true divide (resulting in float) and floor divide
 * (resulting in int), while floating point types only have true divide.
 *
 * @see cppbind::is_integer_ty, cppbind::is_fp_ty
 */
template <typename _Tp, std::__enable_if_t<!has_div<_Tp>(), bool> = true>
inline constexpr binaryfunc synthesize_divide(void) {
  return nullptr;
}

template <typename _Tp, std::__enable_if_t<has_div<_Tp>(), bool> = true>
inline constexpr binaryfunc synthesize_divide(void) {
  return [](PyObject *a, PyObject *b) -> PyObject * {
    PyObject *ret = _PyObject_New(Type<CppObject<_Tp>>::instance);
    if (ret == nullptr) {
      PyErr_SetString(PyExc_RuntimeError, "failed to create result object");
      return nullptr;
    }

    _Tp &lhs = *CppObject<_Tp>::get_payload(a);
    _Tp *result_payload = CppObject<_Tp>::get_payload(ret);
    if (b->ob_type == a->ob_type) {
      _Tp &rhs = *CppObject<_Tp>::get_payload(b);
      new (result_payload) _Tp(lhs / rhs);
      return ret;
    }

    try {
      _Tp rhs = from<_Tp>(b);
      new (result_payload) _Tp(lhs / rhs);
      return ret;
    } catch (std::invalid_argument &) {
      PyErr_SetString(PyExc_TypeError, "unsupported operand type(s) for /");
      return nullptr;
    }
  };
}

/**
 * Inplace divide.
 */
template <typename _Tp, std::__enable_if_t<!has_div<_Tp>(), bool> = true>
inline constexpr binaryfunc synthesize_inplace_divide(void) {
  return nullptr;
}

template <typename _Tp, std::__enable_if_t<has_div<_Tp>(), bool> = true>
inline constexpr binaryfunc synthesize_inplace_divide(void) {
  return [](PyObject *a, PyObject *b) -> PyObject * {
    Py_INCREF(a); /* in-place operator should return the first operand. */
    _Tp &lhs = *CppObject<_Tp>::get_payload(a);
    if (b->ob_type == a->ob_type) {
      _Tp &rhs = *CppObject<_Tp>::get_payload(b);
      lhs /= rhs;
      return a;
    }

    try {
      _Tp rhs = from<_Tp>(b);
      lhs /= rhs;
      return a;
    } catch (std::invalid_argument &) {
      PyErr_SetString(PyExc_TypeError, "unsupported operand type(s) for /=");
      return nullptr;
    }
  };
}

/**
 * Synthesize any other binary operator. `div` is specially documented.
 */
#define __foreach(X)                                                           \
  X(+, add)                                                                    \
  X(-, subtract)                                                               \
  X(*, multiply)                                                               \
  X(%, remainder) X(^, xor) X(&, and) X(|, or) X(>>, rshift) X(<<, lshift)

#define __synthesize(op, opname)                                               \
  template <typename _Tp,                                                      \
            std::__enable_if_t<!has_##opname<_Tp>(), bool> = true>             \
  inline constexpr binaryfunc synthesize_##opname(void) {                      \
    return nullptr;                                                            \
  }                                                                            \
  template <typename _Tp,                                                      \
            std::__enable_if_t<has_##opname<_Tp>(), bool> = true>              \
  inline constexpr binaryfunc synthesize_##opname(void) {                      \
    return [](PyObject *a, PyObject *b) -> PyObject * {                        \
      PyObject *ret = _PyObject_New(Type<CppObject<_Tp>>::instance);           \
      if (ret == nullptr) {                                                    \
        PyErr_SetString(PyExc_RuntimeError, "failed to create result object"); \
        return nullptr;                                                        \
      }                                                                        \
      _Tp &lhs = *CppObject<_Tp>::get_payload(a);                              \
      _Tp *result_payload = CppObject<_Tp>::get_payload(ret);                  \
      if (b->ob_type == a->ob_type) {                                          \
        _Tp &rhs = *CppObject<_Tp>::get_payload(b);                            \
        new (result_payload) _Tp(lhs op rhs);                                  \
        return ret;                                                            \
      }                                                                        \
      try {                                                                    \
        _Tp rhs = from<_Tp>(b);                                                \
        new (result_payload) _Tp(lhs op rhs);                                  \
        return ret;                                                            \
      } catch (std::invalid_argument &) {                                      \
        PyErr_SetString(PyExc_TypeError,                                       \
                        "unsupported operand type(s) for " STR(op));           \
        return nullptr;                                                        \
      }                                                                        \
    };                                                                         \
  }

__foreach(__synthesize)
#undef __synthesize
#undef __foreach

/**
 * Unary operators
 */
#define __foreach(X) X(~, invert) X(-, negative) X(+, positive)

#define has_unary_operator(op, opname)                                         \
  template <typename _Tp> constexpr bool has_##opname##_impl(...) {            \
    return false;                                                              \
  }                                                                            \
  template <typename _Tp>                                                      \
  constexpr auto has_##opname##_impl(int)->decltype(op std::declval<_Tp>(),    \
                                                    true) {                    \
    return true;                                                               \
  }                                                                            \
  template <typename _Tp> constexpr bool has_##opname() {                      \
    return has_##opname##_impl<_Tp>(0);                                        \
  }

    __foreach(has_unary_operator);
#undef has_unary_operator

#define __synthesize(op, opname)                                               \
  template <typename _Tp,                                                      \
            std::__enable_if_t<!has_##opname<_Tp>(), bool> = true>             \
  inline constexpr unaryfunc synthesize_##opname(void) {                       \
    return nullptr;                                                            \
  }                                                                            \
  template <typename _Tp,                                                      \
            std::__enable_if_t<has_##opname<_Tp>(), bool> = true>              \
  inline constexpr unaryfunc synthesize_##opname(void) {                       \
    return [](PyObject *a) -> PyObject * {                                     \
      PyObject *ret = _PyObject_New(Type<CppObject<_Tp>>::instance);           \
      if (ret == nullptr) {                                                    \
        PyErr_SetString(PyExc_RuntimeError, "failed to create result object"); \
        return nullptr;                                                        \
      }                                                                        \
      _Tp &operand = *CppObject<_Tp>::get_payload(a);                          \
      _Tp *result_payload = CppObject<_Tp>::get_payload(ret);                  \
      new (result_payload) _Tp(op operand);                                    \
      return ret;                                                              \
    };                                                                         \
  }

__foreach(__synthesize);
#undef __synthesize
#undef __foreach

/**
 * In-place operators.
 */
#define __foreach(X)                                                           \
  X(+=, inplace_add)                                                           \
  X(-=, inplace_subtract)                                                      \
  X(*=, inplace_multiply)                                                      \
  X(%=, inplace_remainder)                                                     \
  X(^=, inplace_xor)                                                           \
  X(&=, inplace_and)                                                           \
  X(|=, inplace_or) X(>>=, inplace_rshift) X(<<=, inplace_lshift)

#define has_inplace_operator(op, opname)                                       \
  template <typename _Tp> constexpr bool has_##opname##_impl(...) {            \
    return false;                                                              \
  }                                                                            \
  template <typename _Tp>                                                      \
  constexpr auto has_##opname##_impl(int)                                      \
      ->decltype(std::declval<_Tp &>() op std::declval<_Tp>(), true) {         \
    return true;                                                               \
  }                                                                            \
  template <typename _Tp> constexpr bool has_##opname() {                      \
    return has_##opname##_impl<_Tp>(0);                                        \
  }

/**
 * Default implementation for in-place operators.
 */
__foreach(has_inplace_operator);
#undef has_inplace_operator
#define __synthesize(op, opname)                                               \
  template <typename _Tp,                                                      \
            std::__enable_if_t<!has_##opname<_Tp>(), bool> = true>             \
  inline constexpr binaryfunc synthesize_##opname(void) {                      \
    return nullptr;                                                            \
  }                                                                            \
  template <typename _Tp,                                                      \
            std::__enable_if_t<has_##opname<_Tp>(), bool> = true>              \
  inline constexpr binaryfunc synthesize_##opname(void) {                      \
    return [](PyObject *a, PyObject *b) -> PyObject * {                        \
      _Tp &lhs = *CppObject<_Tp>::get_payload(a);                              \
      if (b->ob_type == a->ob_type) {                                          \
        _Tp &rhs = *CppObject<_Tp>::get_payload(b);                            \
        lhs op rhs;                                                            \
      } else {                                                                 \
        _Tp rhs = from<_Tp>(b);                                                \
        lhs op rhs;                                                            \
      }                                                                        \
      Py_INCREF(a); /* in-place operator should return the first operand. */   \
      return a;                                                                \
    };                                                                         \
  }

__foreach(__synthesize);
#undef __foreach

/**
 * Check whether the `PyNumberMethods` struct for a C++ type should be
 * initialized. It is initialized if the C++ type has at least one of the binary
 * operators, unary operators. In-place operators are not checked (e.g. there is
 * an absurd type that has += but not +).
 */
template <typename _Tp> constexpr bool cpp_type_has_pynumber(void) {
#define __foreach(X)                                                           \
  X(add)                                                                       \
  X(subtract)                                                                  \
  X(multiply)                                                                  \
  X(remainder)                                                                 \
  X(xor) X(and) X(or) X(rshift) X(lshift) X(invert) X(negative) X(positive)

#define __or(opname) || has_##opname<_Tp>()

  return is_copyable_ty<_Tp>() && (has_div<_Tp>() __foreach(__or));

#undef __or
#undef __foreach
}

/**
 * Initialize a C++ type's number methods, by synthesizing the binary operators,
 * unary operators, and in-place operators.
 *
 * <h3>True Divide and Floor Divide</h3>
 * By default, this will set (inplace) true divide and floor divide both to
 * `synthesize(_inplace)_divide`,  so that in Python, '/(=)' and '//(=)'
 * will have the same behavior.
 */
template <typename _Tp>
void cpp_type_initialize_number(PyNumberMethods *method) {
  static_assert(
      cpp_type_has_pynumber<_Tp>(),
      "cpp_type_initialize_number should only be called for types that "
      "have at least one of the binary/unary operators.");

#define __foreach(X)                                                           \
  X(add)                                                                       \
  X(subtract)                                                                  \
  X(multiply)                                                                  \
  X(remainder)                                                                 \
  X(xor) X(and) X(or) X(rshift) X(lshift) X(invert) X(negative) X(positive)    \
      X(inplace_add) X(inplace_subtract) X(inplace_multiply)                   \
          X(inplace_remainder) X(inplace_xor) X(inplace_and) X(inplace_or)     \
              X(inplace_rshift) X(inplace_lshift)

#define __set_field(opname)                                                    \
  method->CONCAT(nb_, opname) = synthesize_##opname<_Tp>();
  __foreach(__set_field) {
    auto div_fn = synthesize_divide<_Tp>();
    auto inplace_div_fn = synthesize_inplace_divide<_Tp>();
    method->nb_true_divide = div_fn;
    method->nb_floor_divide = div_fn;
    method->nb_inplace_true_divide = inplace_div_fn;
    method->nb_inplace_floor_divide = inplace_div_fn;
    method->nb_index = synthesize_cast_to_long<_Tp>();
    method->nb_int = synthesize_cast_to_long<_Tp>();
    method->nb_float = synthesize_cast_to_double<_Tp>();
  }

#undef __set_field
#undef __foreach
}

/**
 * Do extra initialization of `PyNumberMethods`.
 */
#define cpp_type_init_number(cpp_class)                                        \
  do {                                                                         \
    auto *ty_ob = ::cppbind::Type<::cppbind::CppObject<cpp_class>>::instance;  \
    static PyNumberMethods number_methods;                                     \
    ::cppbind::cpp_type_initialize_number<cpp_class>(&number_methods);         \
    ty_ob->tp_as_number = &number_methods;                                     \
  } while (0)

/**
 * Do initialization for the C++ type, in a {@link cppbind::RestInitFn}.
 *
 * @param package_name the name of the package (string literal)
 * @param cpp_class the C++ class to be exposed.
 * @param pyclass_name the name of the class in python (string literal).
 * @param foreach_method a macro that takes another macro as argument and
 * applies it to all methods of the class. Example of this: <blockquote><pre>
 * // inside X() is (method name, py method name, return type, argument types)
 * // the py method name must be unique among all methods of the class,
 * // but the method name can be overloaded in C++.
 * #define intvec_foreach_method(X)
 *   X(push_back, push_back, void, int)
 *   X(size, size, int)
 * </pre></blockquote>
 */
#define cpp_type_init(package_name, cpp_class, pyclass_name, foreach_method)   \
  do {                                                                         \
    auto *ty_ob = ::cppbind::PyTypeObject_Zero();                              \
    if (ty_ob == nullptr) {                                                    \
      PyErr_SetString(PyExc_RuntimeError, "failed to create type object");     \
      return (1);                                                              \
    }                                                                          \
    ty_ob->tp_name = package_name "." pyclass_name;                            \
    ty_ob->tp_flags = Py_TPFLAGS_DEFAULT;                                      \
    ty_ob->tp_basicsize = sizeof(::cppbind::CppObject<cpp_class>::layout_t);   \
    ty_ob->tp_dealloc = ::cppbind::CppObject<cpp_class>::del;                  \
    ty_ob->tp_repr = ::cppbind::synthesize_repr<cpp_class>();                  \
    ty_ob->tp_richcompare = ::cppbind::synthesize_richcmp<cpp_class>();        \
    ty_ob->tp_getattr =                                                        \
        ::cppbind::Type<::cppbind::CppObject<cpp_class>>::getattr;             \
    ty_ob->tp_setattr =                                                        \
        ::cppbind::Type<::cppbind::CppObject<cpp_class>>::setattr;             \
    ::cppbind::Type<::cppbind::CppObject<cpp_class>>::instance = ty_ob;        \
    using payload_t = ::cppbind::CppObject<cpp_class>::payload_t;              \
    /* pad a dummy method, because GNU g++ does not accept zero-size array. */ \
    static ::cppbind::MethodTableEntry methods[] = {                           \
        MethodTableEntry_dummy(""),                                            \
        foreach_method(cpp_class_create_method_tbl_entry)};                    \
    auto num_methods = sizeof(methods) / sizeof(::cppbind::MethodTableEntry);  \
    ::cppbind::MethodTableEntry *base = methods;                               \
    if (num_methods != 1) {                                                    \
      num_methods--; /* skip the dummy method. */                              \
      base++;        /* skip the dummy method. */                              \
      ::std::sort(base, base + num_methods);                                   \
    }                                                                          \
    ::cppbind::Type<::cppbind::CppObject<cpp_class>>::methods = base;          \
    ::cppbind::Type<::cppbind::CppObject<cpp_class>>::methods_cnt =            \
        num_methods;                                                           \
  } while (0)

#define cpp_class_create_data_member(member, read_only)                        \
  ::cppbind::DataMember{                                                       \
      STR(member), cpp_type_gen_getter(payload_t, member),                     \
      ((read_only) ? nullptr                                                   \
                   : (::cppbind::DataMember::SetterType)cpp_type_gen_setter(   \
                         payload_t, member))},

/**
 * Generate {@link DataMember} array for a C++ class, and initialize the data
 * member related fields in the C++ type object. The `foreach_data_member` is a
 * macro that takes another macro as argument and applies it to all data
 * members of the class.
 *
 * Example of this: <blockquote><pre>
 * // inside X() is (data member name, whether it is read-only)
 * #define point_2d_members(X) X(x, false) X(y, false)
 * </pre></blockquote>
 *
 * <b>Note</b>: this must be called after {@link cpp_type_init}, because it
 * needs the type object to be created and assigned to {@link Type}.
 */
#define cpp_type_init_data_members(cpp_class, foreach_data_member)             \
  do {                                                                         \
    using payload_t = ::cppbind::CppObject<cpp_class>::payload_t;              \
    static ::cppbind::DataMember data_members[] = {                            \
        foreach_data_member(cpp_class_create_data_member)};                    \
    auto num_data_members =                                                    \
        sizeof(data_members) / sizeof(::cppbind::DataMember);                  \
    ::cppbind::DataMember *base = data_members;                                \
    if (num_data_members != 0) {                                               \
      ::std::sort(base, base + num_data_members);                              \
    }                                                                          \
    ::cppbind::Type<::cppbind::CppObject<cpp_class>>::data_members = base;     \
    ::cppbind::Type<::cppbind::CppObject<cpp_class>>::data_members_cnt =       \
        num_data_members;                                                      \
  } while (0)

#define cpp_get_type_object(cpp_class)                                         \
  (::cppbind::Type<::cppbind::CppObject<cpp_class>>::instance)

/**
 * Register base classes for a C++ class. This should be called
 * in a {@link cppbind::RestInitFn} after
 * {@link cpp_type_init}, because it needs the type object to
 * be created and assigned to {@link Type}.
 *
 * Example usage:
 * <blockquote><pre>
 * struct Base { };
 * struct Ext : public Base { };
 *
 * // an X-macro of all the base types.
 * #define foreach_base_ty(X) X(Base)
 * // in a RestInitFn
 * cpp_type_register_bases(Ext, foreach_base_ty)
 * </pre></blockquote>
 */
#define cpp_type_register_bases(cpp_class, foreach_base_ty)                    \
  do {                                                                         \
    auto *ty_obj_this = cpp_get_type_object(cpp_class);                        \
    PyTypeObject *base_ty_objs[] = {foreach_base_ty(cpp_get_type_object)};     \
    auto num_bases = sizeof(base_ty_objs) / sizeof(PyTypeObject *);            \
    if (num_bases > 1) {                                                       \
      /* at least 2 base types. Set tp_bases with a tuple. */                  \
      auto *bases_tuple = PyTuple_New(num_bases);                              \
      if (bases_tuple == nullptr) {                                            \
        PyErr_SetString(PyExc_RuntimeError, "failed to create bases tuple");   \
        return (1);                                                            \
      }                                                                        \
      for (size_t i = 0; i < num_bases; i++) {                                 \
        Py_INCREF(base_ty_objs[i]); /* PyTuple_SetItem steals reference. */    \
        PyTuple_SetItem(bases_tuple, i, (PyObject *)base_ty_objs[i]);          \
      }                                                                        \
      ty_obj_this->tp_bases = bases_tuple;                                     \
    } else {                                                                   \
      /* only one base type. set tp_base instead. */                           \
      Py_INCREF(base_ty_objs[0]); /* tp_base is a strong reference. */         \
      ty_obj_this->tp_base = base_ty_objs[0];                                  \
    }                                                                          \
  } while (0)

} /* namespace cppbind */

#endif /* __PACKAGE_BIND_H__ */
