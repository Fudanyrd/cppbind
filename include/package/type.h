#ifndef __PACKAGE_TYPE_H__
#define __PACKAGE_TYPE_H__ (1)

#include <algorithm>
#include <cstdio>
#include <cstring>

#include "common.h"
#include "object.h"
#include "package/func.h"
#include "traceback.h"

namespace cppbind {

static inline PyTypeObject *PyTypeObject_Zero() {
  PyTypeObject *ret = reinterpret_cast<PyTypeObject *>(
      _PyObject_New((PyTypeObject *)&PyType_Type));
  memset(&(ret->tp_name), 0, sizeof(*ret) - offsetof(PyTypeObject, tp_name));
  return ret;
}

/**
 * Bind C++ type's data member to Python attribute.
 */
struct DataMember {

  /**
   * Getter type for the data member. It takes a pointer to the object and
   * returns the value of the data member as a `PyObject *`.
   */
  typedef PyObject *(*GetterType)(PyObject *object);

  /**
   * Setter type for the data member. It takes a pointer to the object and a
   * `PyObject *` representing the value to be set.
   */
  typedef void (*SetterType)(PyObject *object, PyObject *value);

  /**
   * Construct a dummy `DataMember. Set getter to a magic
   * number to make sure it is not null.
   */
  DataMember(void) : name(""), getter(GetterType(42)), setter(nullptr) {
    cppbind_check_internal(getter != nullptr);
  }

  /**
   * The constructor you should use.
   *
   * @param name the name of the data member; an identifier.
   * @param getter the getter function for the data member. It should not be
   * null.
   * @param setter the setter function for the data member. It should be null if
   * the data member is read-only; otherwise, it should not be null.
   */
  DataMember(const char *name, GetterType getter, SetterType setter)
      : name(name), getter(getter), setter(setter) {
    cppbind_assert(name != nullptr && getter != nullptr);
  }

  /**
   * Construct a dummy `DataMember. Set getter to a magic
   * number to make sure it is not null.
   */
  DataMember(const char *name)
      : name(name), getter(GetterType(42)), setter(nullptr) {
    cppbind_check_internal(getter != nullptr);
  }

  /**
   * String literal representing the name of the data member.
   */
  const char *name;

  /**
   * Getter function for the data member. It should not be null.
   */
  GetterType getter;

  /**
   * Set this to nullptr if the data member is read-only.
   * Otherwise, it should be a valid function pointer.
   */
  SetterType setter;

  /**
   * Comparison based on `name`.
   */
  bool operator==(const DataMember &other) const {
    return strcmp(name, other.name) == 0;
  }
  /**
   * Comparison based on `name`.
   */
  bool operator!=(const DataMember &other) const { return !(*this == other); }
  /**
   * Comparison based on `name`.
   */
  bool operator<(const DataMember &other) const {
    return strcmp(name, other.name) < 0;
  }
  /**
   * Comparison based on `name`.
   */
  bool operator<=(const DataMember &other) const {
    return strcmp(name, other.name) <= 0;
  }
  /**
   * Comparison based on `name`.
   */
  bool operator>(const DataMember &other) const {
    return strcmp(name, other.name) > 0;
  }
  /**
   * Comparison based on `name`.
   */
  bool operator>=(const DataMember &other) const {
    return strcmp(name, other.name) >= 0;
  }

#define cpp_type_gen_getter(cpp_type, member)                                  \
  [](PyObject *self) -> PyObject * {                                           \
    cpp_type *ptr_this = ::cppbind::CppObject<cpp_type>::get_payload(self);    \
    return ::cppbind::into(ptr_this->member).unwrap();                         \
  }

#define cpp_type_gen_setter(cpp_type, member)                                  \
  [](PyObject *self, PyObject *value) {                                        \
    cpp_type *ptr_this = ::cppbind::CppObject<cpp_type>::get_payload(self);    \
    ptr_this->member = ::cppbind::from<decltype(ptr_this->member)>(value);     \
  }
};

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
   * Default copy constructor. It is sufficient because all members of
   * {@link MethodTableEntry} are trivially copyable.
   */
  MethodTableEntry(const MethodTableEntry &) = default;

  /**
   * Default assignment operator. It is sufficient because all members of
   * {@link MethodTableEntry} are trivially copyable.
   */
  MethodTableEntry &operator=(const MethodTableEntry &) = default;

  /**
   * Comparison based on `name`.
   */
  bool operator==(const MethodTableEntry &other) const {
    return strcmp(name, other.name) == 0;
  }
  /**
   * Comparison based on `name`.
   */
  bool operator!=(const MethodTableEntry &other) const {
    return !(*this == other);
  }
  /**
   * Comparison based on `name`.
   */
  bool operator<(const MethodTableEntry &other) const {
    return strcmp(name, other.name) < 0;
  }
  /**
   * Comparison based on `name`.
   */
  bool operator<=(const MethodTableEntry &other) const {
    return strcmp(name, other.name) <= 0;
  }
  /**
   * Comparison based on `name`.
   */
  bool operator>(const MethodTableEntry &other) const {
    return strcmp(name, other.name) > 0;
  }
  /**
   * Comparison based on `name`.
   */
  bool operator>=(const MethodTableEntry &other) const {
    return strcmp(name, other.name) >= 0;
  }

  /**
   * @param name the name of the method. It should be a string literal.
   * @param gen the generator of {@link MethodWrapper}.
   */
  MethodTableEntry(const char *name, gen_t gen) : name(name), gen(gen) {
    cppbind_assert(name != nullptr && gen != nullptr);
  }

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

/**
 * Create a dummy {@link MethodTableEntry}. It is used to pad the method table
 * when there is no method, because GNU g++ does not accept zero-size array.
 *
 * Its gen function should never be used.
 */
#define MethodTableEntry_dummy(name)                                           \
  ::cppbind::MethodTableEntry(name, (::cppbind::MethodTableEntry::gen_t)42)
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
   *
   * <i>reference count management</i> at package initialization,
   * it is created with reference count 1, and it is never decref-ed
   * until the module object is destroyed, invoking {@link module_free},
   * which will decref it.
   */
  static PyTypeObject *instance; /* only instance to this type. */

  /**
   * Array of {@link MethodTableEntry} for this C++ class.
   * After 975153cc, the methods are sorted by name in ascending
   * order to speed up searching in `getattr`.
   *
   * It is allocated statically, so it should not be freed.
   */
  static MethodTableEntry *methods;

  /**
   * Length of {@link Type::methods}.
   */
  static Py_ssize_t methods_cnt;

  /**
   * Array of {@link DataMember} for this C++ class.
   *
   * It is allocated statically, so it should not be freed.
   */
  static DataMember *data_members;

  /**
   * Length of {@link Type::data_members}.
   */
  static Py_ssize_t data_members_cnt;

  /**
   * Default `__getattr__` implementation.
   * It will search for the method in the {@link MethodTableEntry}
   * array and return the corresponding method wrapper if found.
   * If not, it resumes to searching a data member in the `DataMember`
   * array and return the result of the {@link DataMember::getter} if found.
   */
  static PyObject *getattr(PyObject *, char *);

  /**
   * Implementation of `__setattr__`. It will search for the data member in the
   * `DataMember` array and call the corresponding setter if found.
   */
  static int setattr(PyObject *, char *, PyObject *);

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
    Py_INCREF(a); /* in-place operator should return the first operand. */     \
    return a;                                                                  \
  }

#define type_static_members(cpp_class)                                         \
  template <> PyTypeObject *cppbind::Type<cpp_class>::instance = nullptr;      \
  template <>                                                                  \
  ::cppbind::MethodTableEntry *cppbind::Type<cpp_class>::methods = nullptr;    \
  template <> Py_ssize_t cppbind::Type<cpp_class>::methods_cnt = 0;            \
  template <>                                                                  \
  cppbind::DataMember *cppbind::Type<cpp_class>::data_members = nullptr;       \
  template <> Py_ssize_t cppbind::Type<cpp_class>::data_members_cnt = 0

#define type_static_members_declare(cpp_class)                                 \
  template <> PyTypeObject *cppbind::Type<cpp_class>::instance;                \
  template <>::cppbind::MethodTableEntry *cppbind::Type<cpp_class>::methods;   \
  template <> Py_ssize_t cppbind::Type<cpp_class>::methods_cnt;                \
  template <> cppbind::DataMember *cppbind::Type<cpp_class>::data_members;     \
  template <> Py_ssize_t cppbind::Type<cpp_class>::data_members_cnt

/**
 * This macro does very detailed initialization, therefore
 * should only be called inside a `RestInitFn` function.
 *
 * After 975153cc, the methods are sorted by name to speed up
 * searching in `getattr`.
 *
 * @param `module_name`: (const char *) name of the module.
 * @param `cpp_class`: the C++ class to be exposed.
 * @param `cpp_class_name`: (const char *) name of the class in python.
 */
#define type_init(module_name, cpp_class, cpp_class_name, ...)                 \
  static_assert(sizeof(::cppbind::Type<cpp_class>) == sizeof(PyTypeObject),    \
                "cppbind::Type<cpp_class> must be PyTypeObject-sized.");       \
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
    auto *base_ptr = ::cppbind::Type<cpp_class>::methods;                      \
    auto *end_ptr = base_ptr + ::cppbind::Type<cpp_class>::methods_cnt;        \
    ::std::sort(base_ptr, end_ptr);                                            \
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
#define type_init_mapping(module_name, cpp_class, cpp_class_name)              \
  static PyMappingMethods CONCAT(mapping_methods_, cpp_class) = {              \
      .mp_length = [](PyObject *self) -> Py_ssize_t {                          \
        auto *cppobj = reinterpret_cast<cpp_class *>(self);                    \
        const auto *ro_cppobj = const_cast<const cpp_class *>(cppobj);         \
        return static_cast<Py_ssize_t>(ro_cppobj->size());                     \
      },                                                                       \
      .mp_subscript = [](PyObject *self, PyObject *key) -> PyObject * {        \
        auto *cppobj = reinterpret_cast<cpp_class *>(self);                    \
        const auto *ro_cppobj = const_cast<const cpp_class *>(cppobj);         \
        if (!ro_cppobj->contains(key)) {                                       \
          PyErr_SetString(PyExc_KeyError, "key not found");                    \
          return nullptr;                                                      \
        }                                                                      \
        auto ret = ro_cppobj->get(key);                                        \
        return ::cppbind::into<decltype(ret)>(ret).unwrap();                   \
      },                                                                       \
      .mp_ass_subscript = [](PyObject *self, PyObject *key,                    \
                             PyObject *value) -> int {                         \
        auto *cppobj = reinterpret_cast<cpp_class *>(self);                    \
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
  auto *base = Type<CppClass>::methods;
  /**
   * Create a fake MethodTableEntry. Cannot initialize with (name, nullptr)
   * because of the assertion in the constructor of MethodTableEntry.
   */
  auto target = MethodTableEntry_dummy(name);
  auto *iter = std::lower_bound(base, base + count, target);
  if (iter != base + count && 0 == strcmp(iter->name, name)) {
    auto *ret = reinterpret_cast<PyObject *>(iter->gen(self));
    return ret;
  }

  /**
   * Try looking into the data members.
   */
  if (data_members_cnt > 0) {
    auto *data_base = Type<CppClass>::data_members;
    auto *data_iter = std::lower_bound(data_base, data_base + data_members_cnt,
                                       DataMember(name));
    if (data_iter != data_base + data_members_cnt &&
        0 == strcmp(data_iter->name, name)) {
      auto *getter = data_iter->getter; /* must not be nullptr. */
      return getter(self);
    }
  }

  PyErr_SetString(PyExc_AttributeError, "attribute not found");
  return nullptr;
}

template <typename CppClass>
int Type<CppClass>::setattr(PyObject *self, char *name, PyObject *value) {
  PyTypeObject *tp = self->ob_type;
  if (tp != Type<CppClass>::instance) {
    PyErr_SetString(PyExc_TypeError, "invalid type");
    return -1;
  }

  if (data_members_cnt > 0) {
    auto *data_base = Type<CppClass>::data_members;
    auto *data_iter = std::lower_bound(data_base, data_base + data_members_cnt,
                                       DataMember(name));
    if (data_iter != data_base + data_members_cnt &&
        0 == strcmp(data_iter->name, name)) {
      auto *setter = data_iter->setter;
      if (setter == nullptr) {
        PyErr_SetString(PyExc_AttributeError, "attribute is read-only");
        return -1;
      }
      if (value == nullptr) {
        PyErr_SetString(PyExc_AttributeError,
                        "deletion of attributes is not supported");
        return -1;
      }
      setter(self, value);
      return 0;
    }
  }

  PyErr_SetString(PyExc_AttributeError, "attribute not found");
  return -1;
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
