# Before `git push`

Run code formatting, unit tests and examples to make sure 
everything is working as expected. 

```sh
{
set -e
export CC=gcc  # c compiler
export CXX=g++ # c++ compiler
./script/docs.sh doxygen # generate docs
./script/format.sh  # code formatting
./script/ci-test.sh # unit test and run example
}
```

# Coding style

## CMakeLists.txt

Use the prefix `CPPBIND_` for all custom cmake variables.

## C/C++

* Style is based on [.clang-format](./.clang-format).
* Use Camel case names for `struct`(s) and `class`(es).
* Use `__<namespace>_<file name>__` as the macro defined at the start and end of header files.
* Use snake case names for functions and variables.
* Use GNU-style [single-line comments](https://www.gnu.org/software/grub/manual/grub-dev/grub-dev.html#Comments) and
  [JavaDoc multi-line comments](https://www.doxygen.nl/manual/docblocks.html).

## Shell Scripts

(TBD)

