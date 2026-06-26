#!/bin/bash

set -eo pipefail

echo "echo" >&2

asm="$1"
epack="$2"
s1alib="$3"

if [[ ! -f "$asm" || ! -f "$epack" || ! -f "$s1alib" ]]; then
	echo "$0 <assembler> <epack>"
	exit 1
fi

source ../common.sh "$asm" "$epack" "$s1alib"
trap stop EXIT

assemble_from code.s1a

run | assert_str "1\n$file\n"
run argument | assert_str "2\n$file\nargument\n"
run arg0 arg1 arg2 | assert_str "4\n$file\narg0\narg1\narg2\n"
run "Hello, World!" | assert_str "2\n$file\nHello, World!\n"

assert_ec0

