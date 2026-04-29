# Simple Memory Sanitizer for cppbind

We create a class `Resource` which can only allocate
a single instance. This is useful for checking
incorrect reference count management in the `cppbind`
library -- especially regarding function/method invocation.

