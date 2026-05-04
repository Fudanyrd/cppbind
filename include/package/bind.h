#ifndef __PACKAGE_BIND_H__
#define __PACKAGE_BIND_H__ (1)

#include <functional>
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
        synthesize_##method<cpp_class, ret_type, ##__VA_ARGS__>(payload);      \
    ::cppbind::Tuple tuple = ::cppbind::Tuple::from_args(args);                \
    ::cppbind::PyArgs<0, ret_type, ##__VA_ARGS__> pyargs(tuple);               \
    return pyargs.call(fn);                                                    \
  }                                                                            \
  static PyObject *method_##py_method(PyObject *self, PyObject *args,          \
                                      PyObject *kwargs) {                      \
    auto ret =                                                                 \
        call_method<ret_type>(self, args, kwargs, &method_raw_##method);       \
    return ::cppbind::into<decltype(ret)>(ret).unwrap();                       \
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
    using payload_t = cpp_class;                                               \
    using layout_t = struct {                                                  \
      PyObject pyobj;                                                          \
      cpp_class payload;                                                       \
    };                                                                         \
    static inline payload_t *get_payload(PyObject *obj) {                      \
      return &(reinterpret_cast<layout_t *>(obj)->payload);                    \
    }                                                                          \
    template <typename RetTy,                                                  \
              ::std::__enable_if_t<!is_void_ty<RetTy>(), bool> = true>         \
    static inline RetTy                                                        \
    call_method(PyObject *self, PyObject *args, PyObject *kwargs,              \
                RetTy (*fn)(PyObject *, PyObject *, PyObject *)) {             \
      auto ret = fn(self, args, kwargs);                                       \
      return ret;                                                              \
    }                                                                          \
    template <typename RetTy,                                                  \
              ::std::__enable_if_t<is_void_ty<RetTy>(), bool> = true>          \
    static inline PyObject *                                                   \
    call_method(PyObject *self, PyObject *args, PyObject *kwargs,              \
                void (*fn)(PyObject *, PyObject *, PyObject *)) {              \
      fn(self, args, kwargs);                                                  \
      Py_RETURN_NONE;                                                          \
    }                                                                          \
    staticize_destructor(payload_t) foreach_method(cpp_class_wrapper_impl)     \
  }

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
    ty_ob->tp_getattr =                                                        \
        ::cppbind::Type<::cppbind::CppObject<cpp_class>>::getattr;             \
    ::cppbind::Type<::cppbind::CppObject<cpp_class>>::instance = ty_ob;        \
    using payload_t = ::cppbind::CppObject<cpp_class>::payload_t;              \
    static ::cppbind::MethodTableEntry methods[] = {                           \
        foreach_method(cpp_class_create_method_tbl_entry)};                    \
    ::cppbind::Type<::cppbind::CppObject<cpp_class>>::methods = methods;       \
    ::cppbind::Type<::cppbind::CppObject<cpp_class>>::methods_cnt =            \
        sizeof(methods) / sizeof(::cppbind::MethodTableEntry);                 \
    auto *base = ::cppbind::Type<::cppbind::CppObject<cpp_class>>::methods;    \
    auto *end =                                                                \
        base + ::cppbind::Type<::cppbind::CppObject<cpp_class>>::methods_cnt;  \
    ::std::sort(base, end);                                                    \
  } while (0)

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

} /* namespace cppbind */

#endif /* __PACKAGE_BIND_H__ */
