#!/bin/bash

set -eo pipefail

echo "ADCrr" >&2

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
	num1="$2"
	printf "%llu" $(("$num0" + "$num1"))
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

check 1 2
check 2 3
check 4 5
check 0 0
check 0 1
check 1 0
check 65535 65535
check 65535 0
check 0 65535
check 255 1
check 1 255
check 255 255
check 32767 1
check 32767 32767
check 32768 32768
check 65534 1
check 65534 2
check 43690 21845
check 1234 5678
check 43210 12345
check 60000 50000
check 0x000000000000FFFF 1
check 0x00000000FFFFFFFF 1
check 0x0000FFFFFFFFFFFF 1
check 0x0000FFFFFFFFFFFF 1
check 0x123456789ABCDEF0 0x1111111111111111
check 0x9ABCDEF012345678 0x0505050505050505
check 0x7FFFFFFFFFFFFFFF 1
check 0xFFFFFFFFFFFFFFFE 1
check 0x1000000000000000 0x2000000000000000
check 0x0001000000000000 0x0002000000000000
check 0x0000000100000000 0x0000000200000000

