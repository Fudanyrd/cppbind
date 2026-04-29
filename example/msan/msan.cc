#include <cstdio>
#include <map>

#include <cppbind.h>

using cppbind::Dict;
using cppbind::Long;
using cppbind::MethodWrapper;
using cppbind::Object;
using cppbind::Tuple;

namespace example {
namespace memory {

struct Resource {
private:
  PyObject pyobj;

public:
  /**
   * Default constructor will clear `pyobj`.
   * Use an empty constructor instead.
   */
  Resource() {}

  ~Resource() {
    cppbind_check_internal(count > 0 && "resource count should be positive");
    count--;
  }

  /**
   * @return the reference count of the object.
   */
  int getcount(void) const {
    return reinterpret_cast<const PyObject *>(this)->ob_refcnt;
  }

  /**
   * Track the number of instances to ensure that only one instance exists at a
   * time.
   */
  static int count;
};

int Resource::count = 0;

} /* namespace memory */
} /* namespace example */

extern "C" PyObject *play(PyObject *self, PyObject *args, PyObject *kwargs) {
  Tuple tuple = Tuple::from_args(args);
  Dict dict = Dict::from_kwargs(kwargs);
  return Py_None;
}

extern "C" PyObject *Resource_New(PyObject *self) {
  if (example::memory::Resource::count) {
    PyErr_SetString(PyExc_RuntimeError,
                    "Previous resource instance not freed.");
    return nullptr;
  }
  example::memory::Resource::count++;
  return ::cppbind::Type<example::memory::Resource>::New(self);
}

type_static_members(example::memory::Resource);

namespace example {
namespace memory {

static int _msan_rest_init(void) {
  type_init("msan", Resource, "Resource",
            MethodTableEntry_build_noarg("msan", Resource, getcount));
  return 0;
}

static int _msan_clear(PyObject *) {
  ::cppbind::Type<Resource>::module_free();
  return 0;
}

gen_modinit_fn_from_fns(msan, &_msan_rest_init, nullptr, nullptr, &_msan_clear,
                        gen_PyMethodDef_doc(Resource_New,
                                            ":returns: a new Resource object"),
                        gen_PyMethodDef_doc(play, ":returns: None"))

} /* namespace memory */
} /* namespace example */
