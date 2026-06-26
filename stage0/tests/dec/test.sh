#!/bin/bash

set -eo pipefail

echo "DEC" >&2

asm="$1"
epack="$2"
s1alib="$3"

if [[ ! -f "$asm" || ! -f "$epack" || ! -f "$s1alib" ]]; then
	echo "$0 <assembler> <epack> <s1alib>"
	exit 1
fi

source ../common.sh "$asm" "$epack" "$s1alib"
trap stop EXIT

result() {
	num0="$1"
	printf "%llu" $(("$num0" - 1))
}

set_nr() {
	str="$1"
	num="$2"
	 sed "s/$str+0/$(printf "x%0.4x_" $(( ($num >> 0 ) & 0xffff )) )/g" | \
	sed "s/$str+16/$(printf "x%0.4x_" $(( ($num >> 16) & 0xffff )) )/g" | \
	sed "s/$str+32/$(printf "x%0.4x_" $(( ($num >> 32) & 0xffff )) )/g" | \
	sed "s/$str+48/$(printf "x%0.4x_" $(( ($num >> 48) & 0xffff )) )/g"
}

check() {
	num0="$1"
	res="$(result "$num0")"

	cat fmt.s1a | set_nr "NUM0" "$num0" | assemble

	run | assert_str "$res"

	assert_ec0
}

check 1
check 2
check 42
check 255
check 256
check 65535
check 65536
check 4294967295
check 4294967296
check 0x8000000000000000
check 0xFFFFFFFFFFFFFFFF
check 0
check 100

