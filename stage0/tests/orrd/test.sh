#!/bin/bash

set -eo pipefail

echo "ORrd" >&2

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
	num1="$2"
	printf "%llu" $(("$num0" | "$(sign_extend "$num1")"))
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
	num1="$2"
	res="$(result "$num0" "$num1")"

	cat fmt.s1a | set_nr "NUM0" "$num0" | set_nr "NUM1" "$num1" | assemble

	run | assert_str "$res"

	assert_ec0
}

check 0 0
check 0 1
check 1 1
check 100 200
check 65535 1
check 65535 65535
check 65536 65535
check 65535 65536
check 4294967294 1
check 4294967295 1
check 0x0000000100000000 1
check 0x0000000100000000 0xFFFFFFFF
check 0x7FFFFFFFFFFFFFFF 1
check 0xFFFFFFFFFFFFFFFF 1
check 1000000 999999
check 0x123456780000000 0x12345678

