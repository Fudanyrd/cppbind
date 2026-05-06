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
template <typename _Object_Ty>
inline std::ostream &operator<<(std::ostream &stream, const _Object_Ty &obj) {
  return stream << obj.object();
}

} /* namespace cppbind */

namespace cppbind {

/**
 * Implement Python's `iternext` function for STL container's iterators.
 * This requires only C++'s forward iterators.
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
    auto *self = reinterpret_cast<STLIterator *>(obj + 1);
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
    cpp_class_wrapper_common(stl_class);                                       \
    staticize_destructor(payload_t);                                           \
    foreach_method(cpp_class_wrapper_impl);                                    \
    static PyObject *get_iter(PyObject *self) {                                \
      auto *container = get_payload(self);                                     \
      PyObject *ret =                                                          \
          _PyObject_New(Type<CppObject<STLIterator<stl_class>>>::instance);    \
      if (ret != nullptr) {                                                    \
        new (ret) STLIterator<stl_class>(container);                           \
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
    auto *iter_ty_ob = ::cppbind::Type<                                        \
        ::cppbind::CppObject<::cppbind::STLIterator<stl_class>>>::instance;    \
    iter_ty_ob->tp_iternext = ::cppbind::STLIterator<stl_class>::iternext;     \
  } while (0)

} /* namespace cppbind */

#endif /* __STL_H__ */
