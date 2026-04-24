#ifndef __CAST_H__
#define __CAST_H__ (1)

#include <common.h>

#include <numeric/long.h>
#include <object.h>

namespace cppbind {

template <typename _Tp> Object into(_Tp value) {
  __static_assert(is_integer_ty<_Tp>());
  return Long(value).object();
}

template <typename _Tp> Object into(const _Tp *str) {
  __static_assert(is_char_ty<_Tp>());
  return Str(str).object();
}

} /* namespace cppbind */

#endif /* __CAST_H__ */
