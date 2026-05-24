#include <cppbind-sys.h>

#define this_package "ext"

struct Animal {
  virtual const char *make_noise() { return ""; }
};

struct Cat final : public Animal {
  const char *make_noise() override { return "meow"; }
};

#define foreach_animal_method(X) X(make_noise, make_noise, const char *)

namespace cppbind {
template <> struct CppObject<Animal>;
template <> struct CppObject<Cat>;
type_static_members_declare(CppObject<Animal>);
type_static_members_declare(CppObject<Cat>);

cpp_class_wrapper(Animal, foreach_animal_method);
cpp_class_wrapper(Cat, foreach_animal_method);
} /* namespace cppbind */

method_wrapper_static_members_declare(cppbind::MethodTableEntry::method_t);
type_static_members(cppbind::CppObject<Animal>);
type_static_members(cppbind::CppObject<Cat>);

static int rest_init() {
  MethodWrapper_init(this_package, cppbind::MethodTableEntry::method_t);
  cpp_type_init(this_package, Animal, "Animal", foreach_animal_method);
  if (PyType_Ready(cpp_get_type_object(Animal)) != 0) {
    /* PyType_Ready already set error string. */
    return 1;
  }

  cpp_type_init(this_package, Cat, "Cat", foreach_animal_method);
#define foreach_base(X) X(Animal)
  cpp_type_register_bases(Cat, foreach_base);
#undef foreach_base

  /* Finish initialization. */
  if (PyType_Ready(cpp_get_type_object(Cat)) != 0) {
    return 1;
  }
  return 0;
}

gen_modinit_fn_from_fns(ext, &rest_init, nullptr, nullptr, nullptr,
                        {"Cat",
                         (PyCFunction)cppbind::staticize_constructor<Cat>,
                         METH_FASTCALL, "Create a Cat object."},
                        {"Animal",
                         (PyCFunction)cppbind::staticize_constructor<Animal>,
                         METH_FASTCALL, "Create an Animal object."});
