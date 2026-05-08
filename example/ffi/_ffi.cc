#include <cppbind-sys.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

#if !defined this_package
#define this_package "_ffi"
#endif /* !defined this_package */

/**
 * NOLINTBEGIN(bugprone-easily-swappable-parameters,
 *             readability-identifier-length)
 */

namespace cppbind {

#define std_map_foreach_method(X) X(size, size, size_t)

#define std_vector_foreach_method(X)                                           \
  X(size, size, size_t)                                                        \
  X(push_back, append, void, Object) X(swap, swap, void, pyvec_t &)

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

struct Point2D {
  double x, y;

  Point2D() : x(0), y(0) {}

  Point2D &operator+=(const Point2D &other) {
    x += other.x;
    y += other.y;
    return *this;
  }

  Point2D &operator-=(const Point2D &other) {
    x -= other.x;
    y -= other.y;
    return *this;
  }
};

Point2D operator+(const Point2D &lhs, const Point2D &rhs) {
  Point2D res = lhs;
  res += rhs;
  return res;
}
Point2D operator-(const Point2D &lhs, const Point2D &rhs) {
  Point2D res = lhs;
  res -= rhs;
  return res;
}

std::ostream &operator<<(std::ostream &os, const Point2D &p) {
  os << "Point2D(" << p.x << ", " << p.y << ")";
  return os;
}

/**
 * Forward declarations.
 */
template <> struct CppObject<pymap_t>;
template <> struct CppObject<STLIterator<pymap_t>>;
template <> struct CppObject<pyvec_t>;
template <> struct CppObject<STLIterator<pyvec_t>>;
template <> struct CppObject<int>;
type_static_members_declare(CppObject<pymap_t>);
type_static_members_declare(CppObject<STLIterator<pymap_t>>);
type_static_members_declare(CppObject<int>);
type_static_members_declare(CppObject<pyvec_t>);
type_static_members_declare(CppObject<STLIterator<pyvec_t>>);
type_static_members_declare(CppObject<Point2D>);
template <> pyvec_t &from<pyvec_t &>(PyObject *obj);
template <> pymap_t &from<pymap_t &>(PyObject *obj);

stl_class_wrapper(pymap_t, std_map_foreach_method);
stl_class_wrapper(pyvec_t, std_vector_foreach_method);
cpp_class_wrapper(int, STLIterator_foreach_method);
cpp_class_wrapper(Point2D, STLIterator_foreach_method);

template <> inline pyvec_t &from<pyvec_t &>(PyObject *obj) {
  return *CppObject<pyvec_t>::get_payload(obj);
}

template <> inline pymap_t &from<pymap_t &>(PyObject *obj) {
  return *CppObject<pymap_t>::get_payload(obj);
}

template <> Point2D from<Point2D>(PyObject *obj) {
  if (PyObject_TypeCheck(obj, Type<CppObject<Point2D>>::instance)) {
    return *CppObject<Point2D>::get_payload(obj);
  }
  cppbind_from_on_type_mismatch;
}

} /* namespace cppbind */

type_static_members(cppbind::CppObject<cppbind::pymap_t>);
type_static_members(cppbind::CppObject<cppbind::STLIterator<cppbind::pymap_t>>);
type_static_members(cppbind::CppObject<int>);
type_static_members(cppbind::CppObject<cppbind::pyvec_t>);
type_static_members(cppbind::CppObject<cppbind::STLIterator<cppbind::pyvec_t>>);
type_static_members(cppbind::CppObject<cppbind::Point2D>);

static int rest_init() {
  MethodWrapper_init(this_package, cppbind::MethodTableEntry::method_t);
  stl_type_init(this_package, cppbind::pymap_t, "map", std_map_foreach_method);
  stl_type_init_mapping(cppbind::pymap_t);

  cpp_type_init(this_package, int, "cint", STLIterator_foreach_method);
  cpp_type_init_number(int);

  stl_type_init(this_package, cppbind::pyvec_t, "vector",
                std_vector_foreach_method);
  stl_type_init_sequence(cppbind::pyvec_t);

#define point_2d_members(X) X(x, false) X(y, false)
  cpp_type_init(this_package, cppbind::Point2D, "Point2D",
                STLIterator_foreach_method);
  cpp_type_init_number(cppbind::Point2D);
  cpp_type_init_data_members(cppbind::Point2D, point_2d_members);
#undef point_2d_members
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

extern "C" PyObject *point2d(void) {
  return cppbind::staticize_constructor<cppbind::Point2D>(nullptr, nullptr, 0);
}

gen_modinit_fn_from_fns(
    /* name */ _ffi, &rest_init, nullptr, nullptr, nullptr,
    gen_PyMethodDef(map),
    {"debug_refcnt", debug_refcnt, METH_O,
     "Debug function to get the reference count of a Python object."},
    gen_PyMethodDef(cint), gen_PyMethodDef(vector), gen_PyMethodDef(point2d));

/**
 * NOLINTEND(bugprone-easily-swappable-parameters,
 *           readability-identifier-length)
 */
