#!/usr/bin/sh
# Generate documentation for cppbind library.
# It requires doxygen to be installed.

set -ex
if test -z "$1"; then
  echo "path to doxygen is not set."
  exit 1
fi

DOXYGEN="$1"

test -f "Doxyfile" || {
  echo "Doxyfile not found. Please run this script in the root directory of cppbind."
  exit 1
}

awk '{
  if ($1 ~ /^#/) {
    next
  }
  if ($1 == "WARN_AS_ERROR" && $3 != "YES") {
    print "Oops, you should not disable Doxygen Werror\n"
    exit 1
  }
}' < Doxyfile

"$DOXYGEN" || {
  cat doxygen.log 1>&2
  exit 1
}
exit 0
