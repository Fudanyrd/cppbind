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
  PyObject *put(PyObject *key, PyObject *value) {
    if (value == nullptr) {
      PyErr_SetString(PyExc_ValueError, "value cannot be None");
      return nullptr;
    }
    table[key] = value;
    Py_INCREF(value);
    return Py_None;
  }

  static PyTypeObject *type();
  static PyObject *getattr(PyObject *self, char *name);
  static PyObject *_get(PyObject *self, PyObject *args, PyObject *kwargs) {
    CppMap *cppmap = reinterpret_cast<CppMap *>(self);
    PyObject *key = PyTuple_GetItem(args, 0);
    return cppmap->get(key);
  }
  static PyObject *_put(PyObject *self, PyObject *args, PyObject *kwargs);
  static PyObject *_size(PyObject *self, PyObject *args, PyObject *kwargs) {
    CppMap *cppmap = reinterpret_cast<CppMap *>(self);
    return Long(cppmap->table.size()).object().unwrap();
  }

  static PyObject *getItem(PyObject *self, PyObject *key);
  static int setItem(PyObject *self, PyObject *key, PyObject *value);

  PyObject pyobj;
  map<PyObject *, PyObject *, Compare> table;
};

auto CppMap::_put(PyObject *self, PyObject *args,
                  PyObject *kwargs) -> PyObject * {
  CppMap *cppmap = reinterpret_cast<CppMap *>(self);
  if (!PyTuple_Check(args)) {
    PyErr_SetString(PyExc_TypeError, "arguments must be passed in a tuple");
    return nullptr;
  }
  if (PyTuple_Size(args) != 2) {
    PyErr_SetString(PyExc_TypeError, "put method requires exactly 2 arguments");
    return nullptr;
  }

  PyObject *key = PyTuple_GetItem(args, 0);
  PyObject *value = PyTuple_GetItem(args, 1);
  return cppmap->put(key, value);
}

PyObject *CppMap::getattr(PyObject *self, char *name) {
  if (strcmp(name, "get") == 0) {
    return MethodWrapper<decltype(&CppMap::_get)>::createInstance(
        self, &CppMap::_get, "_ffi");
  } else if (strcmp(name, "put") == 0) {
    return MethodWrapper<decltype(&CppMap::_put)>::createInstance(
        self, &CppMap::_put, "_ffi");
  } else if (strcmp(name, "size") == 0) {
    return MethodWrapper<decltype(&CppMap::_size)>::createInstance(
        self, &CppMap::_size, "_ffi");
  }
  PyErr_SetString(PyExc_AttributeError, "attribute not found");
  return nullptr;
}

PyTypeObject *CppMap::type() {
  auto *ret = (PyTypeObject *)_PyObject_New((PyTypeObject *)&PyType_Type);
  memset(&(ret->tp_name), 0,
         sizeof(PyTypeObject) - offsetof(PyTypeObject, tp_name));
  ret->tp_name = "_ffi.CppMap";
  ret->tp_basicsize = sizeof(CppMap);
  ret->tp_dealloc = [](PyObject *self) {
    CppMap *cppmap = reinterpret_cast<CppMap *>(self);
    cppmap->~CppMap();
  };
  ret->tp_getattr = &CppMap::getattr;
  ret->tp_flags = Py_TPFLAGS_DEFAULT;
  static PyMappingMethods mapping_methods = {
      .mp_length = [](PyObject *self) -> Py_ssize_t {
        auto *self_as_map = reinterpret_cast<CppMap *>(self);
        return static_cast<Py_ssize_t>(self_as_map->table.size());
      },
      .mp_subscript = &CppMap::getItem,
      .mp_ass_subscript = &CppMap::setItem,
  };
  ret->tp_as_mapping = &mapping_methods;
  return ret;
}

PyObject *CppMap::getItem(PyObject *self, PyObject *key) {
  CppMap *cppmap = reinterpret_cast<CppMap *>(self);
  auto iter = cppmap->table.find(key);
  if (iter == cppmap->table.end()) {
    PyErr_SetString(PyExc_KeyError, "key not found");
    return nullptr;
  }
  auto *ret = iter->second;
  return ret;
}

int CppMap::setItem(PyObject *self, PyObject *key, PyObject *value) {
  CppMap *cppmap = reinterpret_cast<CppMap *>(self);
  auto iter = cppmap->table.find(key);
  if (iter == cppmap->table.end()) {
    cppmap->table[key] = value;
  } else {
    Py_DECREF(iter->second);
    iter->second = value;
  }
  Py_INCREF(value);
  return 0;
}

extern "C" PyObject *CppMap_New(PyObject *self, PyObject *compare_fn) {
  auto *type = CppMap::type();
  PyObject *ret = _PyObject_New((PyTypeObject *)type);
  new (ret) CppMap(compare_fn);
  return ret;
}

static inline CInt *from_pyobj(PyObject *obj) {
  return reinterpret_cast<CInt *>(obj);
}

extern "C" void CInt_destructor(PyObject *self) {
  CInt *cint = from_pyobj(self);
  /* Explicitly calls the destructor of CInt. */
  cint->~CInt();
}

extern "C" PyObject *CInt_getvalue(PyObject *self) {
  return Long(from_pyobj(self)->getvalue()).object().unwrap();
}

extern "C" PyObject *CInt_getattr(PyObject *self, char *name) {
  if (strcmp(name, "num") == 0) {
    return CInt_getvalue(self);
  } else if (strcmp(name, "getvalue") == 0) {
    return MethodWrapper<decltype(&CInt_getvalue)>::createInstance(
        self, CInt_getvalue, "_ffi");
  }
  PyErr_SetString(PyExc_AttributeError, "attribute not found");
  return nullptr;
}

extern "C" PyObject *CInt_str(PyObject *self) {
  char buf[16];
  sprintf(buf, "0x%x", from_pyobj(self)->getvalue());
  return cppbind::Str(buf).object().unwrap();
}

static PyMethodDef CInt_methods[] = {
    (PyMethodDef){"getvalue", (PyCFunction)CInt_getvalue,
                  METH_METHOD | METH_NOARGS, nullptr},
    (PyMethodDef){nullptr, nullptr, 0, nullptr} /* Sentinel */
};

extern "C" PyObject *CInt_Init(PyObject *self) {
  PyTypeObject *CInt_type =
      (PyTypeObject *)_PyObject_New((PyTypeObject *)&PyType_Type);
  auto offset = offsetof(PyTypeObject, tp_name);
  memset(&(CInt_type->tp_name), 0, sizeof(*CInt_type) - offset);
  CInt_type->tp_name = "_ffi.CInt";
  CInt_type->tp_basicsize = sizeof(CInt);
  CInt_type->tp_dealloc = CInt_destructor;
  CInt_type->tp_getattr = CInt_getattr;
  CInt_type->tp_flags = Py_TPFLAGS_DEFAULT;
  CInt_type->tp_methods = CInt_methods;
  CInt_type->tp_repr = CInt_str;
  PyObject *ret = _PyObject_New((PyTypeObject *)CInt_type);

  new (from_pyobj(ret)) CInt();
  return ret;
}

extern "C" PyObject *CInt_New(PyObject *self) { return CInt_Init(self); }
extern "C" PyObject *CInt_FromInt(PyObject *self, PyObject *arg) {
  auto *ret = CInt_Init(self);
  from_pyobj(ret)->num = (long)Long(arg);
  return ret;
}

gen_modinit_fn_from_fns(
    _ffi, nullptr, nullptr, nullptr,
    gen_PyMethodDef_doc(CInt_New, ":returns: a new CInt object"),
    (PyMethodDef){"CInt_FromInt", (PyCFunction)CInt_FromInt, METH_O,
                  "Creates a new CInt object from an integer."},
    (PyMethodDef){
        "CppMap_New", (PyCFunction)CppMap_New, METH_O,
        "Creates a new CppMap object with the given compare function."})
