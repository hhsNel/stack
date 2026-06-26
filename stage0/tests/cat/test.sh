#!/bin/bash

set -eo pipefail

echo "cat" >&2

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

echo -ne "Hello World\n" | run | assert_str "Hello World\n"

assert_ec0

