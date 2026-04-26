#include <cassert>
#include <iostream>

#include <cppbind.h>
#include <stl.h>

using cppbind::List;
using cppbind::Long;
using cppbind::Object;
using cppbind::Tuple;

extern "C" PyObject *myadd(PyObject *self, PyObject *args) {
  __static_assert(cppbind::CFunction_flags<decltype(&myadd)>());
  assert(args && PyTuple_Check(args));
  Tuple list{Object{args}};
  assert(list.size() == 2);
  Long a(list[0].ptr), b(list[1].ptr);
  a += b;
  return a.object().unwrap();
}

extern "C" PyObject *mysum(PyObject *self, PyObject *args) {
  __static_assert(cppbind::CFunction_flags<decltype(&mysum)>());
  assert(args && PyTuple_Check(args));
  Tuple list{Object{args}};
  Long sum(0);
  for (Py_ssize_t i = 0; i < list.size(); i++) {
    Long a(list[i].ptr);
    sum += a;
  }
  return sum.object().unwrap();
}

extern "C" PyObject *mysum_vec(PyObject *self, PyObject *const *args,
                               Py_ssize_t arglen) {
  __static_assert(cppbind::CFunction_flags<decltype(&mysum_vec)>());
  Long sum(0);
  for (Py_ssize_t i = 0; i < arglen; i++) {
    Long a(args[i]);
    sum += a;
  }
  return sum.object().unwrap();
}

extern "C" PyObject *kwarg_names(PyObject *self, PyObject *args,
                                 PyObject *kwargs) {
  __static_assert(cppbind::CFunction_flags<decltype(&kwarg_names)>());

  if (kwargs && PyDict_Check(kwargs)) {
  } else {
    List empty;
    return empty.object().unwrap();
  }

  Object ret{PyDict_Keys(kwargs)};
  return ret.object().unwrap();
}

/* Returns a tuple of `(len(args), len(kwargs))`. */
extern "C" PyObject *len_args_kwargs(PyObject *self, PyObject *const *args,
                                     Py_ssize_t arglen, PyObject *kwnames) {
  __static_assert(cppbind::CFunction_flags<decltype(&len_args_kwargs)>());

  Long len_args(arglen);
  long kwarg_size = 0;
  if (kwnames) {
    if (PyTuple_Check(kwnames)) {
      kwarg_size = PyTuple_Size(kwnames);
    } else {
      cppbind_check_internal(0 && "kwargs is not dict or tuple");
    }
  }
  Long len_kwargs(kwarg_size);

  Tuple ret{len_args.object(), len_kwargs.object()};
  return ret.object().unwrap();
}

// clang-format off
/*
 * Example of [rasing exception](https://docs.python.org/3/c-api/exceptions.html#raising-exceptions)
 */
extern "C" PyObject *always_throw(PyObject *self, PyObject *const *args,
                                  Py_ssize_t arglen);
// clang-format on
extern "C" PyObject *always_throw(PyObject *self, PyObject *const *args,
                                  Py_ssize_t arglen) {
  /* raise ValueError */
  PyErr_SetObject(PyExc_ValueError, Py_None);
  return nullptr;
}

gen_modinit_fn_from_fns(
    /* name */ myabc, nullptr, nullptr, nullptr,
    gen_PyMethodDef_doc(myadd, ":returns: sum of two integers"),
    gen_PyMethodDef_doc(mysum, ":returns: sum of all integers"),
    gen_PyMethodDef_doc(mysum_vec, ":returns: sum of all integers"),
    gen_PyMethodDef_doc(kwarg_names,
                        ":returns: a list of name of each kw argument."),
    gen_PyMethodDef_doc(len_args_kwargs,
                        ":returns: a tuple of `(len(args), len(kwargs))`."),
    gen_PyMethodDef_doc(always_throw, ":raises: ValueError always"));
