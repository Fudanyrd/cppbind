#ifndef __PACKAGE_ENUM_H__
#define __PACKAGE_ENUM_H__ (1)

#include <algorithm>
#include <cstddef>
#include <cstring>

#include "package/bind.h"

namespace cppbind {

/**
 * A class representing a C++ enum type.
 *
 * Full example for binding an enum (also in example/ffi):
 * <blockquote><pre>
 * enum Color : unsigned int {
 *   RED = 0, GREEN, BLUE
 * };
 * enum_static_members(Color);
 * # define foreach_color(X) X(RED) X(GREEN) X(BLUE)
 * # define ColorPyName "Color"
 *
 * static int rest_init(cppbind::Module &curmod) {
 *   // ...
 *   enum_type_init(this_package, Color, ColorPyName, foreach_color);
 *   curmod.add_object(ColorPyName, cppbind::EnumObject&lt;Color&gt;::instance);
 *   // ...
 *   return 0;
 * }
 * </pre></blockquote>
 *
 * Using it in python:
 * <blockquote><pre>
 * from myext import Color # matches ColorPyName.
 *
 * r = Color.RED
 * str(r) # "RED"
 * r.value # 0
 * </pre></blockquote>
 */
template <typename EnumType> struct EnumObject {
public:
  /**
   * Representing the value in this enum.
   */
  struct EnumValue {
    /**
     * Name of enum. e.g. "RED", "GREEN", "BLUE" for the `Color` enum.
     */
    const char *name;

    /**
     * Actual value of the enum.
     */
    EnumType value;
#define __gen_compare_op(op)                                                   \
  bool operator op(const EnumValue &other) const {                             \
    return value op other.value;                                               \
  }

#define __cmp_ops(X) X(==) X(!=) X(<) X(<=) X(>) X(>=)
    /**
     * Binary comparison operators for `EnumValue`.
     * They compare the enum values, not the name.
     */
    __cmp_ops(__gen_compare_op);

#undef __cmp_ops
#undef __gen_compare_op

    /**
     * Constructor.
     */
    constexpr EnumValue(const char *name, EnumType value)
        : name(name), value(value) {}

    /**
     * Convertion to long.
     */
    operator long() const { return static_cast<long>(value); }

    /**
     * Convertion to C string.
     */
    operator const char *() const { return name; }

#define enum_value_static_members_declare(enum_class)                          \
  type_static_members_declare(                                                 \
      ::cppbind::CppObject<::cppbind::EnumObject<enum_class>::EnumValue>)

#define enum_value_static_members(enum_class)                                  \
  type_static_members(                                                         \
      ::cppbind::CppObject<::cppbind::EnumObject<enum_class>::EnumValue>)

#define __no_method(X)
#define enum_value_init(package_name, enum_class, pyclass_name)                \
  do {                                                                         \
    cpp_type_init(package_name, ::cppbind::EnumObject<enum_class>::EnumValue,  \
                  pyclass_name "."                                             \
                               "value_type",                                   \
                  __no_method);                                                \
    /* NOTE: the order of members are arranged such that their names appear in \
     * ascending order. */                                                     \
    static ::cppbind::DataMember members[2] = {                                \
        ::cppbind::DataMember(                                                 \
            "name",                                                            \
            [](PyObject *obj) -> PyObject * {                                  \
              auto &payload = *::cppbind::CppObject<::cppbind::EnumObject<     \
                  enum_class>::EnumValue>::get_payload(obj);                   \
              return ::cppbind::into(payload.name).unwrap();                   \
            },                                                                 \
            nullptr),                                                          \
        ::cppbind::DataMember(                                                 \
            "value",                                                           \
            [](PyObject *obj) -> PyObject * {                                  \
              auto &payload = *::cppbind::CppObject<::cppbind::EnumObject<     \
                  enum_class>::EnumValue>::get_payload(obj);                   \
              return ::cppbind::into((long)payload.value).unwrap();            \
            },                                                                 \
            nullptr)};                                                         \
    using type_wrap = ::cppbind::Type<                                         \
        ::cppbind::CppObject<::cppbind::EnumObject<enum_class>::EnumValue>>;   \
    type_wrap::data_members = members;                                         \
    type_wrap::data_members_cnt = 2;                                           \
    type_wrap::instance->tp_str = [](PyObject *obj) -> PyObject * {            \
      auto &payload = *::cppbind::CppObject<                                   \
          ::cppbind::EnumObject<enum_class>::EnumValue>::get_payload(obj);     \
      return PyUnicode_FromString(payload.name);                               \
    };                                                                         \
  } while (0)
  };

  /**
   * Alias for `EnumValue`.
   */
  using value_type = CppObject<EnumValue>;

  /**
   * The payload. In C++, its size is 1 byte
   * because it is empty.
   */
  using payload_t = EnumType;

  /**
   * No payload is included in the Python object,
   * so the layout only contains a `PyObject`.
   */
  using layout_t = struct {
    PyObject pyobj;
  };

  /**
   * The only instance of this type. It is initialized in
   * `enum_type_init`.
   */
  static PyTypeObject *instance;

  /**
   * Array of `EnumValue` for this enum type. It is initialized in
   * `enum_type_init`.
   */
  static EnumValue *values;

  /**
   * Length of `values`.
   */
  static Py_ssize_t values_cnt;

  /**
   * Implementation of `__getattr__`. It will search for the enum value
   * in the `values` array and return the corresponding enum value object if
   * found. If not, it raises `AttributeError`.
   *
   * Note: it is based on the assumption that `values` are sorted by name
   * to speed-up searching.
   */
  static PyObject *getattr(PyObject *, char *);

  /**
   * A short routine that creates a Python object for an enum value.
   */
  static Object into_impl(EnumValue value) {
    PyObject *res = PyObject_New(PyObject, value_type::py_type());
    auto *payload = value_type::get_payload(res);
    new (payload) EnumValue(value);
    return Object(res);
  }
};

template <typename EnumType>
PyObject *EnumObject<EnumType>::getattr(PyObject *, char *name) {
  auto it = std::lower_bound(
      values, values + values_cnt, EnumValue(name, values[0].value),
      [](const EnumValue &a, const EnumValue &b) -> bool {
        return strcmp(a.name, b.name) < 0;
      });
  if (it != values + values_cnt && strcmp(it->name, name) == 0) {
    return into_impl(*it).unwrap();
  }
  PyErr_SetString(PyExc_AttributeError, "no such enum value");
  return nullptr;
}

#define make_enum_value(name)                                                  \
  ::cppbind::EnumObject<__enum_class_alias>::EnumValue(                        \
      #name, __enum_class_alias::name),

#define enum_type_static_members(enum_class)                                   \
  template <>                                                                  \
  PyTypeObject *cppbind::EnumObject<enum_class>::instance = nullptr;           \
  template <>                                                                  \
  cppbind::EnumObject<enum_class>::EnumValue                                   \
      *cppbind::EnumObject<enum_class>::values = nullptr;                      \
  template <> Py_ssize_t cppbind::EnumObject<enum_class>::values_cnt = 0;      \
  enum_value_static_members(enum_class)

#define enum_type_static_members_declare(enum_class)                           \
  template <>                                                                  \
  cppbind::Object ::cppbind::into<                                             \
      ::cppbind::EnumObject<enum_class>::EnumValue>(                           \
      ::cppbind::EnumObject<enum_class>::EnumValue value);                     \
  template <> PyTypeObject *cppbind::EnumObject<enum_class>::instance;         \
  template <>                                                                  \
  cppbind::EnumObject<enum_class>::EnumValue                                   \
      * ::cppbind::EnumObject<enum_class>::values;                             \
  template <> Py_ssize_t cppbind::EnumObject<enum_class>::values_cnt;          \
  enum_value_static_members_declare(enum_class)

/**
 * @param package_name (string literal) the name of the package, e.g. "cppbind".
 * @param enum_type the C++ enum type to be wrapped.
 * @param pyclass_name (string literal) the name of the enum in Python.
 * @param foreach_value an X-macro that applies the argument to all values of
 * the enum. Example of this: <blockquote><pre> enum color { RED, GREEN };
 * #define foreach_color(X) X(RED) X(GREEN)
 * </pre></blockquote>
 */
#define enum_type_init(package_name, enum_type, pyclass_name, foreach_value)   \
  do {                                                                         \
    enum_value_init(package_name, enum_type, pyclass_name);                    \
    using __enum_class_alias = enum_type;                                      \
    static ::cppbind::EnumObject<enum_type>::EnumValue values[] = {            \
        foreach_value(make_enum_value)};                                       \
    auto num_values = sizeof(values) / sizeof(values[0]);                      \
    ::cppbind::EnumObject<enum_type>::values = values;                         \
    ::cppbind::EnumObject<enum_type>::values_cnt = num_values;                 \
    /* sort values by name. */                                                 \
    ::std::sort(values, values + num_values,                                   \
                [](const ::cppbind::EnumObject<enum_type>::EnumValue &a,       \
                   const ::cppbind::EnumObject<enum_type>::EnumValue &b) {     \
                  return strcmp(a.name, b.name) < 0;                           \
                });                                                            \
    auto *ty_ob = ::cppbind::PyTypeObject_Zero();                              \
    ty_ob->tp_name = package_name "." pyclass_name;                            \
    ty_ob->tp_flags = Py_TPFLAGS_DEFAULT;                                      \
    ty_ob->tp_basicsize = sizeof(::cppbind::EnumObject<enum_type>::layout_t);  \
    ty_ob->tp_dealloc = [](PyObject *) {};                                     \
    ty_ob->tp_init = [](PyObject *, PyObject *, PyObject *) { return 0; };     \
    ty_ob->tp_getattr = ::cppbind::EnumObject<enum_type>::getattr;             \
    ::cppbind::EnumObject<enum_type>::instance = ty_ob;                        \
  } while (0)

} /* namespace cppbind */

#endif /* __PACKAGE_ENUM_H__ */
