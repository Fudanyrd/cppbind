#ifndef __COMMON_H__
#define __COMMON_H__ (1)

#include <Python.h>
#include <cassert>
#include <setjmp.h>
#include <stdint.h>
#include <type_traits>
#include <utility>

#if defined __cplusplus
#if __cplusplus < 201100L
#error "requires -std=c++11 or above"
#endif /* __cplusplus < 201100 */
#else
#error "must use C++ compiler"
#endif /* defined __cplusplus */

#define __TO_STR(x) #x
#define STR(x) __TO_STR(x)
#if !defined __CONCAT
#define __CONCAT(a, b) a##b
#endif /* !defined __CONCAT */
#define CONCAT(a, b) __CONCAT(a, b)

#ifdef _GLIBCXX_ASSERTIONS
/*
 * NOTE: do not use this.
 * Use cppbind_assert for writing your own assertions.
 */
#define cppbind_check_internal(cond) assert(cond)
#else
#define cppbind_check_internal(cond) ((void)(cond))
/**
 * We do not use `((void) 0)` because expression `cond` may
 * have side effects.
 */
#endif /* _GLIBCXX_ASSERTIONS */

#define cppbind_assert(cond) assert(cond)

#if !defined __static_assert
#define __static_assert(const_cond)                                            \
  do {                                                                         \
    switch (0) {                                                               \
    case (0):                                                                  \
    case (const_cond):                                                         \
    default:                                                                   \
      break;                                                                   \
    }                                                                          \
  } while (0)
#endif /* __static_assert */

#if !defined likely
#define likely(cond) __builtin_expect(!!(cond), 1)
#endif
#if !defined unlikely
#define unlikely(cond) __builtin_expect(!!(cond), 0)
#endif

/* create a namespace for all classes/functions */
namespace cppbind {

constexpr char author[] = "Fudanyrd";
constexpr char version[] = "0.0.1";

#define instantiate_type_checker(checker_name, ty, ret)                        \
  template <> constexpr bool checker_name<ty>(void) { return (ret); }          \
  /* end of checker */

/**
 * @return `true` if `_Tp` is a boolean type; `false` otherwise.
 */
template <typename _Tp> constexpr bool is_bool_ty(void) { return false; }
#define bool_types(X) X(bool)
#define instantiate_bool_checker(ty)                                           \
  instantiate_type_checker(is_bool_ty, ty, true)
// clang-format off
bool_types( instantiate_bool_checker );
// clang-format on
#undef instantiate_bool_checker

/**
 * @return `true` if `_Tp` is a char type; `false` otherwise.
 */
template <typename _Tp> constexpr bool is_char_ty(void) { return false; }
#define char_types(X) X(signed char) X(unsigned char) X(char16_t) X(char32_t)
#define instantiate_char_checker(ty)                                           \
  instantiate_type_checker(is_char_ty, ty, true)
// clang-format off
/**
 * Template specialization for char types.
 */
char_types( instantiate_char_checker )
    // clang-format on

    template <typename _Tp>
    constexpr bool is_signed_integer_ty(void) {
  return false;
}
#define signed_integer_types(X) X(int8_t) X(int16_t) X(int32_t) X(int64_t)
#define instantiate_signed_integer_checker(ty)                                 \
  instantiate_type_checker(is_signed_integer_ty, ty, true)
// clang-format off
signed_integer_types( instantiate_signed_integer_checker )
    // clang-format on

    template <typename _Tp>
    constexpr bool is_unsigned_integer_ty(void) {
  return false;
}
#define unsigned_integer_types(X) X(uint8_t) X(uint16_t) X(uint32_t) X(uint64_t)
#define instantiate_unsigned_integer_checker(ty)                               \
  instantiate_type_checker(is_unsigned_integer_ty, ty, true)
// clang-format off
unsigned_integer_types( instantiate_unsigned_integer_checker )
    // clang-format on

    /* Interger types check */
    template <typename _Tp>
    constexpr bool is_integer_ty(void) {
  return is_signed_integer_ty<_Tp>() || is_unsigned_integer_ty<_Tp>();
}

/**
 * Floating point types check
 */
template <typename _Tp> constexpr bool is_fp_ty(void) { return false; }

#define floating_point_types(X) X(float) X(double)
#define instantiate_fp_checker(ty) instantiate_type_checker(is_fp_ty, ty, true)
// clang-format off
floating_point_types( instantiate_fp_checker )
    // clang-format on

    template <typename _Tp>
    constexpr bool is_numeric_ty(void) {
  return is_integer_ty<_Tp>() || is_fp_ty<_Tp>();
}

/**
 * @return `true` if `_Tp` is `void`; `false` otherwise.
 */
template <typename T> constexpr bool is_void_ty() { return false; }

/**
 * Specialization for `void` type.
 *
 * @return `true` if `_Tp` is `void`; `false` otherwise.
 */
template <> constexpr bool is_void_ty<void>() { return true; }

/**
 * @return `true` if `_Tp` is `const char *`; `false` otherwise.
 */
template <typename _Tp> constexpr bool is_pkc_ty() { return false; }

/**
 * @return `true` if `_Tp` is `const char *`; `false` otherwise.
 */
template <> constexpr bool is_pkc_ty<const char *>() { return true; }

/**
 * @return true if `_Tp` has `Object object()`.
 */
template <typename _Tp> constexpr bool is_object_ty() { return false; }

#define object_types(X)                                                        \
  X(Bytes) X(Dict) X(List) X(Str) X(Tuple) X(Object) X(Bool)
#define instantiate_object_checker(ty)                                         \
  struct ty;                                                                   \
  instantiate_type_checker(is_object_ty, ty, true)

// clang-format off
object_types( instantiate_object_checker );
// clang-format on
#undef instantiate_object_checker

template <typename _Tp> constexpr bool is_pair_ty_impl(...) { return false; }
template <typename _Tp>
constexpr auto is_pair_ty_impl(int) -> decltype(std::declval<_Tp>().first,
                                                std::declval<_Tp>().second,
                                                true) {
  return true;
}

template <typename T> constexpr bool is_pair_ty() {
  return is_pair_ty_impl<T>(0);
}

/**
 * Helper for implementing `is_copyable_ty`.
 */
template <typename _Tp> constexpr bool can_copy_impl(...) { return false; }

/**
 * Helper for implementing `is_copyable_ty`.
 */
template <typename _Tp>
constexpr auto can_copy_impl(int)
    -> decltype(std::declval<_Tp &>() = std::declval<const _Tp &>(), true) {
  return true;
}

/**
 * @return `true` if `_Tp` is copyable (i.e. has copy assignment operator);
 */
template <typename _Tp> constexpr bool is_copyable_ty() {
  return can_copy_impl<_Tp>(0);
}

/**
 * Helper for `is_pointer_ty`.
 */
template <typename _Tp> constexpr bool is_pointer_ty_impl(...) { return false; }

/**
 * Helper for `is_pointer_ty`.
 */
template <typename _Tp>
constexpr auto is_pointer_ty_impl(int)
    -> decltype(*std::declval<_Tp>(), std::declval<_Tp &>() = nullptr,
                std::declval<_Tp &>() = reinterpret_cast<_Tp>(42),
                std::declval<_Tp &>()++, std::declval<_Tp &>()--, true) {
  return true;
}

/**
 * @return true if _Tp is a pointer type.
 */
template <typename _Tp> constexpr bool is_pointer_ty() {
  return is_pointer_ty_impl<_Tp>(0);
}

/**
 * Helper for `is_pyobject_ty`.
 */
template <typename _Tp> constexpr bool is_pyobject_ty() { return false; }

/**
 * @return true for PyObject type.
 */
template <> constexpr bool is_pyobject_ty<PyObject>() { return true; }

/**
 * helper for `is_pyobject_ptr_ty`.
 */
template <typename _Tp> constexpr bool is_pyobject_ptr_ty() { return false; }

/**
 * @return true for PyObject pointer type.
 */
template <> constexpr bool is_pyobject_ptr_ty<PyObject *>() { return true; }

/**
 * Helper for `is_pyobject_wrap`.
 */
template <typename _Tp> constexpr bool is_pyobject_wrap_ty() { return false; }

/**
 * @return true for `cppbind::Object`
 */
template <> constexpr bool is_pyobject_wrap_ty<Object>() { return true; }

/**
 * Check whether _Tp is an abstract (purely virtual) type.
 */
template <typename _Tp> constexpr bool is_abstract_ty() {
  return std::is_abstract<_Tp>::value;
}

} /* namespace cppbind */

#endif /* __COMMON_H__ */
