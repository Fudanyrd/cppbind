#ifndef __COMMON_H__
#define __COMMON_H__ (1)

#include <Python.h>
#include <stdint.h>

#if defined __cplusplus
#if __cplusplus < 201100L
#error "requires -std=c++11 or above"
#endif /* __cplusplus < 201100 */
#else
#error "must use C++ compiler"
#endif /* defined __cplusplus */

#if !defined __static_assert
#define __static_assert(const_cond)                                            \
  do {                                                                         \
    switch (0) {                                                               \
    case (0):                                                                  \
    case (const_cond):                                                         \
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

template <typename _Tp> constexpr bool is_signed_integer_ty(void) {
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
  instantiate_type_checker(is_signed_integer_ty, ty, false)
// clang-format off
unsigned_integer_types( instantiate_unsigned_integer_checker )
    // clang-format on

    /* Interger types check */
    template <typename _Tp>
    constexpr bool is_integer_ty(void) {
  return is_signed_integer_ty<_Tp>() || is_unsigned_integer_ty<_Tp>();
}

/* Floating point types check */
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

} /* namespace cppbind */

#endif /* __COMMON_H__ */
