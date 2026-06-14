#!/bin/bash

set -eo pipefail

asm="$1"
epack="$2"
s1alib="$3"

if [[ ! -f "$asm" || ! -f "$epack" || ! -f "$s1alib" ]]; then
	echo "$0 <assembler> <epack> <s1alib>" >&2
	exit 1
fi

file="$(mktemp)"

assemble() {
	(cat && cat "$s1alib") | "$asm" | "$epack" > "$file"
	chmod +x "$file"
}

assemble_from() {
	input="$1"
	if [[ ! -f "$input" ]]; then
		echo "$0 <input file>" >&2
		exit 1
	fi
	cat "$input" | assemble
}

run() {
	if [[ ! -f "$file" ]]; then
		echo -e "\tno file to run" >&2
		exit 1
	fi
	"$file"
}

assert_ec0() {
	if [[ $? -ne 0 ]]; then
		echo -e "\texit code not 0" >&2
		exit 1
	else
		if [[ -n "$verbose" ]]; then
			echo -e "\texit code 0" >&2
		fi
	fi
}

assert_str() {
	tgt_str="$1"
	tgt_file="$(mktemp)"
	out_file="$(mktemp)"
	cat > "$out_file"
	echo -en "$tgt_str" > "$tgt_file"
	if ! cmp "$out_file" "$tgt_file"; then
		echo >&2
		echo >&2
		echo >&2
		echo -e "\ntgt_file\n" >&2
		cat "$tgt_file" >&2
		echo -e "\nout_file\n" >&2
		cat "$out_file" >&2
		echo >&2
		echo >&2
		echo >&2
		echo -e "\toutput differs" >&2
		rm "$tgt_file" "$out_file"
		exit 1
	else
		echo -ne "\toutput matches (" >&2
		if [[ "$(cat "$tgt_file" | wc -l)" -eq 0 ]]; then
			cat "$tgt_file" >&2
			echo -n " == " >&2
			cat "$out_file" >&2
		else
			echo -n "too long" >&2
		fi
		echo ")" >&2
		rm "$tgt_file" "$out_file"
	fi
}

stop() {
	if [[ -f "$file" ]]; then
		rm "$file"
	fi
}

sign_extend() {
	num="$1"
	printf "%lld" "$(( ($num & 0x7fffffff) | -($num & 0x80000000) ))"
}

