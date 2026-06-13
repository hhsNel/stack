#!/bin/bash

set -eo pipefail

asm="$1"
epack="$2"

if [[ ! -f "$asm" || ! -f "$epack" ]]; then
	echo "$0 <assembler> <epack>" >&2
	exit 1
fi

file="$(mktemp)"

assemble() {
	"$asm" | "$epack" > "$file"
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
		echo -e "\texit code 0" >&2
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
		echo "tgt_file" >&2
		cat "$tgt_file" >&2
		echo "out_file" >&2
		cat "$out_file" >&2
		echo >&2
		echo >&2
		echo >&2
		echo -e "\toutput differs" >&2
		rm "$tgt_file" "$out_file"
		exit 1
	else
		echo -e "\toutput matches" >&2
		rm "$tgt_file" "$out_file"
	fi
}

stop() {
	if [[ -f "$file" ]]; then
		rm "$file"
	fi
}

