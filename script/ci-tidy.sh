#!/usr/bin/sh
# run clang-tidy on the codebase.
set -e

test -f '.clang-tidy' || {
    echo "Error: .clang-tidy file not found in the current directory." >&2
    exit 1
}

workdir="$( pwd )"
compile_commands="$workdir/build/compile_commands.json"
test -f "$compile_commands" || {
    echo "Error: compile_commands.json not found in $workdir/build." >&2
    exit 1
}

for f in src/*.cc test/*.cc example/ffi/*.cc; do
    # prints to /dev/null to shorten length of CI logs.
    clang-tidy -p "$compile_commands" "$f" >/dev/null 2>&1 || {
        echo "Last failed file: $f" >&2
        exit 1
    }
done

exit 0
