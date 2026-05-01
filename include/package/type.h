#ifndef __PACKAGE_TYPE_H__
#define __PACKAGE_TYPE_H__ (1)

#include <cstdio>
#include <cstring>

#include <common.h>
#include <object.h>
#include <package/func.h>
#include <traceback.h>

namespace cppbind {

static inline PyTypeObject *PyTypeObject_Zero() {
  PyTypeObject *ret = reinterpret_cast<PyTypeObject *>(
      _PyObject_New((PyTypeObject *)&PyType_Type));
  memset(&(ret->tp_name), 0, sizeof(*ret) - offsetof(PyTypeObject, tp_name));
  return ret;
}

/**
 * The method table entry for a type. It contains the method name and a
 * generator of {@link MethodWrapper}. The generator takes the self pointer as
 * argument and returns a {@link MethodWrapper}, which is a callable object that
 * wraps the actual method.
 */
struct MethodTableEntry {
public:
  /**
   * Python method type.
   */
  typedef PyObject *(*method_t)(PyObject *self, PyObject *args,
                                PyObject *kwargs);
  /**
   * {@link MethodWrapper} generator type.
   */
  typedef MethodWrapper<method_t> *(*gen_t)(PyObject *self);

  /**
   * The name of the method. It should be a string literal.
   */
  const char *name; /* static allocated string */

  /**
   * A generator of {@link MethodWrapper}.
   */
  gen_t gen; /* generator of method wrapper, takes self as argument. */

  /**
   * @param name the name of the method. It should be a string literal.
   * @param gen the generator of {@link MethodWrapper}.
   */
  MethodTableEntry(const char *name, gen_t gen) : name(name), gen(gen) {}

#define MethodTableEntry_build(package_name, cpp_class, method, meth_wrapper)  \
  ::cppbind::MethodTableEntry(                                                 \
      STR(method),                                                             \
      (::cppbind::MethodTableEntry::gen_t)(                                    \
          [](PyObject *self) -> ::cppbind::MethodWrapper<                      \
                                 ::cppbind::MethodTableEntry::method_t> * {    \
            auto *ret = ::cppbind::MethodWrapper<                              \
                ::cppbind::MethodTableEntry::method_t>::                       \
                create_instance(self, (::cppbind::MethodTableEntry::method_t)( \
                                          meth_wrapper));                      \
            return reinterpret_cast<::cppbind::MethodWrapper<                  \
                ::cppbind::MethodTableEntry::method_t> *>(ret);                \
          }))

#define MethodTableEntry_build_args_lambda(cpp_class, method)                  \
  [](PyObject *self, PyObject *args, PyObject *kwargs) -> PyObject * {         \
    cpp_class *obj = reinterpret_cast<cpp_class *>(self);                      \
    ::cppbind::Tuple tuple = ::cppbind::Tuple::from_args(args);                \
    auto ret = obj->method(tuple);                                             \
    return ::cppbind::into<decltype(ret)>(ret).unwrap();                       \
  }

#define MethodTableEntry_build_args_except_lambda(cpp_class, method)           \
  [](PyObject *self, PyObject *args, PyObject *kwargs) -> PyObject * {         \
    cpp_class *obj = reinterpret_cast<cpp_class *>(self);                      \
    ::cppbind::Tuple tuple = ::cppbind::Tuple::from_args(args);                \
    try {                                                                      \
      auto ret = obj->method(tuple);                                           \
      return ::cppbind::into<decltype(ret)>(ret).unwrap();                     \
    }                                                                          \
    except(std::exception &ex) { ::cppbind::PyErr_from_cpp_exception(ex); }    \
    return nullptr;                                                            \
  }

#define MethodTableEntry_build_noarg_lambda(cpp_class, method)                 \
  [](PyObject *self, PyObject *args, PyObject *kwargs) -> PyObject * {         \
    cpp_class *obj = reinterpret_cast<cpp_class *>(self);                      \
    auto ret = obj->method();                                                  \
    return ::cppbind::into<decltype(ret)>(ret).unwrap();                       \
  }

#define MethodTableEntry_build_noarg_except_lambda(cpp_class, method)          \
  [](PyObject *self, PyObject *args, PyObject *kwargs) -> PyObject * {         \
    cpp_class *obj = reinterpret_cast<cpp_class *>(self);                      \
    try {                                                                      \
      auto ret = obj->method();                                                \
      return ::cppbind::into<decltype(ret)>(ret).unwrap();                     \
    }                                                                          \
    except(std::exception &ex) { ::cppbind::PyErr_from_cpp_exception(ex); }    \
    return nullptr;                                                            \
  }

/**
 * Generate a {@link MethodTableEntry} for a method with arguments.
 * The method should take a {@link Tuple} as argument and
 * be marked with `noexcept` because the generated method wrapper does not
 * handle C++ exceptions.
 *
 * @param package_name the name of the package (string literal)
 * @param cpp_class the C++ class that contains the method.
 * @param method the method name.
 */
#define MethodTableEntry_build_args(package_name, cpp_class, method)           \
  MethodTableEntry_build(                                                      \
      package_name, cpp_class, method,                                         \
      MethodTableEntry_build_args_lambda(cpp_class, method))

#define MethodTableEntry_build_noarg(package_name, cpp_class, method)          \
  MethodTableEntry_build(                                                      \
      package_name, cpp_class, method,                                         \
      MethodTableEntry_build_noarg_lambda(cpp_class, method))
};

/**
 * The type wrapper for a C++ class.
 *
 * We use the `example:CInt` class to show how to
 * initialize the python typeinfo for a C++ class,
 * which can be found in `example/ffi/_ffi.cc`.
 *
 * <h3>In package source file</h3>
 * We explicitly use `type_static_members` to set
 * all static data members of {@link Type} to zero.
 * This avoids link error: undefined symbol.
 * <blockquote><pre>
 *   type_static_members(example::CInt);
 * </pre></blockquote>
 *
 * <h3>package initialization</h3>
 * in an `int (*)(void)` initialization function, do:
 * <blockquote><pre>
 * static int initialize_package(void) {
 *     type_init("_ffi", // package name
 *               CInt,   // the C++ class to expose
 *               "CInt"); // class name as it appears in python
 *  // use default integer operators:
 * type_init_integer_ops("_ffi", // package name
 *                       CInt,  // the C++ class to expose
 *                       "CInt"); // class name in python
 * }
 * </pre></blockquote>
 */
template <typename CppClass> struct Type {
  Type() = default;
  /**
   * Cast `this` to `PyTypeObject *`.
   */
  PyTypeObject *into() { return reinterpret_cast<PyTypeObject *>(this); }

  /**
   * Place-holder and the <b>only</b> non-static member of
   * {@link Type}.
   */
  PyTypeObject pytype;

  /**
   * The only instance of {@link Type}, used in Python.
   */
  static PyTypeObject *instance; /* only instance to this type. */

  /**
   * Array of {@link MethodTableEntry} for this C++ class.
   */
  static MethodTableEntry *methods;

  /**
   * Length of {@link Type::methods}.
   */
  static Py_ssize_t methods_cnt;

  /**
   * Default `__getattr__` implementation.
   * It will search for the method in the {@link MethodTableEntry}
   * array and return the corresponding method wrapper if found.
   */
  static PyObject *getattr(PyObject *, char *);

  /**
   * The module free function. It will be called when the module
   * is deallocated.
   */
  static void module_free(void);

  /**
   * Create a new instance of this type. It will call the default constructor of
   * the C++ class.
   */
  static PyObject *New(PyObject *mod);

#define type_gen_binary_op(CppClass, Operator)                                 \
  [](PyObject *a, PyObject *b) -> PyObject * {                                 \
    auto obj_a = CppClass::forward_or_convert(::cppbind::Object(a));           \
    auto obj_b = CppClass::forward_or_convert(::cppbind::Object(b));           \
    if (obj_a.ptr == nullptr || obj_b.ptr == nullptr) {                        \
      obj_a.unwrap();                                                          \
      obj_b.unwrap();                                                          \
      PyErr_SetString(PyExc_TypeError,                                         \
                      "arguments must be " STR(                                \
                          CppClass) " or convertible to " STR(CppClass));      \
      return nullptr;                                                          \
    }                                                                          \
    auto *cpp_a = reinterpret_cast<CppClass *>(obj_a.ptr);                     \
    auto *cpp_b = reinterpret_cast<CppClass *>(obj_b.ptr);                     \
    PyObject *ret = _PyObject_New(::cppbind::Type<CppClass>::instance);        \
    if (ret != nullptr) {                                                      \
      new (ret) CppClass();                                                    \
      *(reinterpret_cast<CppClass *>(ret)) = (*cpp_a)Operator(*cpp_b);         \
    } else {                                                                   \
      PyErr_SetString(PyExc_RuntimeError, "failed to create result object");   \
    }                                                                          \
    obj_a.unwrap();                                                            \
    obj_b.unwrap();                                                            \
    return ret;                                                                \
  }

#define type_gen_unary_op(CppClass, Operator)                                  \
  [](PyObject *a) -> PyObject * {                                              \
    auto obj_a = CppClass::forward_or_convert(::cppbind::Object(a));           \
    if (obj_a.ptr == nullptr) {                                                \
      PyErr_SetString(PyExc_TypeError,                                         \
                      "argument must be " STR(                                 \
                          CppClass) " or convertible to " STR(CppClass));      \
      return nullptr;                                                          \
    }                                                                          \
    auto *cpp_a = reinterpret_cast<CppClass *>(obj_a.ptr);                     \
    PyObject *ret = _PyObject_New(::cppbind::Type<CppClass>::instance);        \
    if (ret != nullptr) {                                                      \
      new (ret) CppClass();                                                    \
      *(reinterpret_cast<CppClass *>(ret)) = Operator(*cpp_a);                 \
    } else {                                                                   \
      PyErr_SetString(PyExc_RuntimeError, "failed to create result object");   \
    }                                                                          \
    obj_a.unwrap();                                                            \
    return ret;                                                                \
  }

#define type_gen_inplace_binary_op(CppClass, Operator)                         \
  [](PyObject *a, PyObject *b) -> PyObject * {                                 \
    auto obj_b = CppClass::forward_or_convert(::cppbind::Object(b));           \
    if (!PyObject_TypeCheck(a, ::cppbind::Type<CppClass>::instance) ||         \
        obj_b.ptr == nullptr) {                                                \
      PyErr_SetString(PyExc_TypeError,                                         \
                      "arguments must be " STR(                                \
                          CppClass) " or convertible to " STR(CppClass));      \
      return nullptr;                                                          \
    }                                                                          \
    auto *cpp_a = reinterpret_cast<CppClass *>(a);                             \
    auto *cpp_b = reinterpret_cast<CppClass *>(obj_b.ptr);                     \
    *cpp_a Operator *cpp_b;                                                    \
    obj_b.unwrap();                                                            \
    return a;                                                                  \
  }

#define type_static_members(cpp_class)                                         \
  template <> PyTypeObject * ::cppbind::Type<cpp_class>::instance = nullptr;   \
  template <>                                                                  \
  ::cppbind::MethodTableEntry * ::cppbind::Type<cpp_class>::methods = nullptr; \
  template <> Py_ssize_t ::cppbind::Type<cpp_class>::methods_cnt = 0

/**
 * This macro does very detailed initialization, therefore
 * should only be called inside a `RestInitFn` function.
 *
 * @param `module_name`: (const char *) name of the module.
 * @param `cpp_class`: the C++ class to be exposed.
 * @param `cpp_class_name`: (const char *) name of the class in python.
 */
#define type_init(module_name, cpp_class, cpp_class_name, ...)                 \
  static_assert(sizeof(::cppbind::Type<cpp_class>) == sizeof(PyTypeObject));   \
  static ::cppbind::MethodTableEntry CONCAT(methods_,                          \
                                            cpp_class)[] = {__VA_ARGS__};      \
  do {                                                                         \
    MethodWrapper_init(module_name, ::cppbind::MethodTableEntry::method_t);    \
    ::cppbind::Type<cpp_class>::methods = CONCAT(methods_, cpp_class);         \
    ::cppbind::Type<cpp_class>::methods_cnt =                                  \
        sizeof(CONCAT(methods_, cpp_class)) /                                  \
        sizeof(::cppbind::MethodTableEntry);                                   \
    auto *ty_ob = ::cppbind::PyTypeObject_Zero();                              \
    ::cppbind::Type<cpp_class>::instance = ty_ob;                              \
    if (ty_ob == nullptr) {                                                    \
      PyErr_SetString(PyExc_RuntimeError, "failed to create type object");     \
      return (1);                                                              \
    }                                                                          \
    ty_ob->tp_name = module_name "." cpp_class_name;                           \
    ty_ob->tp_flags = Py_TPFLAGS_DEFAULT;                                      \
    ty_ob->tp_basicsize = sizeof(cpp_class);                                   \
    ty_ob->tp_dealloc = [](PyObject *self) {                                   \
      cpp_class *cppobj = reinterpret_cast<cpp_class *>(self);                 \
      cppobj->~cpp_class();                                                    \
    };                                                                         \
    ty_ob->tp_getattr = ::cppbind::Type<cpp_class>::getattr;                   \
  } while (0)

#define type_float_unary_ops(X) X(-) X(+)
#define type_float_binary_ops(X) X(+) X(-) X(*) X(/)
#define type_float_inplace_ops(X) X(+=) X(-=) X(*=) X(/=)

/**
 * The C++ class `cpp_class` needs to support all integer operators,
 * to be used as an integer type in python. The operators include:
 *
 * <h2>binary operators</h2>
 * `+`, `-`, `*`, `/`, `%`, `^`, `|`, `&`, `>>`, `<<`
 *  and their in-place operators.
 *
 * <h2>unary operators</h2>
 * `~`, `int()` (convertible to `long long`), `-`
 */
#define type_integer_unary_ops(X) type_float_unary_ops(X) X(~)
#define type_integer_binary_ops(X)                                             \
  type_float_binary_ops(X) X(%) X(^) X(|) X(&) X(>>) X(<<)

#define type_integer_inplace_ops(X)                                            \
  type_float_inplace_ops(X) X(%=) X(^=) X(|=) X(&=) X(>>=) X(<<=)

#define type_init_integer_ops(module_name, cpp_class, cpp_class_name)          \
  static PyNumberMethods CONCAT(pynum_methods_, cpp_class) = {                 \
      .nb_add = type_gen_binary_op(cpp_class, +),                              \
      .nb_subtract = type_gen_binary_op(cpp_class, -),                         \
      .nb_multiply = type_gen_binary_op(cpp_class, *),                         \
      .nb_remainder = type_gen_binary_op(cpp_class, %),                        \
      .nb_negative = type_gen_unary_op(cpp_class, -),                          \
      .nb_positive = type_gen_unary_op(cpp_class, +),                          \
      .nb_invert = type_gen_unary_op(cpp_class, ~),                            \
      .nb_lshift = type_gen_binary_op(cpp_class, <<),                          \
      .nb_rshift = type_gen_binary_op(cpp_class, >>),                          \
      .nb_and = type_gen_binary_op(cpp_class, &),                              \
      .nb_xor = type_gen_binary_op(cpp_class, ^),                              \
      .nb_or = type_gen_binary_op(cpp_class, |),                               \
      .nb_int = [](PyObject *self) -> PyObject * {                             \
        const cpp_class *ptr = const_cast<const cpp_class *>(                  \
            reinterpret_cast<cpp_class *>(self));                              \
        long long value = (long long)*ptr;                                     \
        return PyLong_FromLongLong(value);                                     \
      },                                                                       \
      .nb_inplace_add = type_gen_inplace_binary_op(cpp_class, +=),             \
      .nb_inplace_subtract = type_gen_inplace_binary_op(cpp_class, -=),        \
      .nb_inplace_multiply = type_gen_inplace_binary_op(cpp_class, *=),        \
      .nb_inplace_remainder = type_gen_inplace_binary_op(cpp_class, %=),       \
      .nb_inplace_lshift = type_gen_inplace_binary_op(cpp_class, <<=),         \
      .nb_inplace_rshift = type_gen_inplace_binary_op(cpp_class, >>=),         \
      .nb_inplace_and = type_gen_inplace_binary_op(cpp_class, &=),             \
      .nb_inplace_xor = type_gen_inplace_binary_op(cpp_class, ^=),             \
      .nb_inplace_or = type_gen_inplace_binary_op(cpp_class, |=),              \
      .nb_floor_divide = type_gen_binary_op(cpp_class, /),                     \
      .nb_inplace_floor_divide = type_gen_inplace_binary_op(cpp_class, /=),    \
  };                                                                           \
  do {                                                                         \
    auto *instance = ::cppbind::Type<cpp_class>::instance;                     \
    instance->tp_as_number = &CONCAT(pynum_methods_, cpp_class);               \
    instance->tp_repr = [](PyObject *self) -> PyObject * {                     \
      char buf[32];                                                            \
      const cpp_class *ptr =                                                   \
          const_cast<const cpp_class *>(reinterpret_cast<cpp_class *>(self));  \
      long long value = (long long)*ptr;                                       \
      sprintf(buf, "%lld", value);                                             \
      auto *ret = PyUnicode_FromString(buf);                                   \
      if (ret == nullptr) {                                                    \
        PyErr_SetString(PyExc_RuntimeError, "failed to create string object"); \
      }                                                                        \
      return ret;                                                              \
    };                                                                         \
  } while (0)

/**
 * The C++ class `cpp_class` needs to have the following public members
 * to be used as a mapping type in python:
 *
 * <blockquote><pre>
 *   Integer size() const noexcept;
 *   bool contains(KeyType) const;
 *   ValueType get(KeyType) const;
 *   void put(KeyType, ValueType);
 * </pre></blockquote>
 */
#define type_init_mapping(module_name, cpp_class, cpp_class_name, ...)         \
  static PyMappingMethods CONCAT(mapping_methods_, cpp_class) = {              \
      .mp_length = [](PyObject *self) -> Py_ssize_t {                          \
        cpp_class *cppobj = reinterpret_cast<cpp_class *>(self);               \
        const cpp_class *ro_cppobj = const_cast<const cpp_class *>(cppobj);    \
        return static_cast<Py_ssize_t>(ro_cppobj->size());                     \
      },                                                                       \
      .mp_subscript = [](PyObject *self, PyObject *key) -> PyObject * {        \
        cpp_class *cppobj = reinterpret_cast<cpp_class *>(self);               \
        const cpp_class *ro_cppobj = const_cast<const cpp_class *>(cppobj);    \
        if (!ro_cppobj->contains(key)) {                                       \
          PyErr_SetString(PyExc_KeyError, "key not found");                    \
          return nullptr;                                                      \
        }                                                                      \
        auto ret = ro_cppobj->get(key);                                        \
        return ::cppbind::into<decltype(ret)>(ret).unwrap();                   \
      },                                                                       \
      .mp_ass_subscript = [](PyObject *self, PyObject *key,                    \
                             PyObject *value) -> int {                         \
        cpp_class *cppobj = reinterpret_cast<cpp_class *>(self);               \
        try {                                                                  \
          cppobj->put(key, value);                                             \
          return 0; /* success */                                              \
        } catch (std::exception & e) {                                         \
          PyErr_SetString(PyExc_RuntimeError, e.what());                       \
          return -1; /* failure */                                             \
        }                                                                      \
      }};                                                                      \
  do {                                                                         \
    ::cppbind::Type<cpp_class>::instance->tp_as_mapping =                      \
        &CONCAT(mapping_methods_, cpp_class);                                  \
  } while (0)
};

template <typename CppClass>
PyObject *Type<CppClass>::getattr(PyObject *self, char *name) {
  PyTypeObject *tp = self->ob_type;
  if (tp != Type<CppClass>::instance) {
    PyErr_SetString(PyExc_TypeError, "invalid type");
    return nullptr;
  }

  auto count = Type<CppClass>::methods_cnt;
  for (Py_ssize_t i = 0; i < count; i++) {
    if (strcmp(name, methods[i].name) == 0) {
      auto *ret = reinterpret_cast<PyObject *>(methods[i].gen(self));
      return ret;
    }
  }
  PyErr_SetString(PyExc_AttributeError, "attribute not found");
  return nullptr;
}

template <typename CppClass> void Type<CppClass>::module_free() {
  /* decrement refcnt of self. */
  Py_DECREF(Type<CppClass>::instance);

  /* Clear MethodWrapper type object. */
  MethodWrapper<MethodTableEntry::method_t>::fini_method_type();
}

template <typename CppClass> PyObject *Type<CppClass>::New(PyObject *mod) {
  if (unlikely(Type<CppClass>::instance == nullptr)) {
    PyErr_SetString(PyExc_RuntimeError, "type not initialized");
    return nullptr;
  }
  Py_INCREF(Type<CppClass>::instance);
  PyObject *ret = _PyObject_New(Type<CppClass>::instance);
  if (ret == nullptr) {
    PyErr_SetString(PyExc_RuntimeError, "failed to create object");
    return nullptr;
  }
  new (ret) CppClass();
  return ret;
}

} /* namespace cppbind */

#endif /* __PACKAGE_TYPE_H__ */
