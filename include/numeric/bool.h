#ifndef __NUMERIC_BOOL_H__
#define __NUMERIC_BOOL_H__ (1)

#include <cstdbool>

#include "object.h"

namespace cppbind {
#define type_bool_binary_ops(X) X(&) X(&&) X(|) X(||) X(^)
#define type_bool_inplace_ops(X) X(&=) X(|=) X(^=)
#define type_bool_unary_ops(X) X(!) X(~)

struct Bool {
private:
  Object obj;

  bool eval() const { return obj.ptr == Py_True; }

public:
  explicit Bool(bool value)
      : obj(value ? Py_NewRef(Py_True) : Py_NewRef(Py_False)) {}

  Bool(PyObject *pt) : obj(pt) {}

  operator bool() const { return eval(); }

  /**
   * @return the reference to current object.
   */
  Object &object() { return obj; }

  /**
   * @return the reference to current object.
   */
  const Object &object() const { return obj; }

#define Bool_gen_inplace_op(op)                                                \
  Bool operator op(const Bool &other) {                                        \
    bool res = eval();                                                         \
    res op other.eval();                                                       \
    *this = Bool(res);                                                         \
    return *this;                                                              \
  }

  type_bool_inplace_ops(Bool_gen_inplace_op)
#undef Bool_gen_inplace_op
};

inline Bool operator!(const Bool &b) { return Bool(!static_cast<bool>(b)); }
inline Bool operator~(const Bool &b) { return Bool(!static_cast<bool>(b)); }

#define Bool_gen_binary_op(op)                                                 \
  inline Bool operator op(const Bool &lhs, const Bool &rhs) {                  \
    bool lvalue = static_cast<bool>(lhs);                                      \
    bool rvalue = static_cast<bool>(rhs);                                      \
    return Bool(lvalue op rvalue);                                             \
  }

type_bool_binary_ops(Bool_gen_binary_op)

#undef Bool_gen_binary_op

} /* namespace cppbind */

#endif /* __NUMERIC_BOOL_H__ */
