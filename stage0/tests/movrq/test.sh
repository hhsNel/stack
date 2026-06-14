#!/bin/bash

set -eo pipefail

echo "MOVrq" >&2

asm="$1"
epack="$2"
s1alib="$3"

if [[ ! -f "$asm" || ! -f "$epack" || ! -f "$s1alib" ]]; then
	echo "$0 <assembler> <epack>"
	exit 1
fi

source ../common.sh "$asm" "$epack" "$s1alib"
trap stop EXIT

result() {
	num0="$1"
	printf "%llu" $(("$num0"))
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

check 0
check 1
check 65535
check 65536
check 0xFFFFFFFF
check 0x100000000
check 0x0000FFFFFFFFFFFF
check 0x7FFFFFFFFFFFFFFF
check 0xFFFFFFFFFFFFFFFF
check 0x123456789ABCDEF
check 0x5851F42D4C957F2D
check 11111111111111111111
check 0xDEADBEEFDEADBEEF

