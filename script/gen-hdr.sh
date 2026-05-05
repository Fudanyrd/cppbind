#!/usr/bin/sh
# Generate a single-file header for cppbind. Instead of distributing
# the include/ directory, we can just distribute cppbind-sys.h, which contains
# all the necessary system headers and cppbind itself.
set -ex

test -d include || {
    echo "Error: include/ directory not found."
    exit 1
}

cd include

# detect system headers.
find . -name \*.h -print0 | xargs -0 cat | grep -E '#include <(.+)>' | sort | uniq > ./sys.inc

# Perform copy-paste preprocessing to resolve all #include "..." 
# directives in cppbind.h and its dependencies.
cat << EOF | python3 > cppbind.inc
import re

processed = set()
include_pattern = re.compile(r'#include\s+"(.+)"')
sys_include_pattern = re.compile(r'#include\s+<(.+)>')

def preprocess(ifile: str):
    if ifile in processed:
        return
    processed.add(ifile)
    with open(ifile, 'r', encoding='utf-8') as f:
        lines = f.readlines()
        for line in lines:
            if re.match(sys_include_pattern, line):
                pass
            elif m := re.match(include_pattern, line):
                preprocess(m.group(1))
            else:
                print(line, end='')

# cppbind.h is the main header that includes all other headers.
if __name__ == '__main__':
    preprocess('cppbind.h')
EOF

# Generate the final header.
cat << EOF > cppbind-sys.h
#ifndef __CPPBIND_SYS_H__
#define __CPPBIND_SYS_H__ (1)

#include <Python.h>

EOF

{
  grep -v 'gtest' < sys.inc 
  cat < cppbind.inc         
  echo '#endif /* __CPPBIND_SYS_H__ */'
} >> cppbind-sys.h

rm -f sys.inc cppbind.inc || true

# finally, format the generated header.
cd ..
clang-format -i include/cppbind-sys.h

exit 0
