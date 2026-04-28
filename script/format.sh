#!/usr/bin/sh 

set -ex

config_file="$PWD/.clang-format"

do_fmt() {
  find . \( -name \*.cc -or -name \*.h \) -print0 | xargs -0 clang-format --style="file:$config_file" -i
}

for srcdir in include src test example bin; do
  cd "$srcdir"
  do_fmt
  cd ..
done

exit 0

