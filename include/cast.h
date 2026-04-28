#ifndef __CAST_H__
#define __CAST_H__ (1)

#include <common.h>

#include <numeric/long.h>
#include <object.h>

namespace cppbind {

/**
 * Convert C++ integer types to Python objects.
 */
template <typename _Tp> inline Object into(_Tp value) {
  __static_assert(is_integer_ty<_Tp>());
  return Long(value).object();
}

/**
 * Convert C++ string types to Python objects.
 */
template <typename _Tp> inline Object into(const _Tp *str) {
  __static_assert(is_char_ty<_Tp>());
  return Str(str).object();
}

/**
 * Build {@link cppbind::Object} from <pre>PyObject *</pre>.
 */
template <> inline Object into<PyObject *>(PyObject *pt) { return Object{pt}; }

} /* namespace cppbind */

#endif /* __CAST_H__ */
