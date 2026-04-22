#!/usr/bin/sh 

set -ex

do_fmt() {
  find . \( -name \*.cc -or -name \*.h \) -print0 | xargs -0 clang-format -i
}

cd include && do_fmt
cd ../src && do_fmt
cd ../test && do_fmt

exit 0

