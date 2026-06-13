#!/bin/bash

set -eo pipefail

echo "output" >&2

asm="$1"
epack="$2"

if [[ ! -f "$asm" || ! -f "$epack" ]]; then
	echo "$0 <assembler> <epack>"
	exit 1
fi

source ../common.sh "$asm" "$epack"
trap stop EXIT

assemble_from code.s0a

run | assert_str "Hello World\n"

assert_ec0

