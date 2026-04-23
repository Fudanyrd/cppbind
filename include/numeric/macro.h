#ifndef __NUMERIC_MACRO_H__
#define __NUMERIC_MACRO_H__ (1)

#define gen_inplace_op_impl(self, operator_, object_fn)                        \
  self &operator operator_(const self & other) {                               \
    obj.object_fn(other.obj);                                                  \
    return *this;                                                              \
  }

#endif /* __NUMERIC_MACRO_H__ */
