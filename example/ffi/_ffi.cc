#include <cstdio>
#include <map>

#include <cppbind.h>

using cppbind::Long;
using cppbind::MethodWrapper;
using cppbind::Object;
using std::map;

struct CInt {
  CInt() : num(0) {}

  int getvalue(void) const { return num; }

private:
  PyObject pyobj;

public:
  int num;
};

struct CppMap {
  struct Compare {
    PyObject *compare;

    Compare(PyObject *compare) : compare(compare) { Py_INCREF(compare); }
    ~Compare() { Py_DECREF(compare); }
    bool operator()(PyObject *a, PyObject *b) const {
      auto *ret = PyObject_CallFunctionObjArgs(compare, a, b, nullptr);
      assert(ret != nullptr);
      return PyObject_IsTrue(ret);
    }
  };

  CppMap(PyObject *compare_fn) : table(Compare(compare_fn)) {
    Py_INCREF(compare_fn);
  }
  ~CppMap() = default;

  PyObject *get(PyObject *key) const {
    auto it = table.find(key);
    if (it == table.end()) {
      return Py_None;
    }
    return it->second;
  }

  PyObject *get(const ::cppbind::Tuple &key) const;
  PyObject *put(const ::cppbind::Tuple &args);

  PyObject *put(PyObject *key, PyObject *value) {
    table[key] = value;
    Py_INCREF(value);
    return Py_None;
  }

  bool contains(PyObject *key) const { return table.find(key) != table.end(); }
  size_t size() const { return table.size(); }

  static PyObject *getItem(PyObject *self, PyObject *key);
  static int setItem(PyObject *self, PyObject *key, PyObject *value);

  PyObject pyobj;
  map<PyObject *, PyObject *, Compare> table;
};

PyObject *CppMap::get(const ::cppbind::Tuple &args) const {
  if (args.size() == 0) {
    PyErr_SetString(PyExc_ValueError, "key cannot be empty");
    return nullptr;
  }

  Object key = args[0];
  PyObject *default_value = args.size() > 1 ? (args[1].ptr) : Py_None;

  auto it = table.find(key.ptr);
  if (it == table.end()) {
    return default_value;
  }
  return it->second;
}

PyObject *CppMap::put(const ::cppbind::Tuple &args) {
  if (args.size() != 2) {
    PyErr_SetString(PyExc_ValueError,
                    "requires exactly 2 arguments: key and value");
    return nullptr;
  }

  Object key = args[0];
  Object value = args[1];
  (void)put(key.ptr, value.ptr);
  return Py_None;
}

type_static_members(CInt);
type_static_members(CppMap);

extern "C" PyObject *CppMap_New(PyObject *self, PyObject *compare_fn) {
  auto *type = cppbind::Type<CppMap>::instance;
  PyObject *ret = _PyObject_New((PyTypeObject *)type);
  new (ret) CppMap(compare_fn);
  return ret;
}

static PyObject *CInt_Init(PyObject *self) {
  return ::cppbind::Type<CInt>::New(self);
}

extern "C" PyObject *CInt_New(PyObject *self) { return CInt_Init(self); }
extern "C" PyObject *CInt_FromInt(PyObject *self, PyObject *arg) {
  auto *ret = CInt_Init(self);
  reinterpret_cast<CInt *>(ret)->num = (long)Long(arg);
  return ret;
}

static int _ffi_rest_init(void) {
  type_init("_ffi", CInt, "CInt",
            MethodTableEntry_build_noarg("_ffi", CInt, getvalue));
  type_init("_ffi", CppMap, "CppMap",
            MethodTableEntry_build_noarg("_ffi", CppMap, size),
            MethodTableEntry_build_args("_ffi", CppMap, get),
            MethodTableEntry_build_args("_ffi", CppMap, put));
  type_init_mapping("_ffi", CppMap, "CppMap");
  return 0;
}

gen_modinit_fn_from_fns(
    _ffi, &_ffi_rest_init, nullptr, nullptr, nullptr,
    gen_PyMethodDef_doc(CInt_New, ":returns: a new CInt object"),
    (PyMethodDef){"CInt_FromInt", (PyCFunction)CInt_FromInt, METH_O,
                  "Creates a new CInt object from an integer."},
    (PyMethodDef){
        "CppMap_New", (PyCFunction)CppMap_New, METH_O,
        "Creates a new CppMap object with the given compare function."})
