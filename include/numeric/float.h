#ifndef __NUMERIC_FLOAT_H__
#define __NUMERIC_FLOAT_H__ (1)

#include <floatobject.h>

#include <common.h>
#include <object.h>

#include <numeric/long.h>
#include <numeric/macro.h>

namespace cppbind {

/**
 * Python float type.
 */
struct Float {
private:
  Object obj;

public:
  /**
   * Construct from a python float.
   */
  Float(PyObject *obj) : obj(PyFloat_Check(obj) ? obj : PyNumber_Float(obj)) {
    if (this->obj.ptr == nullptr) {
      /* possibly PyNumber_Float failure. */
      cppbind_check_internal(0 && "failed to convert to float");
      /* if assertion is disabled, construct from a default value. */
      PyErr_Clear();
      this->obj.ptr = PyFloat_FromDouble(0.0);
    }
  }

  /**
   * Construct from an {@link Object} guarding a python float.
   */
  Float(const Object &obj)
      : obj(PyFloat_Check(obj.ptr) ? obj.ptr : PyNumber_Float(obj.ptr)) {}

#define gen_constructor_for_ty(cpp_ty)                                         \
  Float(cpp_ty val) : obj(PyFloat_FromDouble(static_cast<double>(val))) {}

  floating_point_types(gen_constructor_for_ty);
  signed_integer_types(gen_constructor_for_ty);
  unsigned_integer_types(gen_constructor_for_ty);
#undef gen_constructor_for_ty

#define gen_convertion_to_ty(cpp_ty)                                           \
  operator cpp_ty() const {                                                    \
    return static_cast<cpp_ty>(PyFloat_AsDouble(obj.ptr));                     \
  }

  floating_point_types(gen_convertion_to_ty);
  signed_integer_types(gen_convertion_to_ty);
  unsigned_integer_types(gen_convertion_to_ty);

#undef gen_convertion_to_ty

  /**
   * @return the reference to current object.
   */
  Object &object() { return obj; }

  /**
   * @return the reference to current object.
   */
  const Object &object() const { return obj; }

  /**
   * @return nan value.
   */
  static Float nan(void) { return Float((double)Py_NAN); }

  /**
   * @return inf value.
   */
  static Float inf(void) {
    double x;
    uint64_t inf_repr = ((uint64_t)0x7ff) << 52;
    *((uint64_t *)&x) = inf_repr;
    return Float(x);
  }

  gen_inplace_op_impl(Float, +=, inplace_num_Add);
  gen_inplace_op_impl(Float, -=, inplace_num_Subtract);
  gen_inplace_op_impl(Float, *=, inplace_num_Multiply);
  gen_inplace_op_impl(Float, /=, inplace_num_TrueDivide);
};

} /* namespace cppbind */

#endif /* __NUMERIC_FLOAT_H__ */
