/**
 * Initialization of static members.
 */

#include <cppbind.h>

namespace cppbind {

template <>
PyTypeObject *MethodWrapper<MethodTableEntry::method_t>::method_type = nullptr;

}
