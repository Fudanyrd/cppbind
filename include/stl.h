#ifndef __STL_H__
#define __STL_H__ (1)

#include <cstdarg>
#include <iostream>
#include <stdexcept>
#include <string>

#include "object.h"

/* Conversion of Objects to STL string */
namespace cppbind {

/**
 * Check if a type is an STL string type.
 */
template <typename _Tp> constexpr bool is_cxx_std_string(void) { return false; }

/**
 * Check if a type is an STL string type.
 */
template <> constexpr bool is_cxx_std_string<std::string>(void) { return true; }

/**
 * Convert a Python object to an STL string.
 */
std::string stringify(PyObject *obj);

/**
 * Convert a Python object to an STL string.
 */
inline std::string stringify(const Object &obj) { return stringify(obj.ptr); }

/**
 * Convert a `HasObject` type (e.g. {@link List}, {@link Long}) to string.
 */
template <typename _Object_Ty>
inline std::string stringify(const _Object_Ty &obj) {
  return stringify(obj.object());
}

} /* namespace cppbind */

/* Print to STL output streams */
namespace cppbind {

/**
 * Writes the representation of a Python object to an STL output stream.
 */
std::ostream &operator<<(std::ostream &stream, PyObject *obj);

/**
 * Writes the representation of the Python object held by
 * {@link Object} to an STL output stream.
 */
inline std::ostream &operator<<(std::ostream &stream, const Object &obj) {
  return stream << obj.ptr;
}

/**
 * Writes the representation of the Python object held by a `HasObject` type
 * (e.g. {@link List}, {@link Long}) to an STL output stream.
 */
template <typename _Object_Ty,
          std::__enable_if_t<is_object_ty<_Object_Ty>(), bool> = true>
inline std::ostream &operator<<(std::ostream &stream, const _Object_Ty &obj) {
  return stream << obj.object();
}

} /* namespace cppbind */

namespace cppbind {

/**
 * Implement Python's `iternext` function for STL container's iterators.
 * This requires only C++'s forward iterators.
 *
 * To instantiate the template, the container must have:
 * <ul>
 *   <li>begin() and end() methods;</li>
 *   <li>iterator type (copyable, comparable by ==, !=);</li>
 * </ul>
 */
template <typename STLContainerTy> struct STLIterator {
  /**
   * Type of the iterator of the STL container.
   */
  using IteratorTy = typename STLContainerTy::iterator;

  /**
   * The constructor of the STL iterator wrapper. It takes an iterator and a
   * pointer to the container to be iterated.
   */
  STLIterator(IteratorTy iter, STLContainerTy *container)
      : iter(iter), container(container) {
    static_assert(is_copyable_ty<IteratorTy>(),
                  "iterator type must be copyable");
    cppbind_check_internal(container != nullptr &&
                           "container pointer cannot be null");
  }

  /**
   * Use a container's pointer and its `begin()` iterator to construct an STL
   * iterator wrapper.
   */
  STLIterator(STLContainerTy *container)
      : iter(container->begin()), container(container) {
    static_assert(is_copyable_ty<IteratorTy>(),
                  "iterator type must be copyable");
    cppbind_check_internal(container != nullptr &&
                           "container pointer cannot be null");
  }
  ~STLIterator() = default;

  /**
   * The implementation of `iternext` for the STL iterator wrapper. It returns
   * the next element in the container, or set `StopIteration` if the end of
   * the container is reached.
   */
  static PyObject *iternext(PyObject *obj) {
    /**
     * This method is called only by CppObject<STLContainer>.
     * So does not have to check type of `obj`.
     */
    auto *self = CppObject<STLIterator>::get_payload(obj);
    auto *container = self->container;
    auto &iter = self->iter;
    if (iter == container->end()) {
      /* end of iteration */
      PyErr_SetNone(PyExc_StopIteration);
      return nullptr;
    }
    PyObject *ret = into(*iter).unwrap();
    iter++;
    return ret;
  }

private:
  IteratorTy iter;
  STLContainerTy *container;
};

/**
 * There is no non-static methods of STLIterator.
 */
#define STLIterator_foreach_method(X)

/**
 * Add a method named `staticized_size` to the wrapper of an STL container,
 * which returns python's `lenfunc` for the container if it has a `size()`
 * method, or `nullptr` otherwise.
 */
#define stl_staticize_size(ContainerTy)                                        \
  template <typename _Tp> static constexpr bool has_size_impl(...) {           \
    return false;                                                              \
  }                                                                            \
  template <typename _Tp>                                                      \
  static constexpr auto has_size_impl(                                         \
      int) -> decltype(::std::declval<_Tp>().size(), true) {                   \
    return true;                                                               \
  }                                                                            \
  template <typename _Tp> static constexpr bool has_size(void) {               \
    return has_size_impl<_Tp>(0);                                              \
  }                                                                            \
  template <typename _Tp, ::std::__enable_if_t<has_size<_Tp>(), bool> = true>  \
  static inline lenfunc staticized_size_impl(void) {                           \
    return [](PyObject *obj) -> Py_ssize_t {                                   \
      auto *self = CppObject<_Tp>::get_payload(obj);                           \
      return static_cast<Py_ssize_t>(self->size());                            \
    };                                                                         \
  }                                                                            \
  template <typename _Tp, ::std::__enable_if_t<!has_size<_Tp>(), bool> = true> \
  static inline lenfunc staticized_size_impl(void) {                           \
    return nullptr;                                                            \
  }                                                                            \
  static inline lenfunc staticized_size(void) {                                \
    return staticized_size_impl<payload_t>();                                  \
  }

#define stl_staticize_hash(ContainerTy)                                        \
  template <typename _Tp> static constexpr bool has_hash_impl(...) {           \
    return false;                                                              \
  }                                                                            \
  template <typename _Tp>                                                      \
  static constexpr auto has_hash_impl(                                         \
      int) -> decltype(::std::declval<_Tp>().hash(), true) {                   \
    return true;                                                               \
  }                                                                            \
  template <typename _Tp> static constexpr bool has_hash(void) {               \
    return has_hash_impl<_Tp>(0);                                              \
  }                                                                            \
  template <typename _Tp, ::std::__enable_if_t<has_hash<_Tp>(), bool> = true>  \
  static inline hashfunc staticized_hash_impl(void) {                          \
    return [](PyObject *obj) -> Py_hash_t {                                    \
      auto *self = CppObject<_Tp>::get_payload(obj);                           \
      return self->hash();                                                     \
    };                                                                         \
  }                                                                            \
  template <typename _Tp, ::std::__enable_if_t<!has_hash<_Tp>(), bool> = true> \
  static inline hashfunc staticized_hash_impl(void) {                          \
    return nullptr;                                                            \
  }                                                                            \
  static inline hashfunc staticized_hash(void) {                               \
    return staticized_hash_impl<payload_t>();                                  \
  }

#define stl_class_wrapper_common(stl_class)                                    \
  cpp_class_wrapper_common(stl_class);                                         \
  stl_staticize_size(stl_class) stl_staticize_hash(stl_class)

/**
 * Add a `get_iter` function to the wrapper of an STL container, which returns
 * an iterator wrapper. Before defining `CppObject<stl_class>`,
 * its iterator wrapper `CppObject<STLIterator<stl_class>>` is automatically
 * defined.
 *
 * <b>Note</b>: this is only used inside namespace `cppbind`.
 *
 * <b>Note</b>: passing a template class like `std::map<PyObject *, PyObject *>`
 * will not work as expected, because the preprocessor will treat
 * `std::map<PyObject *` and `PyObject *>` as multiple arguments.
 *
 * @see cpp_class_wrapper
 * @see STLIterator
 */
#define stl_class_wrapper(stl_class, foreach_method)                           \
  cpp_class_wrapper(STLIterator<stl_class>, STLIterator_foreach_method);       \
  template <> struct CppObject<stl_class> {                                    \
    stl_class_wrapper_common(stl_class);                                       \
    staticize_destructor(payload_t);                                           \
    foreach_method(cpp_class_wrapper_impl);                                    \
    static PyObject *get_iter(PyObject *self) {                                \
      auto *container = get_payload(self);                                     \
      PyObject *ret =                                                          \
          _PyObject_New(Type<CppObject<STLIterator<stl_class>>>::instance);    \
      if (ret != nullptr) {                                                    \
        new (CppObject<STLIterator<stl_class>>::get_payload(ret))              \
            STLIterator<stl_class>(container);                                 \
      } else {                                                                 \
        PyErr_SetString(PyExc_RuntimeError,                                    \
                        "failed to create iterator object");                   \
      }                                                                        \
      return ret;                                                              \
    }                                                                          \
  }

/**
 * Set the container's `getiter`, and iterator wrapper class's `iternext`, to
 * make the container iterable in Python. This should be called in the type
 * initialization of the container wrapper.
 */
#define stl_type_init(package_name, stl_class, pyclass_name, foreach_method)   \
  cpp_type_init(package_name, stl_class, pyclass_name, foreach_method);        \
  cpp_type_init(package_name, ::cppbind::STLIterator<stl_class>,               \
                pyclass_name "_iter", STLIterator_foreach_method);             \
  do {                                                                         \
    auto *container_ty_ob =                                                    \
        ::cppbind::Type<::cppbind::CppObject<stl_class>>::instance;            \
    container_ty_ob->tp_iter = ::cppbind::CppObject<stl_class>::get_iter;      \
    container_ty_ob->tp_hash =                                                 \
        ::cppbind::CppObject<stl_class>::staticized_hash();                    \
    auto *iter_ty_ob = ::cppbind::Type<                                        \
        ::cppbind::CppObject<::cppbind::STLIterator<stl_class>>>::instance;    \
    iter_ty_ob->tp_iternext = ::cppbind::STLIterator<stl_class>::iternext;     \
  } while (0)

/**
 * @return false, because _Tp is not a mapping container in STL.
 */
template <typename _Tp> constexpr bool stl_has_mapping_impl(...) {
  return false;
}

/**
 * @return true if _Tp has key_type, mapped_type and find() method.
 */
template <typename _Tp>
constexpr auto stl_has_mapping_impl(int)
    -> decltype(::std::declval<typename _Tp::key_type>(),
                ::std::declval<typename _Tp::mapped_type>(),
                ::std::declval<_Tp>().find(
                    ::std::declval<typename _Tp::key_type>()),
                true) {
  return true;
}

/**
 * Check if a type is a mapping container in STL. A mapping container is a
 * container that has key_type, mapped_type and find() method, e.g.
 * `std::map`.
 */
template <typename _Tp> constexpr bool stl_has_mapping(void) {
  return stl_has_mapping_impl<_Tp>(0);
}

/**
 * Initialize the mapping protocol of an STL container wrapper. It will set the
 * `mp_length`, `mp_subscript` and `mp_ass_subscript` of the container's type
 * object.
 */
template <typename _Tp>
inline void stl_type_initialize_mapping(PyMappingMethods *mapping_methods) {
  static_assert(stl_has_mapping_impl<_Tp>(0),
                "type does not have mapping protocol");

  mapping_methods->mp_length = ::cppbind::CppObject<_Tp>::staticized_size();
  mapping_methods->mp_subscript = [](PyObject *obj,
                                     PyObject *key) -> PyObject * {
    auto *self = CppObject<_Tp>::get_payload(obj);
    using key_type = typename _Tp::key_type;
    using value_type = typename _Tp::mapped_type;
    try {
      auto iter = self->find(from<key_type>(key));
      if (iter == self->end()) {
        PyErr_SetString(PyExc_KeyError, "key not found");
        return nullptr;
      }
      return into(iter->second).unwrap();
    } catch (const std::exception &e) {
      PyErr_SetString(PyExc_RuntimeError, e.what());
      return nullptr;
    }
  };

  mapping_methods->mp_ass_subscript = [](PyObject *obj, PyObject *key,
                                         PyObject *value) -> int {
    auto *self = CppObject<_Tp>::get_payload(obj);
    using key_type = typename _Tp::key_type;
    using value_type = typename _Tp::mapped_type;
    try {
      if (value == nullptr) {
        /* delete item */
        auto iter = self->find(from<key_type>(key));
        if (iter == self->end()) {
          PyErr_SetString(PyExc_KeyError, "key not found");
          return -1;
        }
        self->erase(iter);
      } else {
        /* set item */
        (*self)[from<key_type>(key)] = from<value_type>(value);
      }
      return 0;
    } catch (const std::exception &e) {
      PyErr_SetString(PyExc_RuntimeError, e.what());
      return -1;
    }
  };
}

#define stl_type_init_mapping(stl_type)                                        \
  do {                                                                         \
    static PyMappingMethods mapping_methods;                                   \
    ::cppbind::stl_type_initialize_mapping<stl_type>(&mapping_methods);        \
    auto *ty_ob = ::cppbind::Type<::cppbind::CppObject<stl_type>>::instance;   \
    ty_ob->tp_as_mapping = &mapping_methods;                                   \
  } while (0)

} /* namespace cppbind */

#endif /* __STL_H__ */
