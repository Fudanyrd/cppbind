#ifndef __PACKAGE_TYPE_H__
#define __PACKAGE_TYPE_H__ (1)

#include <cstring>

#include <common.h>
#include <object.h>
#include <package/func.h>

namespace cppbind {

static inline PyTypeObject *PyTypeObject_Zero() {
  PyTypeObject *ret = reinterpret_cast<PyTypeObject *>(
      _PyObject_New((PyTypeObject *)&PyType_Type));
  memset(&(ret->tp_name), 0, sizeof(*ret) - offsetof(PyTypeObject, tp_name));
  return ret;
}

struct MethodTableEntry {
public:
  typedef PyObject *(*method_t)(PyObject *self, PyObject *args,
                                PyObject *kwargs);
  typedef MethodWrapper<method_t> *(*gen_t)(PyObject *self);
  const char *name; /* static allocated string */
  gen_t gen;        /* generator of method wrapper, takes self as argument. */

  MethodTableEntry(const char *name, gen_t gen) : name(name), gen(gen) {}

#define MethodTableEntry_build(package_name, cpp_class, method, meth_wrapper)  \
  ::cppbind::MethodTableEntry(                                                 \
      STR(method),                                                             \
      (::cppbind::MethodTableEntry::gen_t)(                                    \
          [](PyObject *self) -> ::cppbind::MethodWrapper<                      \
                                 ::cppbind::MethodTableEntry::method_t> * {    \
            auto *ret = ::cppbind::MethodWrapper<                              \
                ::cppbind::MethodTableEntry::method_t>::                       \
                createInstance(                                                \
                    self,                                                      \
                    (::cppbind::MethodTableEntry::method_t)(meth_wrapper),     \
                    package_name);                                             \
            return reinterpret_cast<::cppbind::MethodWrapper<                  \
                ::cppbind::MethodTableEntry::method_t> *>(ret);                \
          }))

#define MethodTableEntry_build_args_lambda(cpp_class, method)                  \
  [](PyObject *self, PyObject *args, PyObject *kwargs) -> PyObject * {         \
    cpp_class *obj = reinterpret_cast<cpp_class *>(self);                      \
    ::cppbind::Tuple tuple{Object{args}};                                      \
    auto ret = obj->method(tuple);                                             \
    tuple.object().unwrap();                                                   \
    return ::cppbind::into<decltype(ret)>(ret).unwrap();                       \
  }

#define MethodTableEntry_build_noarg_lambda(cpp_class, method)                 \
  [](PyObject *self, PyObject *args, PyObject *kwargs) -> PyObject * {         \
    cpp_class *obj = reinterpret_cast<cpp_class *>(self);                      \
    auto ret = obj->method();                                                  \
    return ::cppbind::into<decltype(ret)>(ret).unwrap();                       \
  }

#define MethodTableEntry_build_args(package_name, cpp_class, method)           \
  MethodTableEntry_build(                                                      \
      package_name, cpp_class, method,                                         \
      MethodTableEntry_build_args_lambda(cpp_class, method))

#define MethodTableEntry_build_noarg(package_name, cpp_class, method)          \
  MethodTableEntry_build(                                                      \
      package_name, cpp_class, method,                                         \
      MethodTableEntry_build_noarg_lambda(cpp_class, method))
};

template <typename CppClass> struct Type {
  Type() = default;
  PyTypeObject *into() { return reinterpret_cast<PyTypeObject *>(this); }

  PyTypeObject pytype;

  static PyTypeObject *instance; /* only instance to this type. */
  static MethodTableEntry *methods;
  static Py_ssize_t methods_cnt;
  static PyObject *getattr(PyObject *, char *);
  static void module_free(void);
  static PyObject *New(PyObject *mod);

#define type_static_members(cpp_class)                                         \
  template <> PyTypeObject * ::cppbind::Type<cpp_class>::instance = nullptr;   \
  template <>                                                                  \
  ::cppbind::MethodTableEntry * ::cppbind::Type<cpp_class>::methods = nullptr; \
  template <> Py_ssize_t ::cppbind::Type<cpp_class>::methods_cnt = 0

/*
 * This macro does very detailed initialization, therefore
 * should only be called inside a `RestInitFn` function.
 *
 * Param `module_name`: (const char *) name of the module.
 * Param `cpp_class`: the C++ class to be exposed.
 * Param `cpp_class_name`: (const char *) name of the class in python.
 */
#define type_init(module_name, cpp_class, cpp_class_name, ...)                 \
  static_assert(sizeof(::cppbind::Type<cpp_class>) == sizeof(PyTypeObject));   \
  static ::cppbind::MethodTableEntry CONCAT(methods_,                          \
                                            cpp_class)[] = {__VA_ARGS__};      \
  do {                                                                         \
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

/*
 * The C++ class `cpp_class` needs to have the following public members
 * to be used as a mapping type in python:
 *
 * `IntLike size() const noexcept`
 * `bool contains(KeyType) const noexcept`
 * `ValueType get(KeyType)`
 * `void put(KeyType, ValueType)`
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
      Py_INCREF(ret);
      return ret;
    }
  }
  PyErr_SetString(PyExc_AttributeError, "attribute not found");
  return nullptr;
}

template <typename CppClass> void Type<CppClass>::module_free() {
  /* decrement refcnt of self. */
  Py_DECREF(Type<CppClass>::instance);
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
