#include <cppbind-sys.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

#define this_package "_ffi"

/**
 * NOLINTBEGIN(bugprone-easily-swappable-parameters,
 *             readability-identifier-length)
 */

namespace cppbind {

#define std_map_foreach_method(X) X(size, size, size_t)

#define std_vector_foreach_method(X)                                           \
  X(size, size, size_t) X(push_back, append, void, Object)

/**
 * Object comparator.
 */
struct ObjectCompare {
  /**
   * Compare two `Object`s by comparing the underlying Python objects with
   * `PyObject_RichCompareBool` and `Py_LT`.
   */
  bool operator()(const Object &lhs, const Object &rhs) const noexcept {
    int ret = PyObject_RichCompareBool(lhs.ptr, rhs.ptr, Py_LT);
    assert(ret != -1);
    return ret != 0;
  }
};

/**
 * It is more reasonable to use `Object` than `PyObject *` as the key and
 * value type of `std::map`, because `Object` can manage the reference count
 * of the Python objects.
 */
using pymap_t = ::std::map<Object, Object, ObjectCompare>;
using pyvec_t = ::std::vector<Object>;
type_static_members_declare(CppObject<pymap_t>);
type_static_members_declare(CppObject<STLIterator<pymap_t>>);
type_static_members_declare(CppObject<int>);
type_static_members_declare(CppObject<pyvec_t>);
type_static_members_declare(CppObject<STLIterator<pyvec_t>>);

stl_class_wrapper(pymap_t, std_map_foreach_method);
stl_class_wrapper(pyvec_t, std_vector_foreach_method);
cpp_class_wrapper(int, STLIterator_foreach_method);

} /* namespace cppbind */

type_static_members(cppbind::CppObject<cppbind::pymap_t>);
type_static_members(cppbind::CppObject<cppbind::STLIterator<cppbind::pymap_t>>);
type_static_members(cppbind::CppObject<int>);
type_static_members(cppbind::CppObject<cppbind::pyvec_t>);
type_static_members(cppbind::CppObject<cppbind::STLIterator<cppbind::pyvec_t>>);

static int rest_init() {
  MethodWrapper_init(this_package, cppbind::MethodTableEntry::method_t);
  stl_type_init(this_package, cppbind::pymap_t, "map", std_map_foreach_method);
  stl_type_init_mapping(cppbind::pymap_t);

  cpp_type_init(this_package, int, "cint", STLIterator_foreach_method);
  cpp_type_init_number(int);

  stl_type_init("cxxstd", cppbind::pyvec_t, "vector",
                std_vector_foreach_method);
  stl_type_init_sequence(cppbind::pyvec_t);
  return 0;
}

/**
 * @return an empty `pymap_t` object.
 */
extern "C" PyObject *map(void) {
  return cppbind::staticize_constructor<cppbind::pymap_t>(nullptr, nullptr, 0);
}

/**
 * @return a `CppObject<int>` object with the payload initialized to 0.
 */
extern "C" PyObject *cint(void) {
  return cppbind::staticize_constructor<int>(nullptr, nullptr, 0);
}

PyObject *vector(void) {
  return cppbind::staticize_constructor<cppbind::pyvec_t>(nullptr, nullptr, 0);
}

/**
 * @return the reference count of the given Python object, at the time
 * of calling this function. Before it is called, the reference count
 * of `arg` should be at least 1.
 */
extern "C" PyObject *debug_refcnt(PyObject *self, PyObject *arg) {
  auto refcnt = Py_REFCNT(arg);
  return cppbind::into(refcnt).unwrap();
}

gen_modinit_fn_from_fns(
    /* name */ _ffi, &rest_init, nullptr, nullptr, nullptr,
    gen_PyMethodDef(map),
    {"debug_refcnt", debug_refcnt, METH_O,
     "Debug function to get the reference count of a Python object."},
    gen_PyMethodDef(cint), gen_PyMethodDef(vector));

/**
 * NOLINTEND(bugprone-easily-swappable-parameters,
 *           readability-identifier-length)
 */
